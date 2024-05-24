/*
Copyright (c) 2024 Joe Davisson.

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
#include <vector>

#include "Gamma.H"
#include "Inline.H"

int *Gamma::table_fix;
int *Gamma::table_unfix;

void Gamma::init()
{
  // OS frees these
  table_fix = new int[256];
  table_unfix = new int[65536];

  for (int i = 0; i < 256; i++)
  {
    table_fix[i] = std::round(std::pow((double)i / 255, 2.2) * 65535);
  }

  for (int i = 0; i < 65536; i++)
  {
    table_unfix[i] = std::round(std::pow((double)i / 65535, (1.0 / 2.2)) * 255);
  }
}

