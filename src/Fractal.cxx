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

#include <cmath>

#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>

#include "ExtraMath.H"
#include "Fractal.H"
#include "Map.H"

namespace
{
  int level, turb;
}

void Fractal::marble(Map *src, Map *dest, Map *marbx, Map *marby, float scale, int turbulence)
{
  int xval[256];
  int yval[256];

  int x, y, i;
  int xx, yy;
  int w = src->w;
  int h = src->h;

  for(i = 0; i < 256; i++)
  {
    xval[i] = -scale * std::sin((float)i / (float)turbulence);
    yval[i] = scale * std::sin((float)i / (float)turbulence);
  }

  for(y = 0; y < h; y++)
  {
    for(x = 0; x < w; x++)
    {
      xx = x + xval[marbx->getpixel(x, y)];
      yy = y + yval[marby->getpixel(x, y)];

      while(xx < 0)
        xx += w;
      while(yy < 0)
        yy += h;
      while(xx >= w)
        xx -= w;
      while(yy >= h)
        yy -= h;

      dest->setpixel(x, y, src->getpixel(xx, yy));
    }
  }
}

void Fractal::plasma(Map * map, int turbulence)
{
  int w, h;

//  seed = ExtraMath::rnd();
  map->clear(0);
  turb = turbulence;
  level = 0;

  w = map->w;
  h = map->h;

  map->setpixel(0, 0, 0);
  map->setpixel(w - 1, 0, 0);
  map->setpixel(0, h - 1, 0);
  map->setpixel(w - 1, h - 1, 0);

  divide(map, 0, 0, w - 1, h - 1);
}

int Fractal::adjust(Map * map, int xa, int ya, int x, int y, int xb, int yb)
{
  int rnd = (ExtraMath::rnd() % turb) >> level;

  if((ExtraMath::rnd() % 2) == 0)
    rnd = -rnd;

  rnd = ((map->getpixel(xa, ya) +
          map->getpixel(xb, yb) + 1) >> 1) + rnd;

  if(rnd < 1)
    rnd = 1;
  if(rnd > 255)
    rnd = 255;

  map->setpixel(x, y, rnd);

  if(x == 0)
    map->setpixel(map->w - 1, y, rnd);
  if(y == 0)
    map->setpixel(x, map->h - 1, rnd);
  if(x == map->w - 1)
    map->setpixel(0, y, rnd);
  if(y == map->h - 1)
    map->setpixel(x, 0, rnd);

  return rnd;
}

void Fractal::divide(Map * map, int x1, int y1, int x2, int y2)
{
  int x, y, i, v;

  if(((x2 - x1) < 2) && ((y2 - y1) < 2))
    return;

  level++;

  x = (x1 + x2) >> 1;
  y = (y1 + y2) >> 1;

  v = map->getpixel(x, y1);

  if(!v)
    v = Fractal::adjust(map, x1, y1, x, y1, x2, y1);

  i = v;
  v = map->getpixel(x2, y);

  if(!v)
    v = Fractal::adjust(map, x2, y1, x2, y, x2, y2);

  i += v;
  v = map->getpixel(x, y2);

  if(!v)
    v = Fractal::adjust(map, x1, y2, x, y2, x2, y2);

  i += v;
  v = map->getpixel(x1, y);

  if(!v)
    v = Fractal::adjust(map, x1, y1, x1, y, x1, y2);

  i += v;

  if(!map->getpixel(x, y))
    map->setpixel(x, y, ((i + 2) >> 2));

  divide(map, x1, y1, x, y);
  divide(map, x, y1, x2, y);
  divide(map, x, y, x2, y2);
  divide(map, x1, y, x, y2);

  level--;

}

