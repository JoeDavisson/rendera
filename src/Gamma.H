/*
Copyright (c) 2014 Joe Davisson.

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

#ifndef GAMMA_H
#define GAMMA_H

#include <cmath>

namespace
{
  // create gamma-correction tables
  int *init_table_fix(void)
  {
    int *ret = new int[256];

    for(int i = 0; i < 256; i++)
      ret[i] = std::pow((double)i / 255, 2.2) * 65535;

    return ret;
  }

  int *init_table_unfix(void)
  {
    int *ret = new int[65536];

    for(int i = 0; i < 65536; i++)
      ret[i] = std::pow((double)i / 65535, (1.0 / 2.2)) * 255;

    return ret;
  }

  static const int *table_fix = init_table_fix();
  static const int *table_unfix = init_table_unfix();
}

namespace Gamma
{
  // convert to gamma-corrected colorspace
  inline int fix(const int &val)
  {
    return table_fix[val];
  }

  // return to linear colorspace
  inline int unfix(const int &val)
  {
    return table_unfix[val];
  }
}

#endif

