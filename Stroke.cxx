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

static int lastx, lasty;

static inline int is_edge(Map *map, int x, int y)
{
  if((map->getpixel(x - 1, y) == 0xff) &&
     (map->getpixel(x + 1, y) == 0xff) &&
     (map->getpixel(x, y - 1) == 0xff) &&
     (map->getpixel(x, y + 1) == 0xff))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

Stroke::Stroke()
{
  active = 0;
}

Stroke::~Stroke()
{
}

void Stroke::begin(Map *map, int x, int y, int size, int type)
{
  int r = size / 2;
  int inc = size & 1;

  map->clear(0);

  switch(type)
  {
    case 0:
      map->ovalfill(x - r, y - r, x + r + inc, y + r + inc, 255);
    break;
  }

  x1 = x - (r + 1);
  y1 = y - (r + 1);
  x2 = x + (r + 1);
  y2 = y + (r + 1);

  lastx = x;
  lasty = y;

  active = 1;
}

void Stroke::draw(Map *map, int x, int y, int size, int type)
{
  int r = size / 2;
  int inc = size & 1;
  switch(type)
  {
    case 0:
//      int angle = atan2(y - lasty, x - lastx);
      map->ovalfill(x - r, y - r, x + r + inc, y + r + inc, 255);
    break;
  }

  if(x - r < x1)
    x1 = x - r;
  if(y - r < y1)
    y1 = y - r;
  if(x + r > x2)
    x2 = x + r;
  if(y + r > y2)
    y2 = y + r;

  lastx = x;
  lasty = y;
}

void Stroke::end(Map *map, int xx, int yy, int size, int type)
{
  active = 0;
}

void Stroke::preview(Bitmap *backbuf, Map *map, int zoom, int ox, int oy)
{
  int x, y;

  for(y = y1; y <= y2; y++)
  {
    int yy1 = y * zoom;
    int yy2 = yy1 + zoom - 1;
    unsigned char *p = map->row[y] + x1;
    for(x = x1; x <= x2; x++)
    {
      if(*p > 0 && is_edge(map, x, y))
      {
        int xx1 = x * zoom;
        int xx2 = xx1 + zoom - 1;
        backbuf->rectfill(xx1 - ox * zoom, yy1 - oy * zoom, xx2 - ox * zoom, yy2 - oy * zoom, makecol(0, 0, 0), 192);
      }
      p++;
    }
  }
}

void Stroke::freehand()
{
}

void Stroke::region()
{
}

void Stroke::line()
{
}

void Stroke::polygon()
{
}

void Stroke::rect()
{
}

void Stroke::rectfill()
{
}

void Stroke::oval()
{
}

void Stroke::ovalfill()
{
}

