/*--------------------------------------------------------------------------*/
/*--------------------- File MMCFNetworkDesignBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Implementation of the MMCFNetworkDesignBlock class.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "MMCFNetworkDesignBlock.h"

#include "BlockSolverConfig.h"
#include "CDASolver.h"

#include <algorithm>
#include <cmath>

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*-------------------------------- FUNCTIONS -------------------------------*/
/*--------------------------------------------------------------------------*/

SMSpp_insert_in_factory_cpp_1( MMCFNetworkDesignBlock );

/*--------------------------------------------------------------------------*/
/*----------------------------- METHODS ------------------------------------*/
/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::load( std::istream & input , char frmt )
{
 guts_of_destructor();

 f_mmcf = new MMCFBlock();
 f_mmcf->load( input , frmt );

 if( anyone_there() )
  add_Modification( std::make_shared< NBModification >( this ) );
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::deserialize( const netCDF::NcGroup & group )
{
 auto sg = group.getGroup( "MMCFBlock" );
 if( sg.isNull() )
  throw( std::logic_error( "MMCFNetworkDesignBlock::deserialize: the "
			   "MMCFBlock group is required" ) );

 guts_of_destructor();

 f_mmcf = new MMCFBlock();
 f_mmcf->deserialize( sg );

 Block::deserialize( group );
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::serialize( netCDF::NcGroup & group ) const
{
 Block::serialize( group );

 if( f_mmcf ) {
  auto sg = group.addGroup( "MMCFBlock" );
  f_mmcf->serialize( sg );
  }
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::generate_abstract_variables( Configuration * stvv )
{
 static const std::string _prfx =
                      "MMCFNetworkDesignBlock::generate_abstract_variables: ";

 if( AR & HasVar )
  return;

 if( ! f_mmcf )
  throw( std::logic_error( _prfx + "no instance has been loaded" ) );

 // the formulation, and the scale of the cost of the slack arcs - - - - - - -
 if( ( ! stvv ) && f_BlockConfig )
  stvv = f_BlockConfig->f_static_variables_Configuration;
 if( auto c = dynamic_cast< SimpleConfiguration< int > * >( stvv ) )
  f_form = c->f_value;
 else
  if( auto c = dynamic_cast< SimpleConfiguration< std::pair< int ,
                                                   double > > * >( stvv ) ) {
   f_form = c->f_value.first;
   f_slack_scale = c->f_value.second;
   }
 if( ( f_form != kMonolithic ) && ( f_form != kBenders ) )
  throw( std::invalid_argument( _prfx + "unknown formulation " +
				std::to_string( f_form ) ) );

 // the design Variable- - - - - - - - - - - - - - - - - - - - - - - - - - -
 const Index m = f_mmcf->get_NArcs();
 v_y.resize( m );
 for( auto & y : v_y )
  y.set_type( ColVariable::kBinary , eNoMod );
 add_static_variable( v_y , "y" );

 if( f_form == kMonolithic ) {
  // the MMCFBlock is the sub-Block, with the flow structure (its default),
  // and the y are its design Variable
  add_nested_Block( f_mmcf );
  f_mmcf->generate_abstract_variables();
  f_mmcf->set_design_variables( & v_y );
  }
 else {
  // the epigraph Variable of the cost of the flow, and the hidden copy
  v_epi.set_type( ColVariable::kContinuous , eNoMod );
  add_static_variable( v_epi , "v" );

  f_sub = f_mmcf->get_slack_copy( f_slack_scale );
  f_sub->generate_abstract_variables();
  f_sub->generate_abstract_constraints();
  f_sub->generate_objective();

  // how the copy is solved: the extra Configuration of the BlockConfig
  Configuration * extra = f_BlockConfig ?
                          f_BlockConfig->f_extra_Configuration : nullptr;
  BlockConfig * bc = nullptr;
  BlockSolverConfig * bsc = nullptr;
  if( auto c = dynamic_cast< BlockSolverConfig * >( extra ) )
   bsc = c;
  else
   if( auto c = dynamic_cast< SimpleConfiguration< std::pair<
			 Configuration * , Configuration * > > * >( extra ) ) {
    bc = dynamic_cast< BlockConfig * >( c->f_value.first );
    bsc = dynamic_cast< BlockSolverConfig * >( c->f_value.second );
    if( ( ! bsc ) && c->f_value.second )
     throw( std::invalid_argument( _prfx + "the second element of the "
				   "extra Configuration must be a "
				   "BlockSolverConfig" ) );
    }
   else
    if( extra )
     throw( std::invalid_argument( _prfx + "the extra Configuration must be "
				   "a BlockSolverConfig or a pair of a "
				   "BlockConfig and a BlockSolverConfig" ) );
  if( bc )
   bc->apply( f_sub );
  if( bsc )
   bsc->apply( f_sub );
  }

 AR |= HasVar;
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::generate_abstract_constraints(
						       Configuration * stcc )
{
 if( AR & HasCons )
  return;

 if( ! ( AR & HasVar ) )
  generate_abstract_variables();

 if( f_form == kMonolithic ) {
  // the Configuration of the constraints is the one of the MMCFBlock, e.g.,
  // whether it has the forcing ones [see MMCFBlock::set_design_variables()]
  if( ( ! stcc ) && f_BlockConfig )
   stcc = f_BlockConfig->f_static_constraints_Configuration;
  f_mmcf->generate_abstract_constraints( stcc );
  }
 else {
  add_dynamic_constraint( v_cuts , "Benders" );

  // the cut at y = 1, which bounds v from the start; it is there only if
  // the copy has a Solver, otherwise it comes with the first separation
  if( ! f_sub->get_registered_solvers().empty() ) {
   std::vector< double > y0( v_y.size() );
   for( Index j = 0 ; j < v_y.size() ; ++j ) {
    y0[ j ] = v_y[ j ].get_value();
    v_y[ j ].set_value( 1 );
    }
   separate_Benders_cut( -1 );
   for( Index j = 0 ; j < v_y.size() ; ++j )
    v_y[ j ].set_value( y0[ j ] );
   }
  }

 AR |= HasCons;
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::generate_dynamic_constraints(
						       Configuration * dycc )
{
 if( f_form != kBenders )
  return;

 double eps_rel = 1e-6;
 if( ( ! dycc ) && f_BlockConfig )
  dycc = f_BlockConfig->f_dynamic_constraints_Configuration;
 if( auto c = dynamic_cast< SimpleConfiguration< std::pair< int ,
                                                   double > > * >( dycc ) )
  eps_rel = c->f_value.second;

 separate_Benders_cut( eps_rel );
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::generate_objective( Configuration * objc )
{
 if( AR & HasObj )
  return;

 if( ! ( AR & HasVar ) )
  generate_abstract_variables();

 const auto & F = f_mmcf->get_F();
 LinearFunction::v_coeff_pair coeffs;
 for( Index j = 0 ; j < v_y.size() ; ++j )
  if( ( j < F.size() ) && ( F[ j ] != 0 ) )
   coeffs.push_back( std::make_pair( & v_y[ j ] , double( F[ j ] ) ) );
 if( f_form == kBenders )
  coeffs.push_back( std::make_pair( & v_epi , double( 1 ) ) );

 f_obj.set_function( new LinearFunction( std::move( coeffs ) , 0 ) );
 f_obj.set_sense( Objective::eMin , eNoMod );
 set_objective( & f_obj );

 // the costs of the flow are in the MCFBlock, in kMonolithic
 if( f_form == kMonolithic )
  f_mmcf->generate_objective();

 AR |= HasObj;
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::set_subproblem_capacities(
					        const std::vector< double > & y )
{
 const Index m = v_y.size();
 const auto & UTot = f_sub->get_UTot();
 const auto & U = f_sub->get_U();

 // the arcs whose y is not the one the copy has been given last
 Subset chg;
 for( Index j = 0 ; j < m ; ++j )
  if( ( j >= v_ylast.size() ) || ( v_ylast[ j ] != y[ j ] ) )
   chg.push_back( j );
 if( chg.empty() )
  return;

 // the mutual capacities, those of the arcs that have one
 for( auto j : chg )
  if( auto mc = f_sub->get_mutual_constraint( j ) )
   mc->set_rhs( UTot[ j ] * y[ j ] );

 // the individual capacities of each commodity, i.e., of its MCFBlock
 for( Index k = 0 ; k < f_sub->get_NComm() ; ++k ) {
  auto mk = static_cast< MCFBlock * >( f_sub->get_nested_Block( k ) );
  Subset nms;
  MCFBlock::Vec_FNumber nu;
  for( auto j : chg )
   if( ! mk->is_closed( j ) ) {
    nms.push_back( j );
    nu.push_back( U[ k ][ j ] * y[ j ] );
    }
  if( ! nms.empty() )
   mk->chg_ucaps( nu.cbegin() , std::move( nms ) , true );
  }

 v_ylast = y;
 }

/*--------------------------------------------------------------------------*/

bool MMCFNetworkDesignBlock::separate_Benders_cut( double eps_rel )
{
 if( ( f_form != kBenders ) || ( ! f_sub ) ||
     f_sub->get_registered_solvers().empty() )
  return( false );

 // the current y; an integer one that is integer up to the tolerance of
 // the Solver (say, 1 - 4e-8 at an integer solution of a MILP) is rounded,
 // since its tiny fraction would otherwise become a tiny capacity of the
 // copy (the cut is valid whatever y it is computed at)
 const Index m = v_y.size();
 std::vector< double > y( m );
 for( Index j = 0 ; j < m ; ++j ) {
  y[ j ] = std::clamp( v_y[ j ].get_value() , 0.0 , 1.0 );
  if( v_y[ j ].is_integer() &&
      ( std::abs( y[ j ] - std::round( y[ j ] ) ) <= IntTol ) )
   y[ j ] = std::round( y[ j ] );
  }

 set_subproblem_capacities( y );

 // solve the copy at the current y- - - - - - - - - - - - - - - - - - - - -
 auto slvr = dynamic_cast< CDASolver * >(
                                    f_sub->get_registered_solvers().front() );
 if( ! slvr )
  return( false );
 const int st = slvr->compute();
 if( ( st != Solver::kOK ) && ( st != Solver::kLowPrecision ) )
  return( false );
 f_slv_val = slvr->get_var_value();
 slvr->get_dual_solution();

 // the multipliers of the mutual capacities, and the arcs that have one - -
 /* The sign with which the Solver writes them is not relevant: any lambda
  * >= 0 gives a valid cut, and of the two the one of the larger value at y
  * is taken, which is the one of the Solver. */
 const auto & UTot = f_sub->get_UTot();
 const auto & U = f_sub->get_U();
 std::vector< double > dual( m , 0 );
 std::vector< bool > hasmc( m , false );
 for( Index j = 0 ; j < m ; ++j )
  if( auto mc = f_sub->get_mutual_constraint( j ) ) {
   dual[ j ] = mc->get_dual();
   hasmc[ j ] = true;
   }

 // the cut of a given lambda: alpha + g' y, by weak duality - - - - - - - -
 auto cut = [ & ]( double sgn , double & alpha , std::vector< double > & g ) {
  alpha = 0;
  g.assign( m , 0 );
  for( Index j = 0 ; j < m ; ++j )
   if( hasmc[ j ] )
    g[ j ] = - std::max( 0.0 , sgn * dual[ j ] ) * UTot[ j ];

  for( Index k = 0 ; k < f_sub->get_NComm() ; ++k ) {
   auto mk = static_cast< MCFBlock * >( f_sub->get_nested_Block( k ) );
   const Index n = mk->get_NNodes();
   MCFBlock::Vec_CNumber pi( n );
   mk->get_pi( pi.begin() , Block::Range( 0 , n ) );

   // b' pi, the flow conservation constraints
   const auto & B = mk->get_B();
   for( Index i = 0 ; i < std::min( n , Index( B.size() ) ) ; ++i )
    alpha += B[ i ] * pi[ i ];

   // and the capacities, mu = min( 0 , reduced cost ) of each open arc
   for( Index j = 0 ; j < mk->get_NArcs() ; ++j ) {
    if( mk->is_closed( j ) || mk->is_deleted( j ) )
     continue;
    double rc = mk->get_C( j ) + pi[ mk->get_SN( j ) - 1 ]
                               - pi[ mk->get_EN( j ) - 1 ];
    if( ( j < m ) && hasmc[ j ] )
     rc += std::max( 0.0 , sgn * dual[ j ] );
    const double mu = std::min( 0.0 , rc );
    if( mu == 0 )
     continue;
    if( j < m )   // the capacity of the arc is U y
     g[ j ] += mu * U[ k ][ j ];
    else          // a slack arc, whose capacity is fixed
     alpha += mu * mk->get_U( j );
    }
   }
  };

 double alpha , alpha2;
 std::vector< double > g , g2;
 cut( 1 , alpha , g );
 cut( -1 , alpha2 , g2 );
 auto value = [ & ]( double a , const std::vector< double > & gg ) {
  double v = a;
  for( Index j = 0 ; j < m ; ++j )
   v += gg[ j ] * y[ j ];
  return( v );
  };
 if( value( alpha2 , g2 ) > value( alpha , g ) ) {
  alpha = alpha2;
  g.swap( g2 );
  }
 f_cut_val = value( alpha , g );

 // add it if it is violated enough- - - - - - - - - - - - - - - - - - - - -
 if( eps_rel >= 0 ) {
  const double v = v_epi.get_value();
  if( f_cut_val - v <= eps_rel * std::max( std::abs( v ) , 1.0 ) )
   return( false );
  }

 // v - g' y >= alpha
 LinearFunction::v_coeff_pair coeffs;
 coeffs.reserve( m + 1 );
 coeffs.push_back( std::make_pair( & v_epi , double( 1 ) ) );
 for( Index j = 0 ; j < m ; ++j )
  if( g[ j ] != 0 )
   coeffs.push_back( std::make_pair( & v_y[ j ] , - g[ j ] ) );

 std::list< FRowConstraint > lst( 1 );
 lst.back().set_lhs( alpha );
 lst.back().set_rhs( Inf< RowConstraint::RHSValue >() );
 lst.back().set_function( new LinearFunction( std::move( coeffs ) , 0 ) );
 add_dynamic_constraints( v_cuts , lst , eNoBlck );
 return( true );
 }

/*--------------------------------------------------------------------------*/

Solution * MMCFNetworkDesignBlock::get_Solution( Configuration * solc ,
						 bool emptys )
{
 auto sol = new ColVariableSolution;
 if( ! emptys )
  sol->read( this );
 return( sol );
 }

/*--------------------------------------------------------------------------*/

void MMCFNetworkDesignBlock::guts_of_destructor( void )
{
 for( auto & c : v_cuts )
  c.clear();
 v_cuts.clear();
 f_obj.clear();
 reset_objective();

 if( f_sub ) {  // the Solver of the hidden copy are its own
  f_sub->unregister_Solvers( true );
  delete f_sub;
  f_sub = nullptr;
  }

 // in kMonolithic the MMCFBlock is a sub-Block, and Block deletes it
 if( ! v_Block.empty() )
  reset_nested_Block();
 else
  delete f_mmcf;
 f_mmcf = nullptr;

 reset_static_variables();
 reset_dynamic_constraints();
 v_y.clear();
 v_ylast.clear();
 AR = 0;
 f_form = kMonolithic;
 }

/*--------------------------------------------------------------------------*/
/*------------------ End File MMCFNetworkDesignBlock.cpp -------------------*/
/*--------------------------------------------------------------------------*/
