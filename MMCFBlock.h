/*--------------------------------------------------------------------------*/
/*---------------------------- File MMCFBlock.h ----------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file for the *concrete* class MMCFBlock, which implements the
 * Block concept [see Block.h] for a Multicommodity Min Cost Flow problem.
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
/*----------------------------- DEFINITIONS --------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef __MMCFBlock
 #define __MMCFBlock  /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "Block.h"
#include "MCFBlock.h"
#include "BinaryKnapsackBlock.h"
#include "ColVariable.h"
#include "FRowConstraint.h"
#include "Configuration.h"

/*--------------------------------------------------------------------------*/
/*--------------------------- NAMESPACE ------------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it
{
/*--------------------------------------------------------------------------*/
/*----------------------- MMCFBlock-RELATED TYPES --------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Public Types
 *
 * "Import" basic types from Function and C05Function.
 *
 *  @{ */

 using CNumber = MCFBlock::CNumber;
 using Vec_CNumber = MCFBlock::Vec_CNumber;
 using FNumber = MCFBlock::FNumber;
 using Vec_FNumber = MCFBlock::Vec_FNumber;

 using FMultiVector = std::vector< Vec_FNumber >;
 using CMultiVector = std::vector< Vec_CNumber >;
 using MultiSubset = std::vector< Block::Subset >;

 using Vec_Bool = std::vector< bool >;

/** @}  end( types ) */
/*--------------------------------------------------------------------------*/
/*------------------------------- CLASSES ----------------------------------*/
/*--------------------------------------------------------------------------*/
/** @defgroup MMCFBlock_CLASSES Classes in MMCFBlock.h
 *  @{ */

/*--------------------------------------------------------------------------*/
/*-------------------------- CLASS MMCFBlock ------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// Implementation of a simple MMCF Block concept.

class MMCFBlock : public Block
{
/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

public:

/*--------------------------------------------------------------------------*/
/*---------------------------- PUBLIC TYPES --------------------------------*/
/*--------------------------------------------------------------------------*/

 enum MCFType { kMCF , kSPT };

/*--------------------------------------------------------------------------*/
/*--------------------- PUBLIC METHODS OF THE CLASS ------------------------*/
/*--------------------------------------------------------------------------*/
/*---------------------------- CONSTRUCTOR ---------------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Constructor and Destructor
 *  @{ */

 /// constructor of MMCFBlock
 /** Constructor of MMCFBlock. It accepts a pointer to the father
  * Block, which can be of any type. */

 MMCFBlock( Block *father = nullptr ) : Block( father ) , AR( 0 ) { }

/*--------------------------------------------------------------------------*/
 /// destructor of MMCFBlock
 /** Destructor of MMCFBlock. . */

 virtual ~MMCFBlock();

/*@} -----------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Other initializations
 *  @{ */

 /// loads the instance from the given file of the given type

 void load( const char *const filename , char filetype );

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// simplifies the problem
 /** Performs various pre-processing of the data, trying to make the instance
  * more easily solvable. The parameters to be given are the following:
  *
  * IncUk , DecUk   => (>= 0) upper bounds on the increase and decrease of the
  *                    mutual capacities: may be Inf<FNumber>() if unknown;
  *
  * IncUjk , DecUjk => (>= 0) same as above for single-commodity capacities;
  *
  * ChgDfct         => (>= 0) upper bound on the maximum change, in absolute
  *                    value, of the node deficits: it must be a finite number,
  *                    since it is used to generate "loose" but finite
  *                    individual capacities for arcs that have none;
  *
  *   DecCsts         => (>= 0) upper bound on the decrease of arc Costs: must
  *                      be < Inf<CNumber>().
  *
  * Giving tight bounds (0 is the best, obviously) may cause the preprocessor
  * to find more redundant coupling constraints, to squeeze down individual
  * arc capacities, to remove more unused arcs and in general to do a better
  * preprocessing; for instance, IncUjk == 0 allows PreProcess() to declare
  * un-existent (set the cost to Inf<CNumber>()) any arc with 0 individual
  * capacity.
  *
  * For all k such that, after the pre-processing, the graph has only a source
  * and no (existing) arcs have a "real" capacity, the type of the subproblem
  * is set to kSPT: all other problem types are left unchanged.
  *
  * Important note: in order for PreProcess() to work, it has to be able to
  * guess at least an upper bound on the maximum quantity of each commodity
  * in the graph. In order to do that, *all arcs* with potentially *negative
  * costs* (ChgCsts is used to estimate that) must have a *finite capacity*.
  *
  * PreProcess() will also look for redundancy in the data structures (e.g.
  * identical costs/deficits/individual capacities for some commodities) and
  * eliminate them, thus possibly saving some memory.
  *
  * It can be called *only once*. The ideal would be that it is automatically
  * called after load(), deserialize() ecc. but this would not allow to set
  * the proper parameters, therefore it has to be done independently (if
  * ever). */

 void PreProcess( FNumber IncUk = 0 , FNumber DecUk = 0 ,
		  FNumber IncUjk = 0 , FNumber DecUjk = 0 ,
		  FNumber ChgDfct = 0 , CNumber DecCsts = 0 );

/*--------------------------------------------------------------------------*/
 /// generate the "abstract representation" of the Variable of the Block
 /** This method generates the "abstract representation" of the Variable of
  * the MMCFBlock, and in fact it decides which formulation of the MMCF
  * problem is implemented. This is controlled by the parameter stvv. If stvv
  * is not nullptr and it is a SimpleConfiguration< int >, or if
  * f_BlockConfig->f_static_variables_Configuration is not nullptr and it is a
  * SimpleConfiguration< int >, then the f_value (an int) dictates which
  * MMCF formulation as follows:
  *
  * - [currently all values]: the standard flow formulation in which k
  *   MCFBlock sub-Block are constructed, one for each commodity, and the
  *   linking constraints are handled in the father MMCFBlock;
  *
  * - [other ones to follow].
  */

 void generate_abstract_variables( Configuration * stvv = nullptr ) override;

/*--------------------------------------------------------------------------*/

 void generate_abstract_constraints( Configuration * stcc = nullptr )
  override;

/*--------------------------------------------------------------------------*/
 /*!! not needed yet, the version of Block suffices so far
 void generate_objective( Configuration * objc = nullptr ) override;
 !!*/

/**@} ----------------------------------------------------------------------*/
/*-------------- Methods for reading the data of the MCFBlock --------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for reading the data of the MMCFBlock
  *  @{ */

 char get_filetype( void ) const { return( instance_type ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 std::string get_filename( void ) const { return( instance_name ); }

/*--------------------------------------------------------------------------*/
 /// get the number of nodes

 Index get_NNodes( void ) const { return( NNodes ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// get the number of arcs

 Index get_NArcs( void ) const { return( NArcs ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// get the number of commodities

 Index get_NComm( void ) const { return( NComm ); }

/*@}------------------------------------------------------------------------*/
/*-------------------- PROTECTED PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 protected:

/*--------------------------------------------------------------------------*/
/*-------------------------- PROTECTED METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Protected methods for inserting and extracting
    @{ */

 /// print the MMCFBlock on an ostream with the given verbosity

 virtual void print( std::ostream &output ) const override;

/*--------------------------------------------------------------------------*/
 ///< load the MMCFBlock out of an istream
 /**< Load the MMCFBlock out of an istream. The format is: ...
  *
  */

 void load( std::istream &input ) override { // TODO: implement
  }

/*--------------------------------------------------------------------------*/

 void serialize( netCDF::NcGroup & file ) const override;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
 /// extends Block::deserialize( netCDF::NcGroup )
 /** Extends Block::deserialize( netCDF::NcGroup ) to the specific format of
  * a MMCFBlock.  */

 void deserialize( netCDF::NcGroup & group );

/*--------------------------------------------------------------------------*/
 /** called at the end of any constructor, does some initializations that are
  common to them all: it is "protected" for allowing derived classes that
  use the "void" constructor to call it. */

 void CmnIntlz( void );

/*@}------------------------------------------------------------------------*/
/*--------------------------- PROTECTED FIELDS  ----------------------------*/
/*--------------------------------------------------------------------------*/

 unsigned char AR;   ///< bit-wise coded: what abstract is there

 static constexpr unsigned char HasVar = 1;
 ///< first bit of AR == 1 if the formulation has been chosen already

 static constexpr unsigned char HasMutual = 2;
 ///< second bit of AR == 1 if the Mutual Constraints has been constructed

 Index NXtrV;         ///< Number of "extra" variables
 Index NXtrC;         ///< Number of "extra" constraints

 Subset IdxBeg;       ///< Description of "extra" constraints: start
 Subset CoefIdx;      ///< Description of "extra" constraints: indices
 Vec_CNumber CoefVal; ///< Description of "extra" constraints: values

 Index NNodes;        ///< Number of nodes
 Index NArcs;         ///< Number of arcs
 Index NComm;         ///< Number of commodities
 Index NCnst;         ///< Number of arcs with mutual capacity constraints

 CMultiVector C;      ///< Matrix of the arc costs
 FMultiVector U;      ///< Matrix of the arc upper capacities
 FMultiVector B;      ///< Matrix of the node deficits
 FMultiVector I;      ///< Matrix of the integrality constraints for the variables

 Vec_FNumber UTot;    ///< Vector of mutual capacities

 Subset Startn;       ///< Topology of the graph: starting nodes
 Subset Endn;         ///< Topology of the graph: ending nodes
 Subset NInt;         ///< Number of integer-valued variables
 MultiSubset WIsInt;  ///< Which of the variables are integer-valued

 Index StrtNme;       ///< The "name" of the first node
 Subset NamesK;    ///< The dual multipliers relative to commodity K
                      ///< start with NamesK[ k ] and end with
                      ///< NamesK[ k + 1 ]
 Subset Active;       ///< Set of the arcs for which a mutual capacity
                      ///< constraint is defined
 MultiSubset ActiveK; ///< Like Active for individual capacities
 bool DrctdPrb;       ///< true if the problem is directed
 std::vector<MCFType> PT;  ///< type of flow subproblem
 // std::vector< MCFBlock * > v_mcf;
   ///< the vector of (pointers to) the components of the sum function

 Vec_Bool CIsCpy;     ///< true for each row of C[] that is a copy of another
 Vec_Bool UIsCpy;     ///< true for each row of U[] that is a copy of another
 Vec_Bool BIsCpy;     ///< true for each row of B[] that is a copy of another
 Vec_Bool DIsCpy;     ///< true for each row of D[] that is a copy of another
 
 bool FlowRelaxation; ///< true if we use the flow relaxation and false if we use the knapsack relaxation

 std::vector<FRowConstraint> MCs;  ///< the static mutual capacity constrs.

 char instance_type;
 std::string instance_name;

/*--------------------------------------------------------------------------*/
/*--------------------- PRIVATE PART OF THE CLASS --------------------------*/
/*--------------------------------------------------------------------------*/

 private:


/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE FIELDS ------------------------------*/
/*--------------------------------------------------------------------------*/

 SMSpp_insert_in_factory_h;        // insert it in the Block factory

/*--------------------------------------------------------------------------*/

};  // end( class( MMCFBlock ) )

/*--------------------------------------------------------------------------*/

/*@}  end( group( MMCFBlock_CLASSES ) ) -----------------------------------*/
/*--------------------------------------------------------------------------*/

 }  // end( namespace SMSpp_di_unipi_it )

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#endif  /* MMCFBlock.h included */

/*--------------------------------------------------------------------------*/
/*--------------------- End File MMCFBlock.h ------------------------------*/
/*--------------------------------------------------------------------------*/
