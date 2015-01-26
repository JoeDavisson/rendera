/*
Copyright (c) 2015 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#ifndef COMMON_H
#define COMMON_H

#include <algorithm>

namespace Common
{
  namespace
  {
    template< typename UnaryFunc >
    struct _comparator
    {
      UnaryFunc unaryfunc_;

      explicit _comparator( UnaryFunc unaryfunc ):unaryfunc_( unaryfunc ){}

      template< typename T >
      bool
      operator()( T const&lhs, T const&rhs )
      {
        return ( unaryfunc_( lhs ) < unaryfunc_( rhs ) ) ;
      }
    };
  }

  // for palette sorting etc
  template< typename Iter, typename UnaryFunc >
  void
  sortBy( Iter first, Iter last, UnaryFunc unaryfunc )
  {
    std::sort( first, last, _comparator< UnaryFunc >( unaryfunc ) );
  }

  // sign, used by fastStretch in Bitmap.cxx
  template< typename T >
  inline T sign( T const &v )
  {
    static T const zero( 0 );
    return ( zero > v ) ? T( -1 ) : T( 1 );
  }
}

#endif

