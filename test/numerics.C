/* rendera/test/numerics.C */

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdint.h>
#include <typeinfo>

#include "Common.H"

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
    assert( typeid(char)   == typeid(Common::sign(char   (-42))));
    assert( typeid(int)    == typeid(Common::sign(int    (-42))));
    assert( typeid(short)  == typeid(Common::sign(short  (-42))));
    assert( typeid(long)   == typeid(Common::sign(long   (-42))));
    assert( typeid(float)  == typeid(Common::sign(float  (-42))));
    assert( typeid(double) == typeid(Common::sign(double (-42))));

    /* homogeneous type comparisons */
    assert( -1 == Common::sign( -42 ) );
    assert(  1 == Common::sign(  42 ) );
    assert(  1 == Common::sign(   0 ) );

    assert( -1.0 == Common::sign( -42.0 ) );
    assert(  1.0 == Common::sign(  42.0 ) );
    assert(  1.0 == Common::sign(   0.0 ) );

    /* heterogenous type comparisons */
    assert( -1.0 == Common::sign( -42 ) );
    assert(  1.0 == Common::sign(  42 ) );
    assert(  1.0 == Common::sign(   0 ) );

    assert( -1 == Common::sign( -42.0 ) );
    assert(  1 == Common::sign(  42.0 ) );
    assert(  1 == Common::sign(   0.0 ) );


    return EXIT_SUCCESS ;
}
