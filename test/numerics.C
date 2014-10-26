/* rendera/test/numerics.C */

#include "Rendera.H"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdint.h>
#include <typeinfo>

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

    static uint64_t const billion = uint64_t( 1000000000 );

    uint64_t
    _nsec( timespec const&o )
    {
        uint64_t ret = 0 ;
        return
            ( uint64_t( o.tv_sec ) * billion ) +
            ( uint64_t( o.tv_nsec ) )
            ;
        return ret ;
    }

    timespec
    _timespec( uint64_t const&nsec )
    {
        timespec ret ;
        ret.tv_sec = ( nsec / billion );
        ret.tv_nsec = ( nsec % billion );
        return ret ;        
    }

    timespec
    _now( void )
    {
        timespec ret ;
        int stat = clock_gettime( CLOCK_REALTIME, &ret );
        assert( 0 == stat );
        return ret ;
    }

    timespec
    _duration( timespec const&start, timespec const&stop )
    {
        return _timespec( _nsec( stop ) - _nsec( start ) );
    }

    timespec
    _duration( timespec const& start )
    {
        return _duration( start, _now() );
    }

    std::ostream&
    operator<<( std::ostream&os, timespec const&tp )
    {
        return
            os
            << std::dec
            << "{ tv_sec: " << tp.tv_sec
            << ", tv_nsec: " << tp.tv_nsec
            << " }"
            ;
    }
}


int
main( int, char** )
{
    std::cout << std::endl ;

    size_t const n( 5000000 );

    volatile int a = 42, b = 69 ;

    /* FAST_SIGN vs sign */
    {
        timespec start = _now();
        for( size_t i = 0; i != n; ++i ){
            a = FAST_SIGN( b );
        }
        std::cout
            << std::endl
            << std::dec << n << " iterations of FAST_SIGN takes\n"
            << _duration( start )
            << std::endl
            ;
    }
    {
        timespec start = _now();
        for( size_t i = 0; i != n; ++i ){
            b = Common::sign( a );
        }
        std::cout
            << std::endl
            << std::dec << n << " iterations of sign takes\n"
            << _duration( start )
            << std::endl
            ;
    }

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
