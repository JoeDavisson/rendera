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

#ifndef GAMMA_H
#define GAMMA_H

class Gamma
{
public:
  static void init();

  static inline int fix(const int val)
  {
    return table_fix[val];
  }

  static inline int unfix(const int val)
  {
    return table_unfix[val];
  }

  static int *table_fix;
  static int *table_unfix;

private:
  Gamma() { }
  ~Gamma() { }
};

#endif

