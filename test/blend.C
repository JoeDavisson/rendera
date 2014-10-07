/* rendera/test/blend.C */


#include "Rendera.H"
#include "Inline.H"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>


namespace
{
    inline
    uint32_t
    _blend( uint32_t const&c1,
            uint32_t const&c2,
            uint8_t const&t )
    {
        return blendFast( c1, c2, t );
    }
}


int
main( int, char** )
{
    assert( 0xff000000 == _blend( 0xff, 0x00, 0x00 ) );
    assert( 0xff00007f == _blend( 0xff, 0x00, 0x80 ) );
    assert( 0xff0000fe == _blend( 0xff, 0x00, 0xff ) );

    assert( 0xff000000 == _blend( 0xffff, 0x0000, 0x00 ) );
    assert( 0xff007f7f == _blend( 0xffff, 0x0000, 0x80 ) );
    assert( 0xff00fefe == _blend( 0xffff, 0x0000, 0xff ) );

    assert( 0xff000000 == _blend( 0xff00, 0x0000, 0x00 ) );
    assert( 0xff007f00 == _blend( 0xff00, 0x0000, 0x80 ) );
    assert( 0xff00fe00 == _blend( 0xff00, 0x0000, 0xff ) );

    assert( 0xff000000 == _blend( 0xffff00, 0x000000, 0x00 ) );
    assert( 0xff7f7f00 == _blend( 0xffff00, 0x000000, 0x80 ) );
    assert( 0xfffefe00 == _blend( 0xffff00, 0x000000, 0xff ) );

    assert( 0xff000000 == _blend( 0xffffff, 0x000000, 0x00 ) );
    assert( 0xff7f7f7f == _blend( 0xffffff, 0x000000, 0x80 ) );
    assert( 0xfffefefe == _blend( 0xffffff, 0x000000, 0xff ) );

    assert( 0xff000000 == _blend( 0xff00ff, 0x000000, 0x00 ) );
    assert( 0xff7f007f == _blend( 0xff00ff, 0x000000, 0x80 ) );
    assert( 0xfffe00fe == _blend( 0xff00ff, 0x000000, 0xff ) );

    assert( 0xff000000 == _blend( 0xff0000, 0x000000, 0x00 ) );
    assert( 0xff7f0000 == _blend( 0xff0000, 0x000000, 0x80 ) );
    assert( 0xfffe0000 == _blend( 0xff0000, 0x000000, 0xff ) );

    return EXIT_SUCCESS ;
}
