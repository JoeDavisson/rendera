/* rendera/Algo.cxx */

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Algo.H"


namespace
{
#if 0
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
  operator<<( std::ostream&os, std::vector< T >const&vec )
  {
    typedef std::vector< T >const vec_type ;
    typedef typename vec_type::const_iterator iterator ;
    iterator it = vec.begin(), end = vec.end() ;
    os << "[" ;
    if( it != end ) os << " " << *it++ ;
    while( it != end ) os << ", " << *it++ ;
    return os << " ]" ;
  }
#endif


  template< typename T >
  inline T _sign( T const &v )
  {
    static T const zero( 0 );
    return ( zero > v ) ? T( -1 ) : T( 1 );
  }

  point
  _point( int const&x, int const&y )
  {
    point ret = { x, y } ;
    return ret ;
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

#if 0
  point
  _transpose( point const&o )
  {
    return _point( o.y, o.x );
  }
#endif


  std::vector< point >
  _bresenham_floating( point const&a, point const&b )
  {
    std::vector< point >ret ;

    int dx = _dx( a, b );
    int dy = _dy( a, b );

    if( 0 == dx ){
      for( int x = a.x, y = a.y ; y != b.y ; y += _sign( dy ) ){
        ret.push_back( _point( x, y ) );
      }
      ret.push_back( b );
      return ret ;
    }

    if( 0 == dy ){
      for( int x = a.x, y = a.y ; x != b.x ; x += _sign( dx ) ){
        ret.push_back( _point( x, y ) );
      }
      ret.push_back( b );
      return ret ;
    }

    double err = 0.0 ;
    double derr = std::abs( double(dy) / double(dx) );

    if( 1.0 < derr ){
      /* iterate over y, shim x */
      derr = 1 / derr ;
      for( int x = a.x, y = a.y ; y != b.y ; y += _sign( dy ) ){
        ret.push_back( _point( x, y ) );
        err += derr ;
        if( 0.5 > err ) continue ;
        x += _sign( dx );
        err -= 1.0 ;
      }
      ret.push_back( b );
      return ret ;
    }else{
      /* iterate over x, shim y */
      for( int x = a.x, y = a.y ; x != b.x ; x += _sign( dx ) ){
        ret.push_back( _point( x, y ) );
        err += derr ;
        if( 0.5 > err ) continue ;
        y += _sign( dy );
        err -= 1.0 ;
      }
      ret.push_back( b );
      return ret ;
    }
  }


  std::vector< point >
  _bresenham( point const&a, point const&b )
  {
    return _bresenham_floating( a, b );
  }
}


std::vector< point >
Algo::bresenham( point const&a, point const&b )
{
  return _bresenham( a, b );
}

std::vector< point >
Algo::bresenham( line const&ln )
{
  return bresenham( ln.a, ln.b );
}


