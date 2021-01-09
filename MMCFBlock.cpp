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

#include "SMSTypedefs.h"
#include <math.h>

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;
using namespace std;

using namespace SMSpp_di_unipi_it;

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

template< typename T>
static void read_T( istream & iStrm , T & t )
{
 iStrm >> eatcomments;

 int c = iStrm.peek();

 switch( c ) {
  case 'I' :
  case 'i' : t = Inf<T>();
             break;
  case '-' : iStrm.get();
             read_T( iStrm , t );
             t = - t;
             return;
  case 'M' :
  case 'm' : t = -Inf<T>();
             break;
  default :  iStrm >> t;
             return;
  }

 do { c = iStrm.get(); c = iStrm.peek();
  } while( ( c != iStrm.widen( ' ' ) ) &&
	   ( c != iStrm.widen( '\n' ) ) &&
	   ( c != iStrm.widen( '\t' ) ) );

 }

/*--------------------------------------------------------------------------*/

static inline int read_int( istream & iStrm )
{
 int d;
 read_T( iStrm , d );
 return( d );
 }

/*--------------------------------------------------------------------------*/

static inline double read_dbl( istream & iStrm )
{
 double d;
 read_T( iStrm , d );
 return( d );
 }

/*--------------------------------------------------------------------------*/

static inline string read_string( istream & iStrm )
{
 iStrm >> eatcomments;
 string s;
 int c = iStrm.peek();
 iStrm >> s;
 return( s );
 }

/*--------------------------------------------------------------------------*/

static inline char read_char( istream & iStrm )
{
 char d;
 read_T( iStrm , d );
 return( d );
 }

/*--------------------------------------------------------------------------*/
/*----------------------------- STATIC MEMBERS -----------------------------*/
/*--------------------------------------------------------------------------*/

// register MMCFBlock to the Block factory
SMSpp_insert_in_factory_cpp_1( MMCFBlock );

/*--------------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/

void MMCFBlock::generate_abstract_constraints( Configuration * stcc ) {
 for( auto blck : v_Block )
  blck->generate_abstract_constraints();

 // count number of nonzeroes in each constraint, i.e., #FS( i ) + #BS( i )
 Subset count( get_NArcs() );

 if( ! ( AR & HasMutual ) ) {

  MCs.resize( get_NArcs() );

  // initialize the vectors of coefficients, and reset count[]
  std::vector< LinearFunction::v_coeff_pair > coeffs( get_NArcs() );

  for( Index j = 0 ; j < get_NArcs() ; ++j ) {
   coeffs[ j ].resize( NComm );
   count[ j ] = 0;
   }

  for( Index k = 0 ; k < get_NComm() ; k++ ) {
   auto v_s_Variable = v_mcf[ k ]->get_static_variables();
   for( Index j = 0 ; j < get_NArcs() ; ++j ) {
	ColVariable var = boost::any_cast<ColVariable>( v_s_Variable[j] );
    coeffs[ j ][ k ] = std::make_pair( &var , double( 1 ) );
    }

   }

  // generate the mutual capacity constraints  - - - - - - - - - - - - - - -
  // each constraint is an inequality, i.e., RHS = UTot[ j ]

  for( Index j = 0 ; j < get_NArcs() ; ++j ) {
   MCs[ j ].set_rhs( UTot[ j ] );
   MCs[ j ].set_function( new LinearFunction( std::move( coeffs[ j ] ) , 0 ) );
   }

  add_static_constraint( MCs );

  AR |= HasMutual;
  }

 } // end( MMCFBlock::generate_abstract_constraints() )  - - - - - - - - - - -

/*--------------------------------------------------------------------------*/

MMCFBlock::~MMCFBlock() {

 for( Index k = 0 ; k< NComm ; k++ )
  delete v_mcf[ k ];

 } // end destructor   - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*--------------------------------------------------------------------------*/
/*-------------------------- PROTECTED METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/

void MMCFBlock::print( std::ostream &output ) const
{
 output << "MMCFBlock with " << NArcs * NComm << " vars " << std::endl;
 }

/*--------------------------------------------------------------------------*/

void MMCFBlock::load( std::istream &input )
{

 instance_type = read_char( input );
 /* instance_name = read_string( input );

 char * cstr;
 cstr = new char[ instance_name.size()+1 ];
 strcpy (cstr, instance_name.c_str());    //here str.c_str() generate null terminated char* pointer

 MakeMMCF( cstr  , instance_type ); */

 }

/*--------------------------------------------------------------------------*/

void MMCFBlock::MakeMMCF( const char *const filename , char filetype )
{
 // check parameters- - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 if( ( filetype != 'm' ) && ( filetype != 'p' ) && ( filetype != 'd' )
	 && ( filetype != 'o' ) && ( filetype != 'u' ) && ( filetype != 's' )
	 && ( filetype != 'c' ) )
  throw( std::invalid_argument( "invalid file type" ) );

 bool FourFiles = ( ( filetype != 's' ) && ( filetype != 'c' ) );

 // in principle there is no "extra" stuff- - - - - - - - - - - - - - - - - -

 NXtrV = NXtrC = 0;
 IdxBeg.resize(0);
 CoefIdx.resize(0);
 CoefVal.resize(0);

 // reading general informations- - - - - - - - - - - - - - - - - - - - - - -
 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 c_Index l = strlen( filename );
 char *Name = new char[ l + 5 ];  // temporary string containing the constant
 strcpy( Name , filename );             // part of the pathname + space for `.XXX'

 if( FourFiles )
  strcpy( Name + l , ".nod" );

 ifstream inFile( Name );
 if( ! inFile.is_open() )
  throw( std::invalid_argument( "can't open file" ) );

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

 if( FourFiles )  // preparing to read the supply file
  if( filetype == 'u' ) {
   strcpy( Name + l , ".od" );
   filetype = 'd';
   }
  else
   strcpy( Name + l , ".sup" );

 // determining the actual number of commodities for (OSP) or (ODS)- - - - - -
 // formulations: in the first case, a commodity is a pair ( product , - - - -
 // origin ), while in the second case it is a triplet ( product ,-  - - - - -
 // origin , destination ) - - - - - - - - - - - - - - - - - - - - - - - - - -

 if( ( filetype == 'd' ) || ( filetype == 'o' ) ) {
  StartOfK.resize( NumProd + 1 , 0 );

  inFile.clear();        // ensure failbits are not left dirty
  inFile.open( Name );   // commodities can be told from supplies
  if( ! inFile.is_open() )
   throw( std::invalid_argument( "can't open file" ) );

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
   throw( std::invalid_argument( "can't open file" ) );
  }

 switch( filetype ) {

 case( 's' ): // Canadian format- - - - - - - - - - - - - - - - - - - - - - -
 {            //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // allocate the data structures for "extra" things- - - - - - - - - - - - -

  C[ NComm ].resize( NXtrV = NArcs );

  U[ NComm ].resize( NArcs , FNumber( 0 ) ); // "extra" variables
  U[ NComm + 1 ].resize( NArcs , FNumber( 1 ) ); // are in the ...
                                             // ... [0, 1] range
  NInt[ NComm ] = NArcs;                     // ... and integer

  for( Index i = 0 ; i < NArcs ; i++ ) {  // read arc-related info- - - - - -
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

   inFile >> B[ --k ][ --i ];
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
   for( Index i = 0 ; i < NNodes ; )
    inFile >> B[ k ][ i++ ];

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
     B[ k++ ][ node - 1 ] = - flow;
   else
    B[ comm - 1 ][ node - 1 ] = - flow;
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
     B[ comm ][ dest - 1 ] = flow;
    else
     B[ comm ][ origin - 1 ] = - flow;
    }
   else
    if( origin < 0 )
     for( Index i = NumProd ; i-- ; )
      B[ i ][ dest - 1 ] = flow;
    else
     for( Index i = NumProd ; i-- ; )
      B[ i ][ origin - 1 ] = - flow;

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
     B[ i ][ origin - 1 ] = - flow;
    else
     B[ i ][ dest - 1 ] = flow;
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
      B[ i ][ origin - 1 ] = - flow;
     else
      B[ i ][ dest - 1 ] = flow;

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

    B[ TempIdx[ comm ] ][ dest - 1 ] = flow;
    B[ TempIdx[ comm ]++ ][ origin - 1 ] = - flow;
    }
   else
    for( Index i = NumProd ; i-- ; ) {
     Origins[ TempIdx[ i ] ] = origin;
     Destins[ TempIdx[ i ] ] = dest;

     B[ TempIdx[ i ] ][ dest - 1 ] = flow;
     B[ TempIdx[ i ]++ ][ origin - 1 ] = - flow;
     }
    }  // end while( ! eof() )
   }   // end default()- - - - - - - - - - - - - - - - - - - - - - - - - - - -
  }    // end switch( FT ) - - - - - - - - - - - - - - - - - - - - - - - - - -
       //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 inFile.close();

 if( FourFiles ) {
  // continue only in the multi-file formats - - - - - - - - - - - - - - - - -

  UTot.resize( NArcs , Inf<FNumber>() );

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // now the though part: reading arc infos - - - - - - - - - - - - - - - - -
  // (again, this part is splitted) - - - - - - - - - - - - - - - - - - - - -

  strcpy( Name + l , ".arc" );

  inFile.clear();        // ensure failbits are not left dirty
  inFile.open( Name );
  if( ! inFile.is_open() )
   throw( std::invalid_argument( "can't open file" ) );

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
   throw( std::invalid_argument( "can't open file" ) );

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

 // initialize the children - - - - - - - - - - - - - - - - - - - - - - - - -

 v_mcf.resize(NComm);
 for( Index k = 0 ; k< NComm ; k++ ) {
  if( PT[ k ] != kMCF )
   throw( std::logic_error( "MCF is implemented only " ) );

  Block *sMCFblock = Block::new_Block( "MCFBlock" );

  v_mcf[ k ] = static_cast<MCFBlock *>( sMCFblock );
  v_mcf[ k ]->load( NNodes , NArcs , Startn , Endn ,
		   U[ k ] , C[ k ] , B[ k ] );

  }

 }  // end( Graph( char* , char ) )

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
