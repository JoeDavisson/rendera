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

#ifndef EXTRAMATH_H
#define EXTRAMATH_H

namespace ExtraMath
{
  // sign, used by fastStretch in Bitmap.cxx
  template <typename T>
  inline T sign(T const &v)
  {
    return ( T(0) > v ) ? T(-1) : T(1);
  }

  // absolute value
  // std::abs doesn't seem to work right on some platforms
  template <typename T>
  inline T abs(T const n)
  {
    return (0 > n) ? -n : n;
  }

  // fast pseudo-random number
  inline int rnd(void)
  {
    static int seed = 12345;
    seed ^= seed << 17;
    seed ^= seed >> 13;
    seed ^= seed <<  5;
    return seed;
  }

  // ^2 check
  inline bool isPowerOfTwo(int x)
  {
    while(((x & 1) == 0) && x > 1)
      x >>= 1;

    return (x == 1);
  }

  // fft routines
  void forwardFFT(float *, float *, int);
  void inverseFFT(float *, float *, int);
}

#endif

