/*
Copyright (c) 2023 Joe Davisson.

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

std::vector<int> table_fix(256);
std::vector<int> table_unfix(65536);

void Gamma::init()
{
  for(int i = 0; i < 256; i++)
  {
    table_fix[i] = std::pow((double)i / 255, 2.2) * 65535;
  }

  for(int i = 0; i < 65536; i++)
  {
    table_unfix[i] = std::pow((double)i / 65535, (1.0 / 2.2)) * 255;
  }
}

int Gamma::fix(const int val)
{
  return table_fix[val];
}

int Gamma::unfix(const int val)
{
  return table_unfix[val];
}

