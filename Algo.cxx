/* rendera/Algo.cxx */

#include <algorithm>
#include <cmath>

#include "Algo.H"
/* #include "Rendera.H" */


namespace
{
  template< typename T >
  inline T _sign( T const &v )
  {
    static T const zero( 0 );
    return ( zero > v ) ? T( -1 ) : T( 1 );
  }

  int
  _dx( point const&a,
       point const&b )
  {
    return b.x - a.x ;
  }

  int
  _dy( point const&a,
       point const&b )
  {
    return b.y - a.y ;
  }

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


std::vector< point >
Algo::bresenham( line const&o )
{
  std::vector< point >ret ;

  int dx = _dx( o.a, o.b );
  int dy = _dy( o.a, o.b );

  if( 0 == dx ){
    for( int y = o.a.y ; y != o.b.y ; y += _sign( dy ) ){
      ret.push_back( _point( o.a.x, y ) );
    }
    ret.push_back( o.b );
    return ret ;
  }

  if( 0 == dy ){
    for( int x = o.a.x ; x != o.b.x ; x += _sign( dx ) ){
      ret.push_back( _point( x, o.a.y ) );
    }
    ret.push_back( o.b );
    return ret ;
  }

  double err = 0.0 ;
  double derr = std::abs( double(dy) / double(dx) );
  for( int x = o.a.x, y = o.a.y ; x != o.b.x ; x += _sign( dx ) ){
    ret.push_back( _point( x, y ) );
    err += derr ;
    if( 0.5 > err ) continue ;
    y += _sign( dy );
    err -= 1.0 ;
  }
  ret.push_back( _point( o.b.x, o.b.y ) );
  return ret ;
}


std::vector< point >
Algo::bresenham( point const&a, point const&b )
{
  return bresenham( _line( a, b ) );
}
