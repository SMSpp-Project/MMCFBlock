/*--------------------------------------------------------------------------*/
/*------------------------- File MMCFBlock.cpp ----------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Implementation of the MMCFBlock class.
 *
 * \version 0.01
 *
 * \date 30 - 12 - 2020
 *
 * \author Antonio Frangioni \n
 *         Operations Research Group \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \author Enrico Gorgone \n
 *         Operations Research Group \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * Copyright &copy by Antonio Frangioni
 */
/*--------------------------------------------------------------------------*/
/*---------------------------- IMPLEMENTATION ------------------------------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "MMCFBlock.h"
#include "Configuration.h"
#include "SMSTypedefs.h"
#include <math.h>

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;
using namespace std;

/*--------------------------------------------------------------------------*/
/*--------------------------------- TYPES ----------------------------------*/
/*--------------------------------------------------------------------------*/


using Index = Block::Index;

/*--------------------------------------------------------------------------*/
/*-------------------------------- MACROS ----------------------------------*/
/*--                                                                      --*/
/*--      Some small macro definitions, used throughout the code.         --*/
/*--                                                                      --*/
/*--------------------------------------------------------------------------*/

#define GOODN( n ) if( ( n <= 0 ) || ( Index( n ) > NNodes ) )	\
 throw( std::invalid_argument( "Invalid actual node name" ) )

#define GOODN2( n ) if( ( ( n <= 0 ) || ( Index( n ) > NNodes ) ) && \
			( n != -1 ) ) \
 throw( std::invalid_argument( "Invalid generic node name" ) )

#define GOODP( k ) if( ( k <= 0 ) || ( Index( k ) > NumProd ) ) \
 throw( std::invalid_argument( "Invalid actual product name" ) )

#define GOODP2( k ) if( ( ( k <= 0 ) || ( Index( k ) > NumProd ) ) && \
			( k != -1 ) ) \
 throw( std::invalid_argument( "Invalid generic product name" ) )

#define GOODL( l ) if( Index( l ) > NCnst ) \
 throw( std::invalid_argument( "Invalid link name" ) )

/*--------------------------------------------------------------------------*/
/*----------------------------- FUNCTIONS ----------------------------------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*----------------------------- STATIC MEMBERS -----------------------------*/
/*--------------------------------------------------------------------------*/

// register MMCFBlock to the Block factory
SMSpp_insert_in_factory_cpp_1( MMCFBlock );

/*--------------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/

void MMCFBlock::generate_abstract_variables( Configuration * stvv )
{
 if( AR & HasVar ) {
  // TODO: check if stvv agrees with the formulation we currently have
  //       and thorw exception otherwise
  return;
  }

 // TODO: check stvv and construct other formulations accordingly
  auto c = Configuration::deserialize("GAVPar.txt");
  stvv = dynamic_cast< SimpleConfiguration<int> * >( c );
  if( ! stvv ) {
   cerr << "Error: configuration file not a BlockSolverConfig" << endl;
   delete c;
   exit( 1 );
  }
 unsigned char fr =  dynamic_cast< SimpleConfiguration<int> * >(stvv)->value();
 AR = (AR & (~4))|(4*fr);
 
 
 // initialize the children - - - - - - - - - - - - - - - - - - - - - - - - -

//if(FlowRelaxation == true){
if( AR & FlowRelaxation){

 v_Block.resize( NComm );
 
 for( Index k = 0 ; k < NComm ; ++k ) {
  /*!!
  if( PT[ k ] == kSPT )
   do something more clever
   !!*/

  auto MCFb = new MCFBlock( this );
  MCFb->load( NNodes , NArcs , Startn , Endn , U[ k ] , C[ k ] , B[ k ] );
  v_Block[ k ] = MCFb;
  }
}
else{


 //construct vectors for the flow relaxation
 int items;
 std::vector< double > bound;
 std::vector< bool > Integrality;
 FMultiVector weights;
 FMultiVector costs;
 double Cmax=0;

if(Active.size()){
 weights.resize( NCnst  );  // allocate weights for the knapsack sub-problem
 costs.resize( NCnst  );  // allocate costs for the knapsack sub-problem
 bound.resize(NCnst);

for( Index j = 0 ; j < NCnst ; j++ ){ 
 if( filetypeBlock == 's' ){
   weights[ j ].resize( NComm + 1 );
   costs[ j ].resize( NComm + 1 );
   }else{
   weights[ j ].resize( NComm);
   costs[ j ].resize( NComm);
   }
//   for(Index k=0;k < NComm; k++)
//       Cmax += NNodes*C[ k ][ Active[j] ]; 
   for(Index k =0; k< NComm; k++){
     weights[ j ][ k ] = UTot[ Active[j] ];
     costs[ j ][ k ] =  C[ k ][ Active[j] ]*UTot[ Active[j] ];
//     if( C[ k ][ j ] >= Inf<double>())
//         costs[ j ][ k ] = Cmax*UTot[ Active[j] ];
   }
   
  if( filetypeBlock == 's' ){
     costs[ j ][ NComm ] =  C[ NComm ][ Active[j] ];
     weights[ j ][ NComm ] = - UTot[ Active[j] ];
  }
 }
 
 items = NComm;
  if(filetypeBlock == 's'){
   items++;
   Integrality.resize(NComm+1);
   for( Index k = 0 ; k < NComm ; ++k )  {
      Integrality[ k ] = false; 
   }
   Integrality[ NComm ] = true; 
   for( Index j = 0 ; j < NCnst ; ++j )  {
    bound[j] = 0;
   }
  }else{
   Integrality.resize(NComm);
   for( Index k = 0 ; k < NComm ; ++k )  {
    Integrality[ k ] = false;
   }
   for( Index j = 0 ; j < NCnst ; ++j )  {
    bound[j] = UTot[Active[j]];
   }
  }
   v_Block.resize( NCnst );
 
 for( Index j = 0; j < NCnst; j++ ){
  auto BKb = new BinaryKnapsackBlock( this );
  BKb->load( items, bound[j], weights[ j ], costs[ j ], Integrality ); 
  BKb->set_objective_sense(false);
  v_Block[ j ] = BKb;
  }
  
}else{ 
 weights.resize( NArcs  );  // allocate weights for the knapsack sub-problem
 costs.resize( NArcs  );  // allocate costs for the knapsack sub-problem
 
 
for( Index j = 0 ; j < NArcs ; j++ ){ 
 if( filetypeBlock == 's' ){
   weights[ j ].resize( NComm + 1 );
   costs[ j ].resize( NComm + 1 );
   }else{
   weights[ j ].resize( NComm);
   costs[ j ].resize( NComm);
   }
   for(Index k=0;k < NComm; k++)
       Cmax += NNodes*C[ k ][ j ]; 
   for(Index k =0; k< NComm; k++){
     weights[ j ][ k ] = UTot[ j ];
     costs[ j ][ k ] =  C[ k ][ j ]*UTot[ j ];
     if( C[ k ][ j ] >= Inf<double>())
         costs[ j ][ k ] = Cmax*UTot[j];
   }
   
  if( filetypeBlock == 's' ){
     costs[ j ][ NComm ] =  C[ NComm ][ j ];
     weights[ j ][ NComm ] = - UTot[ j ];
  }
 }
 
 bound.resize(NArcs);
 items = NComm;
  if(filetypeBlock == 's'){
   items++;
   Integrality.resize(NComm+1);
   for( Index j = 0 ; j < NComm ; ++j )  {
      Integrality[ j ] = false; 
   }
   Integrality[ NComm ] = true; 
   for( Index j = 0 ; j < NArcs ; ++j )  {
   bound[j] = 0;
   }
  }else{
  Integrality.resize(NComm);
   for( Index j = 0 ; j < NComm ; ++j )  {
   Integrality[ j ] = false;
   }
   for( Index j = 0 ; j < NArcs ; ++j )  {
   bound[j] = UTot[j];
   }
  }


 v_Block.resize( NArcs );
 
 for( Index j = 0; j < NArcs; j++ ){
  auto BKb = new BinaryKnapsackBlock( this );
  BKb->load( items, bound[j], weights[ j ], costs[ j ], Integrality ); 
  BKb->set_objective_sense(false);
  v_Block[ j ] = BKb;
  }
  }

}
  
 // call the base class method to have it done in the sub-Block, if any
 Block::generate_abstract_variables();

 AR |= HasVar;
 }

/*--------------------------------------------------------------------------*/

void MMCFBlock::generate_abstract_constraints( Configuration * stcc )
{
 if( AR & HasMutual )
  return;
  
 auto c = Configuration::deserialize("GACPar.txt");
 stcc = dynamic_cast< SimpleConfiguration<int> * >( c );
 if( ! stcc ) {
   cerr << "Error: configuration file not a BlockSolverConfig" << endl;
   delete c;
   exit( 1 );
 }
 unsigned char sl =  dynamic_cast< SimpleConfiguration<int> * >(stcc)->value();
 AR = (AR & (~slc))|(slc*sl);

 // do it in the MCF/BKB respectively
 for( auto blck : v_Block )
  blck->generate_abstract_constraints();

// if(FlowRelaxation==true){
 if( AR & FlowRelaxation ){
 
  // count number of nonzeroes in each constraint, i.e., #FS( i ) + #BS( i )
  Subset count( get_NArcs() );
  
  // initialize the vectors of coefficients, and reset count[]
  std::vector< LinearFunction::v_coeff_pair > coeffs( get_NArcs() );

  for( Index j = 0 ; j < get_NArcs() ; ++j ) {
   coeffs[ j ].resize( NComm );
   count[ j ] = 0;
   }
  for( Index k = 0 ; k < get_NComm() ; k++ )
   for( Index j = 0 ; j < get_NArcs() ; ++j )
     coeffs[ j ][ k ] = std::make_pair(
       static_cast< MCFBlock * >( v_Block[ k ] )->i2p_x( j ) , double( 1 ) );

 // generate the mutual capacity constraints  - - - - - - - - - - - - - - -
 // each constraint is an inequality, i.e., RHS = UTot[ j ]
  if( Active.size() ) {
   MCs.resize( NCnst );
   for( Index j = 0 ; j < NCnst ; ++j ) {
    if( UTot[ Active[ j ] ] >= Inf<double>() )
     throw( std::logic_error( "Constraint required to have a finite rhs" ) );

    MCs[ j ].set_function( new LinearFunction(
				 std::move( coeffs[ Active[ j ] ] ) , 0 ) );
    MCs[ j ].set_rhs( UTot[ Active[ j ] ] );
    MCs[ j ].set_lhs( -Inf<double>() );
    }
   }
  else{
   MCs.resize( get_NArcs() );
   for( Index j = 0 ; j < get_NArcs() ; ++j ) {
    if( UTot[ j ] >= Inf<double>() )
     throw( std::logic_error( "Constraint required to have a finite rhs" ) );
    MCs[ j ].set_rhs( UTot[ j ] );
    MCs[ j ].set_lhs( -Inf<double>() );
    MCs[ j ].set_function( new LinearFunction( std::move( coeffs[ j ] ) , 0 ) );
    }
   }
  add_static_constraint( MCs , "Mut" );

 }else{
 
 
  // count number of nonzeroes in each constraint, i.e., #FS( i ) + #BS( i )
  std::vector< Subset > count( get_NComm() );
 
  // initialize the vectors of coefficients, and reset count[]
  //std::vector< std::vector < LinearFunction::v_coeff_pair > > coeffs;
  boost::multi_array< LinearFunction::v_coeff_pair , 2 > coeffs( boost::extents[get_NComm()][get_NNodes()] );
  
  //coeffs.resize( get_NNodes()*get_NComm() );
  
  for( Index k = 0 ; k < get_NComm() ; ++k ) {
   count[k].resize(get_NNodes());
   if(Active.size()){
     for( Index i = 0 ; i < NCnst ; ++i ) {
       count[ k ][ Startn[ Active[i] ] - 1 ]++;
       count[ k ][ Endn[ Active[i] ] - 1 ]++;
     }
   }else{   
     for( Index i = 0 ; i < get_NArcs() ; ++i ) {
       count[ k ][ Startn[ i ] - 1 ]++;
       count[ k ][ Endn[ i ] - 1 ]++;
     }
   }    
  }
  
 for( Index k = 0 ; k < get_NComm() ; ++k ) {
  for( Index i = 0 ; i < get_NNodes() ; ++i ) {
   coeffs[ k ][ i ].resize( count[ k ][ i ] );
   count[ k ][ i ] = 0;
   }
  }

  // construct the vector of coefficients, static phase


 for( Index k = 0 ; k < get_NComm() ; ++k ) {
  if(Active.size()){
    for(  Index i = 0; i < NCnst ; ++i ) {
      if(Startn[ Active[i] ]==Endn[ Active[i] ])
        continue;
      coeffs[ k ][ Startn[ Active[i] ] - 1 ][ count[ k ][ Startn[ Active[i] ] - 1 ]++ ] =  std::make_pair( ( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] )->get_Var( k ))  ,  double( UTot[ Active[i] ] )  ) ;
      coeffs[ k ][ Endn[ Active[i] ] - 1 ][ count[ k ][ Endn[ Active[i] ] - 1 ]++ ] = std::make_pair( ( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] )->get_Var( k ))  ,  double( - UTot[ Active[i] ]  )  ) ;
    }
  }else{
    for(  Index i = 0; i < get_NArcs() ; ++i ) {
      if(Startn[ i ]==Endn[ i ])
         continue;
     coeffs[ k ][ Startn[ i ] - 1 ][ count[ k ][ Startn[ i ] - 1 ]++ ] =  std::make_pair( ( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] )->get_Var( k ))  ,  double( UTot[ i ] )  ) ;

     coeffs[ k ][ Endn[ i ] - 1 ][ count[ k ][ Endn[ i ] - 1 ]++ ] = std::make_pair( ( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] )->get_Var( k ))  ,  double( - UTot[ i ]  )  ) ;
     }
   }
 }
   
 FCs.resize( boost::extents[ get_NComm() ][ get_NNodes() ]);
 for(  Index i = 0; i < get_NNodes() ; ++i ){ 
   for( Index k = 0 ; k < get_NComm() ; ++k ) {  
    (FCs)[ k ][ i ].set_both( B.empty() ? 0 : B[ k ][ i ] );
    (FCs)[ k ][ i ].set_function( new LinearFunction( std::move( coeffs[ k ][ i ] ) , 0 ) );
   }
 }
 
   add_static_constraint( FCs, "Flow" );
   
   
if(AR & slc){
 
 boost::multi_array< LinearFunction::v_coeff_pair , 2 > coeffsSLC( boost::extents[get_NComm()][get_NArcs()] );
  
 for( Index k = 0 ; k < get_NComm() ; ++k ) {
  for( Index i = 0 ; i < get_NArcs() ; ++i ) {
   coeffsSLC[ k ][ i ].resize( 2 );
   }
  }

  // construct the vector of coefficients, static phase


 for( Index k = 0 ; k < get_NComm() ; ++k ) {
  for(  Index i = 0; i < get_NArcs() ; ++i ) {

   coeffsSLC[ k ][ i ][ 0 ] =  std::make_pair( ( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] )->get_Var( k ))  ,  double( 1 )  ) ;
   
 coeffsSLC[ k ][ i ][ 1 ] =  std::make_pair( ( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] )->get_Var( get_NComm() ))  ,  double( -1 )  ) ;
   }
 }
 
 SLCs.resize( boost::extents[ get_NComm() ][ get_NArcs() ]);
 
 for(  Index i = 0; i < get_NArcs() ; ++i ){ 
   for( Index k = 0 ; k < get_NComm() ; ++k ) {  
   
    (SLCs)[ k ][ i ].set_lhs( -Inf<double>() );
    
    (SLCs)[ k ][ i ].set_rhs( 0 );
    
    (SLCs)[ k ][ i ].set_function( new LinearFunction( std::move( coeffsSLC[ k ][ i ] ) , 0 ) );
    
   }
 }
 
   add_static_constraint( SLCs, "StrongForcCons" );
 
 
 }
 
 
 }
 AR |= HasMutual;
 

 }  // end( MMCFBlock::generate_abstract_constraints() )

/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

MMCFBlock::~MMCFBlock() {

 // for( Index k = 0 ; k< NComm ; k++ )
 //  delete v_Block[ k ];

 } // end destructor   - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*--------------------------------------------------------------------------*/
/*-------------------------- PROTECTED METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/

void MMCFBlock::print( std::ostream &output ) const
{
 output << "MMCFBlock with " << NArcs * NComm << " vars " << std::endl;
 }

/*--------------------------------------------------------------------------*/

void MMCFBlock::load( const char *const filename , char filetype )
{
 // check parameters- - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 filetypeBlock = filetype;
 if( ( filetype != 'm' ) && ( filetype != 'p' ) && ( filetype != 'd' ) &&
     ( filetype != 'o' ) && ( filetype != 'u' ) && ( filetype != 's' ) &&
     ( filetype != 'c' ) )
  throw( std::invalid_argument( "invalid file type" +
				std::string( 1 , filetype ) ) );

 bool FourFiles = ( ( filetype != 's' ) && ( filetype != 'c' ) );

 // in principle there is no "extra" stuff- - - - - - - - - - - - - - - - - -

 NXtrV = NXtrC = 0;
 IdxBeg.resize( 0 );
 CoefIdx.resize( 0 );
 CoefVal.resize( 0 );

 // reading general informations- - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 c_Index l = strlen( filename );
 char *Name = new char[ l + 5 ];  // temporary string containing the constant
 strcpy( Name , filename );       // part of the pathname + space for `.XXX'

 if( FourFiles )
  strcpy( Name + l , ".nod" );

 ifstream inFile( Name );
 if( ! inFile.is_open() )
  throw( std::invalid_argument( "can't open file" + std::string( Name ) ) );

 if( FourFiles ) {
  inFile >> NComm;
  inFile >> NNodes;
  inFile >> NArcs;
  inFile >> NCnst;
  }
 else {
  inFile >> NNodes;
  inFile >> NArcs;
  inFile >> NComm;

  if( filetype == 'c' ) {   // in the PPRN format, read description of side - - - -
   inFile >> NXtrC;   // constraints and prepare the data structures

   Index NNZ;
   inFile >> NNZ;

   if( NNZ ) {
    IdxBeg.resize( NXtrC );
    CoefIdx.resize( NNZ );
    CoefVal.resize( NNZ );
    }
   }

  NCnst = NArcs;
  }

 if( NNodes <= 1 )
  throw( std::invalid_argument( "wrong node number" ) );
 if( NArcs <= 0 )
  throw( std::invalid_argument( "wrong arc number" ) );
 if( NComm <= 0 )
  throw( std::invalid_argument( "wrong commodity number" ) );
 if( NCnst > NArcs )
  throw( std::invalid_argument( "wrong constraints number" ) );

 if( FourFiles )
  inFile.close();

 Subset Origins;
 Subset Destins;

 Subset StartOfK;
 Subset TempIdx;

 Index NumProd = NComm;

 // format-dependent part - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 if( FourFiles ) {  // preparing to read the supply file
  if( filetype == 'u' ) {
   strcpy( Name + l , ".od" );
   filetype = 'd';
   }
  else {
   strcpy( Name + l, ".sup" );
  }
  }

 // determining the actual number of commodities for (OSP) or (ODS)- - - - - -
 // formulations: in the first case, a commodity is a pair ( product , - - - -
 // origin ), while in the second case it is a triplet ( product ,-  - - - - -
 // origin , destination ) - - - - - - - - - - - - - - - - - - - - - - - - - -

 if( ( filetype == 'd' ) || ( filetype == 'o' ) ) {
  StartOfK.resize( NumProd + 1 , 0 );

  inFile.clear();        // ensure failbits are not left dirty
  inFile.open( Name );   // commodities can be told from supplies
  if( ! inFile.is_open() )
   throw( std::invalid_argument( "can't open file" + std::string( Name ) ) );

  int origin;   // the *.sup file is read once here just to count the number
  int dest;     // of commodities: the actual data reading will be done later
  int comm;
  FNumber flow;

  if( filetype == 'd' )  // in (ODS) count the different O/D pairs - - - - - - - - -
   while( inFile >> origin ) {
    GOODN( origin );

    inFile >> dest;
    GOODN( dest );

    inFile >> comm;
    GOODP2( comm );

    inFile >> flow;

    if( comm != -1 )
     StartOfK[ comm ]++;
    else
     for( Index i = NumProd ; i ; )
      StartOfK[ i-- ]++;
    }
  else  // in (OSP) count the number of different Origins- - - - - - - - - - -
   while( inFile >> origin ) {
    GOODN( origin );

    inFile >> dest;
    GOODN2( dest );

    inFile >> comm;
    GOODP2( comm );

    inFile >> flow;

    if( dest == -1 ) {
     if( comm != -1 )
      StartOfK[ comm ]++;
     else
      for( Index i = NumProd ; i ; )
       StartOfK[ i-- ]++;
     }
    }

  // really construct StartOfK - - - - - - - - - - - - - - - - - - - - - - - -

  StartOfK[ 0 ] = 0;
  for( Index i = 2 ; i <= NumProd ; i++ )
   StartOfK[ i ] += StartOfK[ i - 1 ];

  NComm = StartOfK[ NumProd ]; // note that NComm can "surprisingly" be
                               // < NumProd if some product does not appear

  Origins.resize( NComm );

  inFile.close();

  }  // end if( (OSP) or (ODP) )

 // allocating and initializing memory- - - - - - - - - - - - - - - - - - - -
 // (note that this part is again common) - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 C.resize( NComm + 1 );  // allocate costs- - - - - - - - - - - - - - - -

 if( filetype == 'c' )
  for( Index i = 0 ; i < NComm ; i++ )
   C[ i ].resize( NArcs );
 else
  for( Index i = 0 ; i < NComm ; i++ )
   C[ i ].resize( NArcs , Inf<CNumber>() );   // arcs are un-existent
                               // unless otherwise stated

 // C[ NComm ] is empty

 U.resize( NComm + 2 );  // allocate capacities - - - - - - - - - - - - -

 if( filetype == 'c' )
  for( Index i = 0 ; i < NComm ; i++ )
   U[ i ].resize( NArcs );
 else
  for( Index i = 0 ; i < NComm ; i++ )
   U[ i ].resize( NArcs , 0 );     // arcs are un-existent unless otherwise stated

 // U[ NComm ] = U[ NComm + 1 ] are empty

 B.resize( NComm + 2 );  // allocate deficits - - - - - - - - - - - - - -

 if( filetype == 'c' )
  for( Index i = 0 ; i < NComm ; i++ )
   B[ i ].resize( NNodes );
 else
  for( Index i = 0 ; i < NComm ; i++ )
   B[ i ].resize( NNodes , 0 );    // nodes all have 0 deficit
                                   // unless otherwise stated

 // B[ NComm ] = B[ NComm + 1 ] are empty

 // allocate start/end nodes and mutual capacities- - - - - - - - - - - - - -

 Startn.resize( NArcs );
 Endn.resize( NArcs );

 UTot.resize( NArcs );

 // allocate info on integrality of the variables - - - - - - - - - - - - - -

 NInt.resize( NComm + 1 , 0 );
 WIsInt.resize( NComm + 1 );

 // reading supply/demand infos, or everything in one-files format- - - - - -
 // (this part is partly splitted again)- - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 if( FourFiles ) {
  if( filetype == 'm' )
   TempIdx.resize( NCnst );
  else {
   TempIdx.resize( max( Index( NumProd + 1 ) , NComm ) );
   if( ( filetype == 'o' ) || ( filetype == 'd' ) )
    std::copy( StartOfK.begin() , StartOfK.begin() + NumProd + 1 ,
    		TempIdx.begin() );
   }

  inFile.clear();        // ensure failbits are not left dirty
  inFile.open( Name );  // the right name is already there
  if( ! inFile.is_open() )
   throw( std::invalid_argument( "can't open file" + std::string( Name )  ) );
  }

 switch( filetype ) {

 case( 's' ): // Canadian format- - - - - - - - - - - - - - - - - - - - - - -
 {            //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // allocate the data structures for "extra" things- - - - - - - - - - - - -

  C[ NComm ].resize( NXtrV = NArcs );
  U[ NComm ].resize( NArcs , FNumber( 0 ) );     // "extra" variables
  U[ NComm + 1 ].resize( NArcs , FNumber( 1 ) ); // are in the ...
                                                 // ... [0, 1] range
  NInt[ NComm ] = NArcs;                         // ... and integer

  for( Index i = 0 ; i < NArcs ; i++ ) {         // read arc-related info- - - - - -
   inFile >> Endn[ i ];
   GOODN( Endn[ i ] );

   inFile >> Startn[ i ];
   GOODN( Startn[ i ] );
   if( Startn[ i ] == Endn[ i ] )
	throw( std::invalid_argument( "self-loop" ) );

   inFile >> C[ NComm ][ i ];

   FNumber f;
   inFile >> f;

   UTot[ i ] = ( f >= 0 ? f : Inf<FNumber>() );

   Index h;
   inFile >> h;

   for( ; h-- ; ) {
    Index k;
    inFile >> k;
    GOODP( k );

    inFile >> C[ --k ][ i ];
    inFile >> f;

    U[ k ][ i ] = ( f >= 0 ? f : Inf<FNumber>() );

    }  // end for( h )
   }  // end for( i )

  for( Index k ; inFile >> k ; ) {  // read node-related info - - - - - - - -
   GOODP( k );

   Index i;
   inFile >> i;
   GOODN( i );

   FNumber f;
   inFile >> f;
   B[ --k ][ --i ] = -f;

   // inFile >> B[ --k ][ --i ];
   }

  break;

  }  // end case( s ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 case( 'c' ): // PPRN format- - - - - - - - - - - - - - - - - - - - - - - - -
 {            //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  for( Index k = 0 ; k < NComm ; k++ )  // read all costs
   for( Index i = 0 ; i < NArcs ; )
    inFile >> C[ k ][ i++ ];

  for( Index k = 0 ; k < NComm ; k++ )  // read all capacities
   for( Index i = 0 ; i < NArcs ; ) {
    FNumber f;
    inFile >> f;

    U[ k ][ i++ ] = ( f >= 0 ? f : Inf<FNumber>() );
    }

  for( Index k = 0 ; k < NComm ; k++ )  // read all supplies
   for( Index i = 0 ; i < NNodes ; ) {
	FNumber f;
	inFile >> f;
	B[ k ][ i++ ]=-f;
    }

  for( Index i = 0 ; i < NArcs ; ) {      // read all total capacities
   FNumber f;
   inFile >> f;

   UTot[ i++ ] = ( f >= 0 ? f : Inf<FNumber>() );
   }

  for( Index i = 0 ; i < NArcs ; i++ ) {  // read graph topology
   inFile >> Startn[ i ];
   GOODN( Startn[ i ] );

   inFile >> Endn[ i ];
   GOODN( Endn[ i ] );

   if( Startn[ i ] == Endn[ i ] )
    throw( std::invalid_argument( "self-loop" ) );
   }

  if( NXtrC ) {  // if there are "extra" constraints- - - - - - - - - - - - -
  // FRow EL = B[ NComm ]     = new FNumber[ NXtrC ];
  // FRow EU = B[ NComm + 1 ] = new FNumber[ NXtrC ];

   B[ NComm ].resize( NXtrC );
   B[ NComm + 1 ].resize( NXtrC );

   for( Index i = 0 ; i < NXtrC ; ) {  // read "extra" Uppr./Lwr. bounds
    inFile >> B[ NComm ][ i ];
    inFile >> B[ NComm + 1 ][ i++ ];
    }

   Index currc = 0;
   Index currpos = 0;

   for( Index j ; inFile >> j ; ) {  // read extra constraints description:
    Index k;                         // j = arc name
    inFile >> k;                     // commodity name

    CoefIdx[ currpos ] = (--k) * NArcs + (--j);

    Index h;
    inFile >> h;                     // constraint name
    h--;

    while( h > currc ) {
     IdxBeg[ currc ] = currpos;
     currc++;
     }

    inFile >> CoefVal[ currpos++ ];    // the coefficient

    }  // end( for( ! eof() ) )

   IdxBeg[ currc ] = currpos;

   }  // end( if( extra constraints ) )

  break;

  }  // end case( c ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 case( 'm' ): // mnetgen format - - - - - - - - - - - - - - - - - - - - - - -
 {            //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  for( Index node ; inFile >> node ; ) {

   GOODN( node );

   int comm;
   inFile >> comm;
   GOODP2( comm );

   FNumber flow;
   inFile >> flow;

   if( comm == -1 )
    for( Index k = 0 ; k < NComm ; )
     B[ k++ ][ node - 1 ] = flow;
   else
    B[ comm - 1 ][ node - 1 ] = flow;
   }

  break;

  }  // end case( m ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 case( 'p' ): // JL (PSP) format- - - - - - - - - - - - - - - - - - - - - - -
 {            //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  for( int origin ; inFile >> origin ; ) {
   GOODN2( origin );

   int dest;
   inFile >> dest;
   GOODN2( dest );
   if( ( ( origin == -1 ) && ( dest == -1 ) ) ||
       ( ( origin != -1 ) && ( dest != -1 ) ) )
	throw( std::invalid_argument( "exactly one p/d in PSP" ) );

   int comm;
   inFile >> comm;
   GOODP2( comm );

   FNumber flow;
   inFile >> flow;

   if( comm != -1 ) {
    comm--;

    if( origin < 0 )
     B[ comm ][ dest - 1 ] = -flow;
    else
     B[ comm ][ origin - 1 ] = flow;
    }
   else
    if( origin < 0 )
     for( Index i = NumProd ; i-- ; )
      B[ i ][ dest - 1 ] = -flow;
    else
     for( Index i = NumProd ; i-- ; )
      B[ i ][ origin - 1 ] = flow;

   }  // end( for( ! eof() ) )

  break;

  }  // end case( p ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 case( 'o' ): // JL (OSP) format- - - - - - - - - - - - - - - - - - - - - - -
 {            // while reading supplies, commodity "names" are assigned - - -

  for( int origin ; inFile >> origin ; ) {
   int dest;
   inFile >> dest;

   int comm;
   inFile >> comm;

   FNumber flow;
   inFile >> flow;

   if( comm != -1 ) {
    // origin or destination node for the given pair ( product , origin )

    Index i = StartOfK[ --comm ];

    while( ( i < TempIdx[ comm ] ) && ( Origins[ i ] != Index( origin ) ) )
     i++;  // seek the name of the commodity

    if( i == TempIdx[ comm ] ) {  // a "new" commodity
     Origins[ i ] = origin;
     TempIdx[ comm ]++;
     }

    if( dest == -1 )  // it is an origin
     B[ i ][ origin - 1 ] = flow;
    else
     B[ i ][ dest - 1 ] = - flow;
    }
   else {  // comm == -1
    // origin or destination node for all the commodities ( k , origin )
    // for each product k

    Index i;
    for( Index k = NumProd ; k-- ; ) {
     i = StartOfK[ k ];

     while( ( i < TempIdx[ k ] ) && ( Origins[ i ] != Index( origin ) ) )
      i++;  // seek the name of the commodity

     if( i == TempIdx[ k ] ) {  // a "new" commodity
      Origins[ i ] = origin;
      TempIdx[ i ]++;
      }

     if( dest == -1 )  // it is an origin
      B[ i ][ origin - 1 ] = flow;
     else
      B[ i ][ dest - 1 ] = - flow;

     }  // end for( k )
    }  // end else( comm == -1 )
   }  // end while( ! eof() )

  break;

  }  // end case( o ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 default: // 'd' == JL (ODS) format - - - - - - - - - - - - - - - - - - - - -
 {        // while reading supplies, commodity "names" are assigned - - - - -

  Destins.resize( NComm );

  for( int origin ; inFile >> origin ; ) {
   int dest;
   inFile >> dest;

   int comm;
   inFile >> comm;

   FNumber flow;
   inFile >> flow;

   if( comm != -1 ) {
    comm--;

    Origins[ TempIdx[ comm ] ] = origin;
    Destins[ TempIdx[ comm ] ] = dest;

    B[ TempIdx[ comm ] ][ dest - 1 ] = -flow;
    B[ TempIdx[ comm ]++ ][ origin - 1 ] = flow;
    }
   else
    for( Index i = NumProd ; i-- ; ) {
     Origins[ TempIdx[ i ] ] = origin;
     Destins[ TempIdx[ i ] ] = dest;

     B[ TempIdx[ i ] ][ dest - 1 ] = -flow;
     B[ TempIdx[ i ]++ ][ origin - 1 ] = flow;
     }
    }  // end while( ! eof() )
   }   // end default()- - - - - - - - - - - - - - - - - - - - - - - - - - - -
  }    // end switch( FT ) - - - - - - - - - - - - - - - - - - - - - - - - - -
       //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 inFile.close();

 if( FourFiles ) {
  // continue only in the multi-file formats - - - - - - - - - - - - - - - - -

  UTot.resize( NArcs );
  UTot.assign( NArcs , Inf<FNumber>() );

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // now the though part: reading arc infos - - - - - - - - - - - - - - - - -
  // (again, this part is splitted) - - - - - - - - - - - - - - - - - - - - -

  strcpy( Name + l , ".arc" );

  inFile.clear();        // ensure failbits are not left dirty
  inFile.open( Name );
  if( ! inFile.is_open() )
   throw( std::invalid_argument( "can't open file" + std::string( Name )  ) );

  if( filetype == 'm' )   // mnetgen format - - - - - - - - - - - - - - - - - - - -
  {                 // it is dealt with separatedly, since it's simpler: the
   Index who;       // name of the arc (who) is explicitely given

   while( inFile >> who ) {
    if( ( who <= 0 ) || ( who > NArcs ) )
     throw( std::invalid_argument( "invalid arc name" ) );
    who--;

    Index from;
    inFile >> from;
    GOODN( from );

    Index to;
    inFile >> to;
    GOODN( to );
    if( from == to )
     throw( std::invalid_argument( "self-loop" ) );

    int comm;
    inFile >> comm;
    GOODP2( comm );

    CNumber cost;
    inFile >> cost;

    FNumber cap;
    inFile >> cap;
    if( cap < 0 )
     cap = Inf<FNumber>();

    Index ptr;
    inFile >> ptr;
    GOODL( ptr );

    if( ptr )
     TempIdx[ ptr - 1 ] = who;

    Startn[ who ] = from;
    Endn[ who ] = to;

    if( comm == -1 )
     for( Index k = 0 ; k < NComm ; ) {
      C[ k ][ who ] = cost;
      U[ k++ ][ who ] = cap;
      }
    else {
     C[ --comm ][ who ] = cost;
     U[ comm ][ who ] = cap;
     }
    }   // end while()
   }    // end if( mnetgen )
  else {  // the three JL formats - - - - - - - - - - - - - - - - - - - - - -
          //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Index unbndld = 0;  // counter of unbundled links so far

   for( Index from ; inFile >> from ; ) {  // first, the usual data reading
    GOODN( from );

    Index to;
    inFile >> to;
    GOODN( to );
    if( from == to )
     throw( std::invalid_argument( "self-loop" ) );

    int comm;
    inFile >> comm;
    GOODP2( comm );

    CNumber cost;
    inFile >> cost;

    int cap;
    inFile >> cap;

    int origin;
    inFile >> origin;
    GOODN2( origin );

    int dest;
    inFile >> dest;
    GOODN2( dest );

    Index ptr;
    inFile >> ptr;
    GOODL( ptr );

    // now, the main part: from the triplet (origin, destination, product)
    // plus the file type (p, d, o) a list of applicable commodities is
    // constructed and put into TempIdx: then the arc will be replicated over
    // all the commodities of the list
    // in all the three fields, a "-1" takes the place of a wildcard

    Index TmpCommCntr = 1;

    switch( filetype ) {

    case( 'p' ):  // the simplest, only 2 subcases- - - - - - - - - - - - - -

     if( comm != -1 )      // a specific commodity (prod)
      TempIdx[ 0 ] = comm - 1;
     else                  // all commodities (!?)
      for( Index i = TmpCommCntr = NComm ; i-- ; )
       TempIdx[ i ] = i;

     break;

    case( 'o' ):  // tougher, 4 subcases- - - - - - - - - - - - - - - - - - -

     if( comm != -1 ) {
      Index i = StartOfK[ comm - 1 ];

      if( origin != -1 ) {          // a specific commodity ( prod , origin )
       while( ( i < StartOfK[ comm ] ) &&
	      ( Origins[ i ] != Index( origin ) ) )
        i++;

       assert( i != StartOfK[ comm ] );
       TempIdx[ 0 ] = i;
       }
      else                          // all commodities with a given product
       for( TmpCommCntr = 0 ; i < StartOfK[ comm ] ; )
        TempIdx[ TmpCommCntr++ ] = i++;
      }
     else
      if( origin != -1 )            // all commodities with a given origin
       for( Index i = TmpCommCntr = 0 ; i < NumProd ; ) {
        Index k = StartOfK[ i++ ];

        while( ( k < StartOfK[ i ] ) &&
	       ( Origins[ k ] != Index( origin ) ) )
         k++;

        if( k < StartOfK[ comm ] )
         TempIdx[ TmpCommCntr++ ] = k;
        }
      else                         // all commodities
       for( Index i = TmpCommCntr = NComm ; i-- ; )
        TempIdx[ i ] = i;

     break;

    default:     // == 'd', the toughest: 8 subcases- - - - - - - - - - - - -

     if( comm != -1 ) {
      Index i = StartOfK[ comm - 1 ];

      if( dest != -1 ) {
       if( origin != -1 ) {      // a specific commodity (prod, origin, dest)
        while( ( i < StartOfK[ comm ] ) && ( Destins[ i ] != Index( dest ) )
               && ( Origins[ i ] != Index( origin ) ) )
         i++;

        assert( i != StartOfK[ comm ] );
        TempIdx[ 0 ] = i;
        }
       else                      // all commodities with a given (prod, dest)
        for( TmpCommCntr = 0; i < StartOfK[ comm ] ; i++ )
         if( Destins[ i ] == Index( dest ) )
          TempIdx[ TmpCommCntr++ ] = i;
       }
      else {                     // dest == -1
       if( origin != -1 ) {      // all commodities with a given (prod, orig)
        for( TmpCommCntr = 0; i < StartOfK[ comm ] ; i++ )
         if( Origins[ i ] == Index( origin ) )
          TempIdx[ TmpCommCntr++ ] = i;
        }
       else                      // all commodities with a given (prod)
        for( TmpCommCntr = 0; i < StartOfK[ comm ] ; i++ )
         TempIdx[ TmpCommCntr++ ] = i;
       }
      }
     else                       // comm == -1
      if( dest != -1 ) {
       if( origin != -1 ) {     // all commodities with a given (orig, dest)
        for( Index i = TmpCommCntr = 0 ; i < NComm ; i++ )
         if( ( Destins[ i ] == Index( dest ) ) &&
	     ( Origins[ i ] == Index( origin ) ) )
          TempIdx[ TmpCommCntr++ ] = i;
        }
       else                    // all commodities with a given (dest)
        for( Index i = TmpCommCntr = 0 ; i < NComm ; i++ )
         if( Destins[ i ] == Index( dest ) )
          TempIdx[ TmpCommCntr++ ] = i;
       }
     else                      // dest == -1
      if( origin != -1 ) {     // all commodities with a given (origin)
       for( Index i = TmpCommCntr = 0 ; i < NComm ; i++ )
        if( Origins[ i ] == Index( origin ) )
         TempIdx[ TmpCommCntr++ ] = i;
        }
      else                     // all commodities
       for( Index i = TmpCommCntr = NComm ; i-- ; )
        TempIdx[ i ] = i;

     }  // end switch( FT )

    // now "filling" the proper arc for each commodity

    for( Index i = TmpCommCntr ; i-- ; ) {
     comm = TempIdx[ i ];
     Index who;

     if( ptr ) {      // if ptr != 0 it's easy
      who = ptr - 1;  // ( ptr - 1 ) is already the correct name

      Startn[ who ] = from;
      Endn[ who ] = to;
      }
     else {           // otherwise find the "name" of arc (from, to)
      Index k = 0;    // and put it into who

      while( ( k < unbndld ) &&
             ( ( Startn[ NCnst + k ] != from ) ||
               ( Endn[ NCnst + k ] != to ) ||
               ( C[ comm ][ NCnst + k ] < Inf<CNumber>() ) ) )
       k++;

      // search for an arc (from, to) already defined among the unbundled ones
      // and whose "instance" relative to commodity comm has not already been
      // taken: this is not the only way of accomodating unbundled arcs, (in
      // case of multiple instances of an unbundled arc (i, j)), but it is
      // easy to see that all the resulting problems, however you distribute
      // the instances to arcs, are equivalent

      who = NCnst + k;
      assert( who < NArcs );

      if( k == unbndld ) {  // if no such arc exists ...
       unbndld++;           // ... a new one is created
       Startn[ who ] = from;
       Endn[ who ] = to;
       }
      }  // end else( ! ptr )

     C[ comm ][ who ] = cost;
     U[ comm ][ who ] = ( cap >= 0 ? cap : Inf<FNumber>() );

     } // end for( all comm. )
    }  // end while( ! eof() )

   if( NCnst + unbndld < NArcs )
    NArcs = NCnst + unbndld;

   }   // end else( JL formats )

  inFile.close();

  // reading mutual capacities- - - - - - - - - - - - - - - - - - - - - - - -
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  strcpy( Name + l , ".mut" );
  inFile.clear();        // ensure failbits are not left dirty
  inFile.open( Name );
  if( ! inFile.is_open() )
   throw( std::invalid_argument( "can't open file" + std::string( Name ) ) );

  for( Index i = 0 ; i < NCnst ; ) {
   Index j;
   inFile >> j;

   FNumber f;
   inFile >> f;

   if( filetype == 'm' )
    j = TempIdx[ i++ ];
   else
    j = i++;

   UTot[ j ] = ( f >= 0 ? f : Inf<FNumber>() );

   }

  // temporary deallocation and final things- - - - - - - - - - - - - - - - -

  if( ( filetype == 'd' ) || ( filetype == 'o' ) ) {
   StartOfK.clear();
   Origins.clear();

   if( filetype == 'd' )
    Destins.clear();
   }

  TempIdx.clear();

  }  // end if( FourFiles )

 delete[] Name;

 // common initializations- - - - - - - - - - - - - - - - - - - - - - - - - -
 CmnIntlz();
 
 }  // end( MMCFBlock::Load )

/*--------------------------------------------------------------------------*/

void MMCFBlock::PreProcess( FNumber IncUk , FNumber DecUk ,
			    FNumber IncUjk , FNumber DecUjk ,
			    FNumber ChgDfct , CNumber DecCsts )
{
 if( ChgDfct >= Inf<double>() )
  throw( std::invalid_argument( "infinite ChgDfct" ) );
 if( DecCsts > Inf<double>() )
  throw( std::invalid_argument( "infinite DecCsts" ) );

 // allocate (temporary) data structures- - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 Active.resize( NArcs );

 Vec_FNumber tmpv( NComm );
 Subset srck( NComm );

 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // squeeze rhss, declare arcs as "non-existent", etc.- - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 // ensure that all arcs entering/leaving a non-existent node do not exist- -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 for( Index k = 0 ; k < NComm ; k++ )
  for( Index i = 0 ; i < NArcs ; i++ )
   if( ( B[ k ][ Startn[ i ] - StrtNme ] == Inf<double>() ) ||
       ( B[ k ][ Endn[ i ] - StrtNme ] == Inf<double>() ) )
    C[ k ][ i ] = Inf<double>();

 // ensure that all non-existent arcs have zero capacity- - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 for( Index k = 0 ; k < NComm ; k++ )
  for( Index i = 0 ; i < NArcs ; i++ )
   if( C[ k ][ i ] == Inf<double>() )
    U[ k ][ i ] = 0;

 // a *very* rough estimate of the max. flow across any arc is computed for
 // each commodity, and it is stored in tmpv[ k ] - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 FNumber maxU = 0;
 for( Index k = NComm ; k-- ; ) {
  // first, count the flow out of the sources

  Index srcs = 0;    // meanwhile, the sources are counted
  FNumber maxUk = 0;
  for( auto &Bk : B[k] )
   if( Bk < 0 ) {
	srcs++;
	maxUk -= Bk;
    }

  // now the contribution of arcs with potentially negative costs

  for( Index j = 0 ; j < NArcs ; j++ ) {
   const FNumber tMF = min( U[ k ][j] , UTot[j] );

   if( C[ k ][ j ] < DecCsts ) {
    if( tMF >= Inf<double>() )
     throw( std::invalid_argument( "negative cost, infinite capacity" ) );
    maxUk += tMF;
    }
   }

  srck[ k ] = srcs;
  maxUk += ( ( NNodes + 1 ) / 2 ) * ChgDfct;  // count potential changes in
                                              // the deficits
  maxU += ( tmpv[ k ] = maxUk );

  }

 // detection of redundant mutual capacity constraints is attempted, and- - -
 // all the mutual capacity upper bounds are turned to finite values- - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 for( Index i = NCnst = 0 ; i < NArcs ; i++ ) {
  if( ( ! IncUk ) && ( ! UTot[ i ] ) ) {   // if mutual capacities can not
   for( Index k = NComm ; k-- ; ) {        // increase, and UTot[] == 0 ...
    C[ k ][ i ] = Inf<double>();           // ... this arc does not exist
    U[ k ][ i ] = 0;
    }

   continue;
   }

  if( DecUk == Inf<double>() ) {     // all mutual capacity constraints exist
   if( UTot[ i ] == Inf<double>() )  // but those that are declared non-so
    UTot[ i ] = maxU;                 // ensure that UTot is "finite" anyway
   else
    Active[ NCnst++ ] = i;

   continue;
   }

  // compute is an upper bound on the max quantity of flow (of any commodity)
  // on arc i: if capacities can increase indefinitely, the only bound is
  // the total quantity of flow in the graph

  FNumber Ui = 0;
  if( IncUjk < Inf<double>() )
   for( Index k = NComm ; k-- ; )
    if( U[ k ][ i ] == Inf<double>() )
     Ui += tmpv[ k ];
    else
     Ui += min( tmpv[ k ] , U[ k ][ i ] + IncUjk );
  else
   Ui = maxU;

  // note: when e.g. the mutual capacity and the sum of all the individual
  // capacities of an arc are identical, the arc is marked as "inactive"; this
  // is an arbitrary choice, since one could as well keep it and eliminate all
  // the individual capacities

  if( UTot[ i ] >= Ui - DecUk )
   UTot[ i ] = Ui;
  else
   Active[ NCnst++ ] = i;

  }  // end for( i )

 if( NCnst < NArcs )
  Active[ NCnst ] = Inf<Index>();

 // now a squeeze of single-commodity capacities is attempted, and SPTs are -
 // definitively recognized - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // meanwhile, construct the "active" individual capacity constraints

 for( Index k = 0 ; k < NComm ; k++ ) {

  ActiveK[ k ].clear();
  ActiveK[ k ].resize( NArcs );

  Index cnt = 0;  // active individual capacity constraints

  Index count1 = 0;
  Index count2 = 0;
  for( Index i = 0 ; i < NArcs ; i++ ) {
   bool Ai = ( Active[count1] == i );    // true if arc i is "active"
   if( Ai )
    count1++;

   if( C[ k ][ i ] == Inf<double>() )  // a non-existent arc
    continue;

   if( ( ! IncUjk ) && ( ! U[ k ][ i ] ) ) {
    // an arc that can be declared non-existent by its capacity
    // (that will never increase)
    C[ k ][ i ] = Inf<double>();
    continue;
    }

   if( DecUjk < Inf<double>() ) {
    // if individual capacities cannot decrease forever, then the
    // individual capacity constraint of some existing arc can be
    // declared redundant

    if( U[ k ][ i ] >= tmpv[ k ] + DecUjk ) {
     // the constraint is redundant because there will never be that much
     // flow in the graph

     U[ k ][ i ] = min( tmpv[ k ] , UTot[ i ] );  // give it a "nice"
     continue;                                    // finite value anyway
     }

    if( ( IncUk < Inf<double>() ) &&
	( Ai && ( U[ k ][ i ] >= UTot[ i ] + IncUk + DecUjk ) ) ) {
     // if mutual capacities cannot increase forever, some individual
     // capacities may be declared redundant by the mutual capacity
     // note that the mutual capacity of an arc can be used to declare
     // that the individual capacity is redundant only if the arc is
     // "active", as "inactive" arcs precisely mean that no mutual
     // capacity constraint is imposed on them (i.e., the value of
     // UTot[ i ] is not really meaningful and can be ignored)

     U[ k ][ i ] = UTot[ i ];  // give it a "nice" finite value anyway
     continue;
     }
    }

   ActiveK[ k ][ count2++ ] = i;
   cnt++;

   }  // end for( i )

  ///if( ( ! cnt ) && ( srck[ k ] == 1 ) )
  /// PT[ k ] = kSPT;

  NamesK[ k + 1 ] = NamesK[ k ] + cnt;

  if( cnt >= NArcs )   // all individual capacity constraints are active
   ActiveK[ k ].clear();
  else { // some are active, some are not
   ActiveK[ k ].resize(cnt + 1);
   ActiveK[ k ][ cnt ] = Inf<Index>();
   }
  }   // end for( k )

 if( NCnst >= NArcs )
  Active.clear();

 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // find and eliminate redundancies in the data structures- - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 // examine B[] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 BIsCpy.resize( NComm , bool( false ) );

 bool cpy = false;
 for( Index k = 1 ; k < NComm ; k++ )
  for( Index i = 0 ; i < k ; i++ )
   if( ( ! BIsCpy[ i ] ) && ( B[ k ] == B[ i ] ) ) {
    BIsCpy[ k ] = cpy = true;
    B[ k ].resize( B[ i ].size() );
    std::copy( B[ i ].begin() , B[ i ].end(), B[ k ].begin() );
    break;
    }

 if( ! cpy )
  BIsCpy.clear();

 // examine U[] and UTot- - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 UIsCpy.resize( NComm , bool( false ) );

 cpy = false;
 if( U[ 0 ] == UTot ) {
  UIsCpy[ 0 ] = cpy = true;
  U[ 0 ].resize( UTot.size() );
  std::copy( UTot.begin() , UTot.end(), U[ 0 ].begin() );
  }

 for( Index k = 1 ; k < NComm ; k++ )
  for( Index i = 0 ; i < k ; i++ )
   if( ( ! UIsCpy[ i ] ) && ( U[ k ] == U[ i ] ) ) {
    UIsCpy[ k ] = cpy = true;
    U[ k ].resize( U[ i ].size() );
    std::copy( U[ i ].begin() , U[ i ].end(), U[ k ].begin() );
    break;
    }

 if( ! cpy )
  UIsCpy.clear();

 // examine C[] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 CIsCpy.resize( NComm , bool( false ) );

 cpy = false;
 for( Index k = 1 ; k < NComm ; k++ )
  for( Index i = 0 ; i < k ; i++ )
   if( ( ! CIsCpy[ i ] ) && ( C[ k ] == C[ i ] ) ) {
    CIsCpy[ k ] = cpy = true;
    C[ k ].resize( C[ i ].size() );
    std::copy( C[ i ].begin() , C[ i ].end(), C[ k ].begin() );
    break;
    }

 if( ! cpy )
  CIsCpy.clear();

 // cleanup - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 srck.clear();
 tmpv.clear();

 }  // end( MMCFBlock::PreProcess )

/*--------------------------------------------------------------------------*/

void MMCFBlock::serialize( netCDF::NcGroup & group ) const
{

 // TODO: the method has to be implemented!!

 }  // end( MMCFBlock::serialize )

/*--------------------------------------------------------------------------*/

void MMCFBlock::deserialize( netCDF::NcGroup & group )
{

 // TODO: the method has to be implemented!!

 }  // end( MMCFBlock::deserialize )  - - - - - - - - - - - - - - - - - - - -

/*--------------------------------------------------------------------------*/

void MMCFBlock::CmnIntlz( void )
{
 // some initializations that are common to all the constructors- - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 StrtNme = 1;
 Active.clear();
 DrctdPrb = true;

 PT.resize( NComm );

 Index k = NComm;
 for( ; k-- ; )
  PT[ k ] = kMCF;

 CIsCpy.clear();
 UIsCpy.clear();
 DIsCpy.clear();
 BIsCpy.clear();

 // find arcs that might have individual capacity constraints - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 ActiveK.resize( NComm );
 NamesK.resize( NComm + 1 );

 for( NamesK[ 0 ] = NCnst , k = 0 ; k < NComm ; k++ ) {
  // first, count how many active constraints there are - - - - - - - - - - -

  Index i = 0;
  Index cnt = 0;
  // cCRow Ck = C[ k ];
  // cFRow Uk = U[ k ];

  for( ; i < NArcs ; i++ )
   if( ( C[ k ][ i ] < Inf<CNumber>() )
	   && ( U[ k ][ i ] < Inf<FNumber>() ) )
	cnt++;

  // second, (if necessary) construct the actual vector of indices - - - - - -

  NamesK[ k + 1 ] = NamesK[ k ] + cnt;

  if( cnt < NArcs ) {
   ActiveK[ k ].resize( cnt + 1 );
   for( i = 0 ; i < NArcs ; i++ )
    if( ( C[ k ][ i ] < Inf<CNumber>() )
    	 && ( U[ k ][ i ] < Inf<FNumber>() ) )
     ActiveK[ k ].push_back( i );

   ActiveK[ k ].push_back( Inf<Index>() );
   }
  else
   ActiveK[ k ].clear();

  }  // end( for( k ) )

 }  // end( CmnIntlz )

/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*--------------------- End File MMCFBlock.cpp ----------------------------*/
/*--------------------------------------------------------------------------*/
