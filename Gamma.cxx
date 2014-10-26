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

#include <cmath>

#include "Gamma.H"

namespace
{
  int table_fix[256];
  int table_unfix[65536];
}

int Gamma::fix(int val)
{
  if(val >= 0 && val < 256)
    return table_fix[val];
  else
    return 0;
}

int Gamma::unfix(int val)
{
  if(val >= 0 && val < 65536)
    return table_unfix[val];
  else
    return 0;
}

void Gamma::init()
{
  int i;

  for(i = 0; i < 65536; i++)
    table_unfix[i] = std::pow((double)i / 65535, (1.0 / 2.2)) * 255;
  for(i = 0; i < 256; i++)
    table_fix[i] = std::pow((double)i / 255, 2.2) * 65535;
}
