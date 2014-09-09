/* rendera/test/fltk_version.cc */

#include <iostream>
#include <cassert>

#include <FL/Enumerations.H>

int
main( int, char** )
{
    std::cout
        << std::dec
        << std::endl
        << __PRETTY_FUNCTION__
        << std::endl
        << "FL_MAJOR_VERSION == " << FL_MAJOR_VERSION
        << std::endl
        << "FL_MINOR_VERSION == " << FL_MINOR_VERSION
        << std::endl
        << "FL_PATCH_VERSION == " << FL_PATCH_VERSION
        << std::endl
        << "FL_VERSION == " << FL_VERSION
        << std::endl
        ;
    assert( 1 <= FL_MAJOR_VERSION );
    assert( 3 <= FL_MINOR_VERSION );
    assert( 0 <= FL_PATCH_VERSION );
    return 0 ;
}
