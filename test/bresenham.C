/* rendera/test/bresenham.C */


#include "Algo.H"
#include "Types.H"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>


namespace
{
  point
  _point( int const&x, int const&y )
  {
    point ret = { x, y } ;
    return ret ;
  }

  line
  _line( point const&a, point const&b )
  {
    line ret = { a, b } ;
    return ret ;
  }
}


int
main( int, char** )
{
  {
    point const a = _point(0,0);
    point const b = _point(5,7);
    std::vector< point >const ln = Algo::bresenham( _line( a, b ) );
    assert( 8 == ln.size() );
    assert( _point(0,0) == ln[0] );
    assert( _point(1,1) == ln[1] );
    assert( _point(1,2) == ln[2] );
    assert( _point(2,3) == ln[3] );
    assert( _point(3,4) == ln[4] );
    assert( _point(4,5) == ln[5] );
    assert( _point(4,6) == ln[6] );
    assert( _point(5,7) == ln[7] );
  }

  {
    std::vector< point >const ln = Algo::bresenham( _point(0,0), _point(0,0) );
    assert( 1 == ln.size() );
    assert( _point(0,0) == ln[0] );
  }

  {
    std::vector< point >const ln = Algo::bresenham( _point(0,0), _point(0,1) );
    assert( 2 == ln.size() );
    assert( _point(0,0) == ln[0] );
    assert( _point(0,1) == ln[1] );
  }

  {
    std::vector< point >const ln = Algo::bresenham( _point(0,0), _point(1,0) );
    assert( 2 == ln.size() );
    assert( _point(0,0) == ln[0] );
    assert( _point(1,0) == ln[1] );
  }

  {
    std::vector< point >const ln = Algo::bresenham( _point(0,0), _point(1,1) );
    assert( 2 == ln.size() );
    assert( _point(0,0) == ln[0] );
    assert( _point(1,1) == ln[1] );
  }

  return EXIT_SUCCESS ;
}
