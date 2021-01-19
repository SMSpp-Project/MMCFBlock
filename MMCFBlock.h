/*--------------------------------------------------------------------------*/
/*--------------------------- File MMCFBlock.h ----------------------------*/
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
 #define __MMCFBlock
                      /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "Block.h"
#include "MCFBlock.h"

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
/*-------------------- SimpleMILPBlock-RELATED TYPES -----------------------*/
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
/** @defgroup SimpleMILPBlock_CLASSES Classes in SimpleMILPBlock.h
 *  @{ */

/*--------------------------------------------------------------------------*/
/*-------------------------- CLASS MMCFBlock ------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// Implementation of a simple MMCF Block concept.

class MMCFBlock : public Block {

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

 virtual void Load( const char *const filename , char filetype );

 virtual void PreProcess( const FNumber IncUk = 0 ,  const FNumber DecUk = 0 ,
		 const FNumber IncUjk = 0 , const FNumber DecUjk = 0 ,
		 const FNumber ChgDfct = 0 , const CNumber DecCsts = 0 );

 virtual void MakeMMCF( void );

/*--------------------------------------------------------------------------*/

 virtual void generate_abstract_constraints( Configuration * stcc = nullptr );

/**@} ----------------------------------------------------------------------*/
/*-------------- Methods for reading the data of the MCFBlock --------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for reading the data of the MMCFBlock
  *  @{ */

 inline char get_filetype( void ) const{ return( instance_type ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 inline std::string get_filename( void ) const{ return( instance_name ); }

/*--------------------------------------------------------------------------*/
  /// get the number of nodes

  inline Index get_NNodes( void ) const { return( NNodes ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /// get the number of arcs

  inline Index get_NArcs( void ) const { return( NArcs ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /// get the number of commodities

  inline Index get_NComm( void ) const { return( NComm ); }

/*@}------------------------------------------------------------------------*/
/*-------------------- PROTECTED PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 protected:

/*--------------------------------------------------------------------------*/
/*-------------------------- PROTECTED METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Protected methods for inserting and extracting
    @{ */

 virtual void print( std::ostream &output ) const override;
 ///< print the MMCFBlock on an ostream with the given verbosity

/*--------------------------------------------------------------------------*/
 void load( std::istream &input ) override;
 ///< load the MMCFBlock out of an istream
 /**< Load the MMCFBlock out of an istream. The format is:
  *
  */

/*--------------------------------------------------------------------------*/

 virtual void serialize( netCDF::NcGroup & file ) const override;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
 /// extends Block::deserialize( netCDF::NcGroup )
 /** Extends Block::deserialize( netCDF::NcGroup ) to the specific format of
  * a MMCFBlock.  */

 virtual void deserialize( netCDF::NcGroup & group );

/*--------------------------------------------------------------------------*/
 /** called at the end of any constructor, does some initializations that are
  common to them all: it is "protected" for allowing derived classes that
  use the "void" constructor to call it. */

 void CmnIntlz( void );

/*@}------------------------------------------------------------------------*/
/*--------------------------- PROTECTED FIELDS  ----------------------------*/
/*--------------------------------------------------------------------------*/

 unsigned char AR;   ///< bit-wise coded: what abstract is there

 static constexpr unsigned char HasMutual = 1;
 ///< first bit of AR == 1 if the Mutual Constraints has been constructed

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
