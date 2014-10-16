/* rendera/Types.cxx */


#include "Types.H"


bool
operator==( point const&lhs,
            point const&rhs )
{
  return
      ( lhs.x == rhs.x ) &&
      ( lhs.y == rhs.y ) ;
}

