/* rendera/test/endian.cc */

#include <iostream>
#include <cassert>

#include "rendera.h"
#include "inline.h"

int
main( int, char** )
{
    assert( 4 == sizeof( rgba_t ) );
    assert( 4 == sizeof( un_rgba_t ) );
    assert( 1 == sizeof( rgba_t::r ) );
    assert( 1 == sizeof( rgba_t::g ) );
    assert( 1 == sizeof( rgba_t::b ) );
    assert( 1 == sizeof( rgba_t::a ) );

    rgba_t rgba = get_rgba( 0x03020100 );
    assert( 0 == rgba.r );
    assert( 1 == rgba.g );
    assert( 2 == rgba.b );
    assert( 3 == rgba.a );

    return 0 ;
}
