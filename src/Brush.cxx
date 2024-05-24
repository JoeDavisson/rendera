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

#include "Brush.H"
#include "Inline.H"
#include "Map.H"

Brush::Brush()
{
  solidx = new int[96 * 96];
  solidy = new int[96 * 96];
  hollowx = new int[96 * 96];
  hollowy = new int[96 * 96];
  solid_count = 0;
  hollow_count = 0;
  size = 1;
  shape = 0;
  coarse_edge = 0;
  fine_edge = 0;
  blurry_edge = 0;
  watercolor_edge = 0;
  chalk_edge = 0;
  texture_edge = 0;
  texture_marb = 0;
  texture_turb = 0;
  average_edge = 0;
  blend = 0;
  color = makeRgb(255, 255, 255);
  trans = 0;
  aa = 0;
//  alpha_mask = false;
  make(shape, size);
}

Brush::~Brush()
{
  delete[] solidx;
  delete[] solidy;
  delete[] hollowx;
  delete[] hollowy;
}

void Brush::make(int s, float round)
{
  size = s;

  int r = s / 2;
  int inc = s & 1;

  int x1 = 48 - r;
  int y1 = 48 - r;
  int x2 = 48 + r - 1 + inc;
  int y2 = 48 + r - 1 + inc;

  solid_count = 0;
  hollow_count = 0;

  Map map(96, 96);
  Map map2(96, 96);
  map.clear(0);

  if (s == 1)
  {
    map.setpixel(x1, y1, 1);
  }
    else if (s == 2)
  {
    map.rectfill(x1, y1, x2, y2, 1);
  }
    else if (s == 3)
  {
    if (round > .5)
      map.ovalfill(x1, y1, x2, y2, 1);
    else
      map.rectfill(x1, y1, x2, y2, 1);
  }
    else
  {
    int rr = (s - 1) * round;

    map.rectfill(x1, y1 + rr / 2 + 1, x2, y2 - rr / 2 - 1, 1);
    map.rectfill(x1 + rr / 2 + 1, y1, x2 - rr / 2 - 1, y2, 1);
    map.ovalfill(x1, y1, x1 + rr, y1 + rr, 1);
    map.ovalfill(x2, y1, x2 - rr, y1 + rr, 1);
    map.ovalfill(x1, y2, x1 + rr, y2 - rr, 1);
    map.ovalfill(x2, y2, x2 - rr, y2 - rr, 1);
  }

  // solid
  for (int y = 0; y < 96; y++)
  {
    for (int x = 0; x < 96; x++)
    {
      if (map.getpixel(x, y))
      {
        solidx[solid_count] = x - 48;
        solidy[solid_count] = y - 48;
        solid_count++;
      }
    }
  }

  // hollow
  for (int y = 0; y < 96; y++)
  {
    for (int x = 0; x < 96; x++)
    {
      map2.setpixel(x, y, map.getpixel(x, y));
    }
  }

  map2.shrink(0);
  map2.shrink(1);
  map2.shrink(0);

  for (int y = 0; y < 96; y++)
  {
    for (int x = 0; x < 96; x++)
    {
      if (map.getpixel(x, y) && !map2.getpixel(x, y))
      {
        hollowx[hollow_count] = x - 48;
        hollowy[hollow_count] = y - 48;
        hollow_count++;
      }
    }
  }
}

