/* rendera/test/numerics.C */

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdint.h>
#include <typeinfo>

#include "Math.H"

#define FAST_SIGN(v) ( 1 | ( v >> ( sizeof(v) * CHAR_BIT - 1)))

namespace
{
    template< typename T >
    inline
    int
    _sign( T const&v )
    {
        static int const n( sizeof( T ) * CHAR_BIT - 1 );
        return +1 | ( v >> n ) ;
    }
}


int
main( int, char** )
{
    /* FAST_SIGN works for integral types only */
    assert( -1 == FAST_SIGN(char (-42)) );
    assert(  1 == FAST_SIGN(char ( 42)) );
    assert(  1 == FAST_SIGN(char (  0)) );
    assert( -1 == FAST_SIGN(short(-42)) );
    assert(  1 == FAST_SIGN(short( 42)) );
    assert(  1 == FAST_SIGN(short(  0)) );
    assert( -1 == FAST_SIGN(int  (-42)) );
    assert(  1 == FAST_SIGN(int  ( 42)) );
    assert(  1 == FAST_SIGN(int  (  0)) );
    assert( -1 == FAST_SIGN(long (-42)) );
    assert(  1 == FAST_SIGN(long ( 42)) );
    assert(  1 == FAST_SIGN(long (  0)) );

    /* sign works for all numeric type, returns the type it was passed */
    assert( typeid(char)   == typeid(Math::sign(char   (-42))));
    assert( typeid(int)    == typeid(Math::sign(int    (-42))));
    assert( typeid(short)  == typeid(Math::sign(short  (-42))));
    assert( typeid(long)   == typeid(Math::sign(long   (-42))));
    assert( typeid(float)  == typeid(Math::sign(float  (-42))));
    assert( typeid(double) == typeid(Math::sign(double (-42))));

    /* homogeneous type comparisons */
    assert( -1 == Math::sign( -42 ) );
    assert(  1 == Math::sign(  42 ) );
    assert(  1 == Math::sign(   0 ) );

    assert( -1.0 == Math::sign( -42.0 ) );
    assert(  1.0 == Math::sign(  42.0 ) );
    assert(  1.0 == Math::sign(   0.0 ) );

    /* heterogenous type comparisons */
    assert( -1.0 == Math::sign( -42 ) );
    assert(  1.0 == Math::sign(  42 ) );
    assert(  1.0 == Math::sign(   0 ) );

    assert( -1 == Math::sign( -42.0 ) );
    assert(  1 == Math::sign(  42.0 ) );
    assert(  1 == Math::sign(   0.0 ) );


    /* absolute value stuff */

    assert(  0 == Math::abs(   0 ) );
    assert(  0 == Math::abs(  -0 ) );
    assert( 42 == Math::abs(  42 ) );
    assert( 42 == Math::abs( -42 ) );
    assert( 42 == Math::abs(  42 ) );
    assert( 42 == Math::abs( -42 ) );

    assert( CHAR_MAX == Math::abs( 1 + CHAR_MIN ) );
    assert( SHRT_MAX == Math::abs( 1 + SHRT_MIN ) );
    assert(  INT_MAX == Math::abs( 1 +  INT_MIN ) );
    assert( LONG_MAX == Math::abs( 1 + LONG_MIN ) );

    assert(  FLT_MIN == Math::abs(    FLT_MIN ) );
    assert(  FLT_MAX == Math::abs(    FLT_MAX ) );
    assert(  FLT_MIN == Math::abs( -  FLT_MIN ) );
    assert(  FLT_MAX == Math::abs( -  FLT_MAX ) );
    assert(  DBL_MIN == Math::abs(    DBL_MIN ) );
    assert(  DBL_MAX == Math::abs(    DBL_MAX ) );
    assert(  DBL_MIN == Math::abs( -  DBL_MIN ) );
    assert(  DBL_MAX == Math::abs( -  DBL_MAX ) );
    assert( LDBL_MIN == Math::abs(   LDBL_MIN ) );
    assert( LDBL_MAX == Math::abs(   LDBL_MAX ) );
    assert( LDBL_MIN == Math::abs( - LDBL_MIN ) );
    assert( LDBL_MAX == Math::abs( - LDBL_MAX ) );

    return EXIT_SUCCESS ;
}
