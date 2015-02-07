/* rendera/test/pathexe.C */

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#include "Path.H"

int
main( int argc, char**argv )
{
  std::cout
    << std::endl
    << __PRETTY_FUNCTION__ << " in " << __FILE__
    << std::endl
    << "argc == " << std::dec << argc
    << std::endl
    << "argv[0] == "
    << "\""
    << argv[0]
    << "\""
    << std::endl
    << "::Path::executable() == "
    << "\""
    << ::Path::executable()
    << "\""
    << std::endl
    << "::Path::basename( argv[0] ) == "
    << "\""
    << ::Path::basename( argv[0] )
    << "\""
    << std::endl
    << "::Path::basename( ::Path::executable() ) == "
    << "\""
    << ::Path::basename( ::Path::executable() )
    << "\""
    << std::endl
    ;

  assert
    ( ::Path::basename( argv[0] )
      ==
      ::Path::basename( ::Path::executable() ) );

  return EXIT_SUCCESS ;
}
