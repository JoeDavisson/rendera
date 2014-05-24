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
  xbuf = new int[96 * 96];
  ybuf = new int[96 * 96];
  count = 0;
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

  count = 0;

  Map *map = new Map(96, 96);
  map->clear(0);
  map->ovalfill(x1, y1, x2, y2, 255);

  if(size > 4)
    map->ovalfill(x1 + 2, y1 + 2, x2 - 2, y2 - 2, 0);

  for(y = 0; y < 96; y++)
  {
    for(x = 0; x < 96; x++)
    {
      if(map->getpixel(x, y))
      {
        xbuf[count] = x - 48;
        ybuf[count] = y - 48;
        count++;
      }
    }
  }

  delete map;
}

