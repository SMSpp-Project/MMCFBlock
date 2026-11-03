/*--------------------------------------------------------------------------*/
/*-------------------------- File MMCFBlock.cppm ---------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Interface of the C++20 module smspp.mmcfblock, which exports every name
 * that the headers of MMCFBlock declare at namespace scope, together with the
 * modules it depends on; a translation unit can then write
 *
 *     import smspp.mmcfblock;
 *
 * in place of the #include of the headers. The macros (e.g., those of the
 * factory in SMSFactory.h) are not exported by a module, so a file that uses
 * them still includes the header that defines them.
 *
 * This file is written by SMS++/cmake/smspp_module_exports.py, which reads
 * the names from the AST of the headers: it is not to be edited by hand.
 *
 * \author Antonio Frangioni \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \author Enrico Gorgone \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \author Francesca Demelas \n
 *         Laboratoire d'Informatique de Paris Nord \n
 *         Universite' Sorbonne Paris Nord \n
 *
 * \copyright &copy; by Antonio Frangioni, Enrico Gorgone, Francesca Demelas
 */
/*--------------------------------------------------------------------------*/
/*------------------------- GLOBAL MODULE FRAGMENT -------------------------*/
/*--------------------------------------------------------------------------*/

module;

#include "MMCFNetworkDesignBlock.h"

/*--------------------------------------------------------------------------*/
/*----------------------------- MODULE PURVIEW -----------------------------*/
/*--------------------------------------------------------------------------*/

export module smspp.mmcfblock;

export import smspp.binaryknapsackblock;
export import smspp.mcfblock;

export namespace SMSpp_di_unipi_it {

 // MMCFBlock.h
 using SMSpp_di_unipi_it::CMultiVector;
 using SMSpp_di_unipi_it::CNumber;
 using SMSpp_di_unipi_it::FMultiVector;
 using SMSpp_di_unipi_it::FNumber;
 using SMSpp_di_unipi_it::MMCFBlock;
 using SMSpp_di_unipi_it::MultiSubset;
 using SMSpp_di_unipi_it::Vec_Bool;
 using SMSpp_di_unipi_it::Vec_CNumber;
 using SMSpp_di_unipi_it::Vec_FNumber;
 using SMSpp_di_unipi_it::c_RHSValue;

 // MMCFNetworkDesignBlock.h
 using SMSpp_di_unipi_it::MMCFNetworkDesignBlock;
}

/*--------------------------------------------------------------------------*/
/*------------------------ End File MMCFBlock.cppm -------------------------*/
/*--------------------------------------------------------------------------*/
