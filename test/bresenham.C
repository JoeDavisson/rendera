/* rendera/test/bresenham.C */


#include "Algo.H"
#include "Types.H"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>


namespace
{
  std::ostream&
  operator<<( std::ostream&os, point const&pt )
  {
    return os << "{ x:" << pt.x << ", y:" << pt.y << " }" ;
  }

  std::ostream&
  operator<<( std::ostream&os, line const&ln )
  {
    return os << "{ a:" << ln.a << ", b:" << ln.b << " }" ;
  }

  template< typename T >
  std::ostream&
  operator<<( std::ostream&os, std::vector< T >vec )
  {
    typedef std::vector< T >vec_type ;
    typedef typename vec_type::iterator iterator ;
    iterator it = vec.begin(), end = vec.end() ;
    os << "[" ;
    if( it != end ) os << " " << *it++ ;
    while( it != end ) os << ", " << *it++ ;
    return os << " ]" ;
  }
}


int
main( int, char** )
{
  point const foo = { 2, 5 } ;
  point const bar = { 5, 7 } ;
  line const foobar = { foo, bar } ;

  std::cout
    << std::dec
    << std::endl
    << __PRETTY_FUNCTION__
    << std::endl
    << "foo == " << foo
    << std::endl
    << "bar == " << bar
    << std::endl
    << "foobar == " << foobar
    << std::endl
    ;

  std::cout
    << std::endl
    << "std::vector< int >( 0 ) == " << std::vector< int >( 0 )
    << std::endl
    << "std::vector< int >( 1 ) == " << std::vector< int >( 1 )
    << std::endl
    << "std::vector< int >( 2 ) == " << std::vector< int >( 2 )
    << std::endl
    ;

  std::cout
    << std::endl
    << "Algo::bresenham( foobar ) == " << Algo::bresenham( foobar )
    << std::endl
    ;

  point const p00 = { 0, 0 } ;
  point const p01 = { 0, 1 } ;
  point const p10 = { 1, 0 } ;
  point const p11 = { 1, 1 } ;

  std::cout
    << std::endl
    << "Algo::bresenham( " << p00 << ", " << p00 << " ) == "
    << Algo::bresenham( p00, p00 )
    << std::endl
    << "Algo::bresenham( " << p00 << ", " << p01 << " ) == "
    << Algo::bresenham( p00, p01 )
    << std::endl
    << "Algo::bresenham( " << p00 << ", " << p10 << " ) == "
    << Algo::bresenham( p00, p10 )
    << std::endl
    << "Algo::bresenham( " << p00 << ", " << p11 << " ) == "
    << Algo::bresenham( p00, p11 )
    << std::endl
    ;

  return EXIT_SUCCESS ;
}
