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

#include "rendera.h"

Brush::Brush(int s)
{
  solidx = new int[96 * 96];
  solidy = new int[96 * 96];
  hollowx = new int[96 * 96];
  hollowy = new int[96 * 96];
  solid_count = 0;
  hollow_count = 0;
  size = s;
  make(s);
}

Brush::~Brush()
{
}

void Brush::make(int s)
{
  size = s;
  int x, y;

  int r = s / 2;
  int inc = s & 1;
  if(s == 1)
    inc = 0;

  int x1 = 48 - r;
  int y1 = 48 - r;
  int x2 = 48 + r + inc;
  int y2 = 48 + r + inc;

  solid_count = 0;
  hollow_count = 0;

  Map *map = new Map(96, 96);
  map->clear(0);
  map->ovalfill(x1, y1, x2, y2, 255);

  for(y = 0; y < 96; y++)
  {
    for(x = 0; x < 96; x++)
    {
      if(map->getpixel(x, y))
      {
        solidx[solid_count] = x - 48;
        solidy[solid_count] = y - 48;
        solid_count++;
      }
    }
  }

  if(size > 4)
    map->ovalfill(x1 + 2, y1 + 2, x2 - 2, y2 - 2, 0);

  for(y = 0; y < 96; y++)
  {
    for(x = 0; x < 96; x++)
    {
      if(map->getpixel(x, y))
      {
        hollowx[hollow_count] = x - 48;
        hollowy[hollow_count] = y - 48;
        hollow_count++;
      }
    }
  }

  delete map;
}

