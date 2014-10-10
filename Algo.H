/* rendera/Algo.H */

#ifndef ___RENDERA___ALGO_H___
#define ___RENDERA___ALGO_H___

#include "Types.H"

#include <vector>

namespace Algo
{
  std::vector< point >
  bresenham( line const& );

  std::vector< point >
  bresenham( point const&, point const& );
}

#endif//ndef ___RENDERA___ALGO_H___
