/*
Copyright (c) 2025 Joe Davisson.

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

#include "Fractal.H"
#include "Inline.H"
#include "Map.H"

namespace
{
  int level, turb;
}

void Fractal::marble(Map *src, Map *dest, Map *marbx, Map *marby, float scale, float turbulence, int type)
{
  int xval[256];
  int yval[256];

  int x, y, i;
  int xx, yy;
  int w = src->w;
  int h = src->h;
  float xv, yv;

  for (i = 0; i < 256; i++)
  {
    switch (type)
    {
      case 0:
        xv = scale * std::sin((float)i / turbulence);
        yv = scale * std::sin((float)i / turbulence);
        xval[i] = -xv;
        yval[i] = yv;
        break;
      case 1:
        xv = scale * std::sin((float)i / turbulence);
        yv = scale * std::sin((float)i / turbulence);
        xval[i] = -xv;
        yval[i] = yv;
        break;
      case 2:
        xv = (scale / 4) * std::sin((float)i / turbulence);
        yv = (scale / 4) * std::atan((float)i / turbulence);
        xval[i] = -(int)((xv - (int)yv) * 10);
        yval[i] = (int)((yv - (int)xv) * 100);
        break;
      case 3:
        xv = -scale * std::sin((float)i / turbulence);
        yv = scale * std::sin((float)i / turbulence);
        xval[i] = (xv - (int)xv) * 50;
        yval[i] = (yv - (int)yv) * 50;
        break;
      case 4:
        xv = scale * std::sin(i / (int)turbulence);
        yv = scale * std::atan(i / (int)turbulence);
        xval[i] = -(xv - (int)xv) * 40;
        yval[i] = (yv - (int)yv) * 40;
        break;
      case 5:
        xv = scale * std::sin((float)i / turbulence);
        yv = scale * std::atan((float)i / turbulence);
        xval[i] = -(int)((xv - (int)xv)) * 10;
        yval[i] = (int)((yv - (int)xv)) - xval[i];
        break;
      case 6:
        xv = scale * std::sin((float)i / turbulence);
        yv = scale * std::sin((float)i / turbulence);
        xval[i] = -(int)((xv - (int)xv));
        yval[i] = (int)(30.0 / (yv - (int)yv));
        break;
      default:
        xv = scale * std::sin((float)i / turbulence);
        yv = scale * std::sin((float)i / turbulence);
        xval[i] = -xv;
        yval[i] = yv;
        break;
    }
  }

  for (y = 0; y < h; y++)
  {
    for (x = 0; x < w; x++)
    {
      xx = x + xval[marbx->getpixel(x, y)];
      yy = y + yval[marby->getpixel(x, y)];

      while (xx < 0)
        xx += w;
      while (yy < 0)
        yy += h;
      while (xx >= w)
        xx -= w;
      while (yy >= h)
        yy -= h;

      dest->setpixel(x, y, src->getpixel(xx, yy));
    }
  }
}

void Fractal::plasma(Map * map, int turbulence)
{
  int w, h;

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
  int r = (rnd() % turb) >> level;

  if ((rnd() % 2) == 0)
    r = -r;

  r = ((map->getpixel(xa, ya) + map->getpixel(xb, yb) + 1) >> 1) + r;

  if (r < 1)
    r = 1;
  if (r > 255)
    r = 255;

  map->setpixel(x, y, r);

  if (x == 0)
    map->setpixel(map->w - 1, y, r);
  if (y == 0)
    map->setpixel(x, map->h - 1, r);
  if (x == map->w - 1)
    map->setpixel(0, y, r);
  if (y == map->h - 1)
    map->setpixel(x, 0, r);

  return r;
}

void Fractal::divide(Map * map, int x1, int y1, int x2, int y2)
{
  int x, y, i, v;

  if (((x2 - x1) < 2) && ((y2 - y1) < 2))
    return;

  level++;

  x = (x1 + x2) >> 1;
  y = (y1 + y2) >> 1;

  v = map->getpixel(x, y1);

  if (!v)
    v = Fractal::adjust(map, x1, y1, x, y1, x2, y1);

  i = v;
  v = map->getpixel(x2, y);

  if (!v)
    v = Fractal::adjust(map, x2, y1, x2, y, x2, y2);

  i += v;
  v = map->getpixel(x, y2);

  if (!v)
    v = Fractal::adjust(map, x1, y2, x, y2, x2, y2);

  i += v;
  v = map->getpixel(x1, y);

  if (!v)
    v = Fractal::adjust(map, x1, y1, x1, y, x1, y2);

  i += v;

  if (!map->getpixel(x, y))
    map->setpixel(x, y, ((i + 2) >> 2));

  divide(map, x1, y1, x, y);
  divide(map, x, y1, x2, y);
  divide(map, x, y, x2, y2);
  divide(map, x1, y, x, y2);

  level--;
}

