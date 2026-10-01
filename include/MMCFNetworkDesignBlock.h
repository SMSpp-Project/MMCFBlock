/*--------------------------------------------------------------------------*/
/*---------------------- File MMCFNetworkDesignBlock.h ---------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file for the class MMCFNetworkDesignBlock, a Block for the
 * (capacitated, fixed-charge) Multicommodity Min-Cost Flow Network Design
 * problem: it holds the binary design Variable of the arcs, and the flow
 * part is a MMCFBlock.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- DEFINITIONS --------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef __MMCFNetworkDesignBlock
 #define __MMCFNetworkDesignBlock
                      /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "MMCFBlock.h"

#include <list>

/*--------------------------------------------------------------------------*/
/*--------------------------- NAMESPACE ------------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it
{

/*--------------------------------------------------------------------------*/
/*---------------------- CLASS MMCFNetworkDesignBlock ----------------------*/
/*--------------------------------------------------------------------------*/
/// a Block for the Multicommodity Min-Cost Flow Network Design problem
/** The MMCFNetworkDesignBlock represents the fixed-charge capacitated
 * Multicommodity Min-Cost Flow Network Design problem
 * \f[
 *  \min \sum_{ j \in A } F_j y_j + \sum_{ k \in K } \sum_{ j \in A }
 *       c^k_j x^k_j
 * \f]
 * \f[
 *  E x^k = b^k \;\; k \in K \;, \quad
 *  \sum_{ k \in K } x^k_j \leq \bar{u}_j y_j \;\; j \in A \;, \quad
 *  0 \leq x^k_j \leq \bar{u}^k_j y_j \;\; k \in K \,,\, j \in A \;, \quad
 *  y \in \{ 0 , 1 \}^{ |A| }
 * \f]
 * i.e., an arc can only carry flow if it is opened, paying its fixed cost
 * F_j. The data are those of a MMCFBlock (with its fixed costs F, which the
 * MMCFBlock itself does not use), which the MMCFNetworkDesignBlock owns;
 * \f$ \bar{u}_j \f$ and \f$ \bar{u}^k_j \f$ are the capacities that
 * MMCFBlock::get_design_capacity() gives.
 *
 * The MMCFNetworkDesignBlock only defines the design Variable y (and their
 * part of the Objective), and it has two formulations, chosen by the int of
 * the Configuration of its static Variable [see
 * generate_abstract_variables()]:
 *
 * - 0, "monolithic": the MMCFBlock is the (only) sub-Block, with the flow
 *   structure (one MCFBlock per commodity) and the y as its design Variable
 *   [see MMCFBlock::set_design_variables()], so that the whole problem is
 *   in the abstract representation, e.g., for a MILPSolver;
 *
 * - 1, "Benders": the abstract representation only has y and an epigraph
 *   Variable v of the cost of the flow, and the dynamic Constraint of the
 *   Benders cuts v >= alpha + g' y, which generate_dynamic_constraints()
 *   separates, typically called by the callback of a MILPSolver. The cuts
 *   come from a "hidden" copy of the MMCFBlock with slack arcs [see
 *   MMCFBlock::get_slack_copy()], hence feasible for every y, which is not a
 *   sub-Block: at the current y, the mutual capacities of its arcs are set
 *   to \f$ \bar{u}_j y_j \f$ and the individual capacities of each MCFBlock
 *   to \f$ \bar{u}^k_j y_j \f$, which is what reaches the Solver of each
 *   commodity as a change of the capacities (closing the arcs with y_j = 0),
 *   and the Solver registered to the copy (typically a LagrangianDualSolver
 *   that relaxes the mutual capacities, with a MCF Solver per commodity) is
 *   run.
 *
 * The Benders cut is obtained by weak duality, and hence it is valid
 * whatever the precision of that Solver: given the multipliers lambda >= 0
 * of the mutual capacity constraints and the node potentials pi^k of each
 * commodity that the Solver writes in the copy, taking for each arc the
 * dual multiplier of its capacity mu^k_j = min( 0 , r^k_j ), r^k_j being
 * its reduced cost at the costs c^k + lambda, gives a feasible solution of
 * the dual of the flow problem at every y, whose value
 * \f[
 *  \sum_{ k } ( b^k \pi^k + \sum_j \mu^k_j \bar{u}^k_j y_j ) -
 *  \sum_j \lambda_j \bar{u}_j y_j
 * \f]
 * is linear in y and below the cost of the flow at every y. The better the
 * Solver has converged, the closer the cut is to the cost of the flow at
 * the current y.
 *
 * The extra Configuration of the BlockConfig gives, in the Benders
 * formulation, how the hidden copy is solved: a BlockSolverConfig, applied
 * to it, or a SimpleConfiguration< std::pair< Configuration * ,
 * Configuration * > > whose first element is a BlockConfig and the second a
 * BlockSolverConfig, both applied to it. */

class MMCFNetworkDesignBlock : public Block
{
/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 public:

/*--------------------------------------------------------------------------*/
/*---------------------------- PUBLIC TYPES --------------------------------*/
/*--------------------------------------------------------------------------*/

 /// the formulations of the MMCFNetworkDesignBlock
 enum formulation_type {
  kMonolithic = 0 ,  ///< the MMCFBlock is the sub-Block
  kBenders = 1       ///< y, v and the Benders cuts of a hidden copy
  };

/*--------------------------------------------------------------------------*/
/*--------------------- CONSTRUCTOR AND DESTRUCTOR -------------------------*/
/*--------------------------------------------------------------------------*/

 /// constructor, taking the (pointer to the) father Block
 explicit MMCFNetworkDesignBlock( Block * father = nullptr )
  : Block( father ) {}

/*--------------------------------------------------------------------------*/
 /// destructor

 virtual ~MMCFNetworkDesignBlock() { guts_of_destructor(); }

/*--------------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/

 /// loads the instance, in any of the formats of MMCFBlock::load()
 /** Loads the MMCF instance in the MMCFBlock that the
  * MMCFNetworkDesignBlock owns [see MMCFBlock::load()]: the fixed costs
  * of the arcs are those of the instance (the Canad format has them, the
  * PPRN one has not, and they are then all 0). */

 void load( std::istream & input , char frmt = 0 ) override;

 using Block::load;  // the one that takes the name of the file

/*--------------------------------------------------------------------------*/
 /// extends Block::deserialize( netCDF::NcGroup )
 /** The netCDF group of a MMCFNetworkDesignBlock has a sub-group
  * "MMCFBlock", the netCDF group of the MMCFBlock [see
  * MMCFBlock::deserialize()], with the fixed costs F. */

 void deserialize( const netCDF::NcGroup & group ) override;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// extends Block::serialize( netCDF::NcGroup ), as deserialize() reads

 void serialize( netCDF::NcGroup & group ) const override;

/*--------------------------------------------------------------------------*/
 /// generates the design Variable, and the epigraph one in kBenders
 /** The formulation [see formulation_type] is the int of \p stvv, which can
  * be a SimpleConfiguration< int > or a SimpleConfiguration< std::pair<
  * int , double > >, whose double is then the scale of the cost of the
  * slack arcs of the hidden copy [see MMCFBlock::get_slack_copy()], 100 by
  * default; if \p stvv is nullptr, it is taken from the BlockConfig, and
  * the default is kMonolithic. In kBenders, this also constructs the
  * hidden copy and applies to it the extra Configuration of the
  * BlockConfig. */

 void generate_abstract_variables( Configuration * stvv = nullptr )
  override;

/*--------------------------------------------------------------------------*/
 /// generates the constraints: those of the MMCFBlock, or the first cut
 /** In kMonolithic, the constraints of the MMCFBlock, to which \p stcc is
  * passed [see MMCFBlock::generate_abstract_constraints()]; in kBenders,
  * the (empty) group of the Benders cuts, and the cut at y = 1, so that
  * the Objective is bounded from the start. */

 void generate_abstract_constraints( Configuration * stcc = nullptr )
  override;

/*--------------------------------------------------------------------------*/
 /// separates a Benders cut at the current y, in kBenders
 /** If \p dycc (or else the dynamic constraints Configuration of the
  * BlockConfig) is a SimpleConfiguration< std::pair< int , double > >, its
  * double is the relative violation the cut must have to be added, 1e-6 by
  * default. Nothing happens in kMonolithic. */

 void generate_dynamic_constraints( Configuration * dycc = nullptr )
  override;

/*--------------------------------------------------------------------------*/
 /// the Objective: the fixed costs of the open arcs, and v in kBenders

 void generate_objective( Configuration * objc = nullptr ) override;

/*--------------------------------------------------------------------------*/
/*-------------------- Methods for reading the data ------------------------*/
/*--------------------------------------------------------------------------*/

 /// the MMCFBlock with the data of the instance
 [[nodiscard]] MMCFBlock * get_MMCFBlock( void ) const { return( f_mmcf ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// the formulation [see formulation_type]

 [[nodiscard]] int get_formulation( void ) const { return( f_form ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// the hidden copy of the MMCFBlock of the Benders cuts, if any

 [[nodiscard]] MMCFBlock * get_Benders_subproblem( void ) const {
  return( f_sub );
  }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// the design Variable of arc j

 [[nodiscard]] ColVariable * get_y( Index j ) { return( & v_y[ j ] ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// the epigraph Variable of the cost of the flow, in kBenders

 [[nodiscard]] ColVariable * get_v( void ) { return( & v_epi ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// the sense of the Objective, which is minimization

 [[nodiscard]] int get_objective_sense( void ) const override {
  return( Objective::eMin );
  }

/*--------------------------------------------------------------------------*/
/*------------------------ Methods for the Benders cuts --------------------*/
/*--------------------------------------------------------------------------*/

 /// separates a Benders cut at the current value of the y
 /** Solves the hidden copy at the current y [see the class description] and
  * adds the cut if its relative violation at the current (y, v) is more
  * than \p eps_rel times max( | v | , 1 ); a negative \p eps_rel adds it
  * anyway. Returns true if the cut has been added, false if not, or if the
  * Solver of the copy did not end with a solution. */

 bool separate_Benders_cut( double eps_rel = 1e-6 );

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// the lower bound on the cost of the flow given by the last cut
 /** The value at the y of the last separation of the last cut computed
  * (added or not), which is below the cost of the flow at that y, and the
  * value that the Solver of the copy reported there; their difference is
  * how far the cut is from being tight. */

 [[nodiscard]] double get_last_cut_value( void ) const { return( f_cut_val ); }

 [[nodiscard]] double get_last_Solver_value( void ) const {
  return( f_slv_val );
  }

/*--------------------------------------------------------------------------*/
/*------------------------ Methods for Solution ----------------------------*/
/*--------------------------------------------------------------------------*/

 /// returns a ColVariableSolution of the Variable of the whole Block

 Solution * get_Solution( Configuration * solc = nullptr ,
			  bool emptys = true ) override;

/*--------------------------------------------------------------------------*/
/*--------------------- PRIVATE PART OF THE CLASS --------------------------*/
/*--------------------------------------------------------------------------*/

 private:

/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/

 void guts_of_destructor( void );

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// gives the hidden copy the capacities of the current y

 void set_subproblem_capacities( const std::vector< double > & y );

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE FIELDS ------------------------------*/
/*--------------------------------------------------------------------------*/

 MMCFBlock * f_mmcf = nullptr;   ///< the data, the sub-Block in kMonolithic
 MMCFBlock * f_sub = nullptr;    ///< the hidden copy of kBenders

 int f_form = kMonolithic;       ///< the formulation
 double f_slack_scale = 100;     ///< the scale of the cost of the slack arcs

 std::vector< ColVariable > v_y;  ///< the design Variable
 ColVariable v_epi;               ///< the epigraph Variable, in kBenders

 std::list< FRowConstraint > v_cuts;  ///< the Benders cuts

 FRealObjective f_obj;            ///< the Objective

 std::vector< double > v_ylast;   ///< the y the copy has been given last

 double f_cut_val = 0;  ///< the value of the last cut at its y
 double f_slv_val = 0;  ///< the value of the Solver of the copy there

 unsigned char AR = 0;  ///< what of the abstract representation is there

 /// how far from an integer an integer y may be to be rounded to it
 static constexpr double IntTol = 1e-5;

 static constexpr unsigned char HasVar = 1;   ///< the Variable
 static constexpr unsigned char HasCons = 2;  ///< the Constraint
 static constexpr unsigned char HasObj = 4;   ///< the Objective

/*--------------------------------------------------------------------------*/

 SMSpp_insert_in_factory_h;  // insert it in the Block factory

/*--------------------------------------------------------------------------*/

 };  // end( class( MMCFNetworkDesignBlock ) )

/*--------------------------------------------------------------------------*/

}  // end( namespace SMSpp_di_unipi_it )

/*--------------------------------------------------------------------------*/

#endif  /* MMCFNetworkDesignBlock.h included */

/*--------------------------------------------------------------------------*/
/*------------------- End File MMCFNetworkDesignBlock.h --------------------*/
/*--------------------------------------------------------------------------*/
