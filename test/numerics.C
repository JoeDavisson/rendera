/* rendera/test/numerics.C */

#include "Rendera.H"

#include <algorithm>
#include <cassert>
/* #include <climits> */
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdint.h>


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
            b = sign( a );
        }
        std::cout
            << std::endl
            << std::dec << n << " iterations of sign takes\n"
            << _duration( start )
            << std::endl
            ;
    }

    assert( -1 == FAST_SIGN( -42 ) );
    assert(  1 == FAST_SIGN(  42 ) );
    assert(  1 == FAST_SIGN(   0 ) );

    assert( -1 == sign( -42 ) );
    assert(  1 == sign(  42 ) );
    assert(  1 == sign(   0 ) );

    assert( -1.0 == sign( -42.0 ) );
    assert(  1.0 == sign(  42.0 ) );
    assert(  1.0 == sign(   0.0 ) );


    return EXIT_SUCCESS ;
}
