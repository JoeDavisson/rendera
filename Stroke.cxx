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

static inline int is_edge(Map *map, int x, int y)
{
  if((map->getpixel(x - 1, y)) &&
     (map->getpixel(x + 1, y)) &&
     (map->getpixel(x, y - 1)) &&
     (map->getpixel(x, y + 1)))
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

void Stroke::clip()
{
  if(x1 < 0)
    x1 = 0;
  if(y1 < 0)
    y1 = 0;
  if(x2 < 0)
    x2 = 0;
  if(y2 < 0)
    y2 = 0;
  if(x1 > Bmp::main->w - 1)
    x1 = Bmp::main->w - 1;
  if(y1 > Bmp::main->h - 1)
    y1 = Bmp::main->h - 1;
  if(x2 > Bmp::main->w - 1)
    x2 = Bmp::main->w - 1;
  if(y2 > Bmp::main->h - 1)
    y2 = Bmp::main->h - 1;
}

void Stroke::make_blitrect(int x1, int y1, int x2, int y2, int ox, int oy, int size, float zoom)
{
  int r = (size + 1) / 2 + 1;

  r *= zoom;

  x1 -= ox;
  y1 -= oy;
  x2 -= ox;
  y2 -= oy;

  x1 *= zoom;
  y1 *= zoom;
  x2 *= zoom;
  y2 *= zoom;

  if(x2 < x1)
    SWAP(x1, x2);
  if(y2 < y1)
    SWAP(y1, y2);

  x1 -= r;
  y1 -= r;
  x2 += r;
  y2 += r;

  blitx = x1;
  blity = y1;
  blitw = x2 - x1;
  blith = y2 - y1;

  if(blitw < 1)
    blitw = 1;
  if(blith < 1)
    blith = 1;
}

void Stroke::begin(Brush *brush, Map *map, int x, int y, int ox, int oy, float zoom, int type)
{
  int r = brush->size / 2;
  int inc = brush->size & 1;

  map->clear(0);

  switch(type)
  {
    case 0:
      if(brush->size == 1)
      {
        map->setpixel(x, y, 255);
        break;
      }

      map->ovalfill(x - r, y - r, x + r + inc, y + r + inc, 255);
      break;
  }

  x1 = x - (r + 1);
  y1 = y - (r + 1);
  x2 = x + (r + 1);
  y2 = y + (r + 1);

  lastx = x;
  lasty = y;
  beginx = x;
  beginy = y;
  make_blitrect(x, y, beginx, beginy, ox, oy, brush->size, zoom);

  active = 1;
}

void Stroke::draw(Brush *brush, Map *map, int x, int y, int ox, int oy, float zoom, int type)
{
  int r = brush->size / 2;
  int inc = brush->size & 1;
  int xbuf[4];
  int ybuf[4];
  int i;

  switch(type)
  {
    case 0:
      if(brush->size == 1)
      {
        map->line(x, y, lastx, lasty, 255);
        break;
      }

      for(i = 0; i < brush->count; i++)
      {
        map->line(x + brush->xbuf[i], y + brush->ybuf[i],
                  lastx + brush->xbuf[i], lasty + brush->ybuf[i], 255);
      }

      break;
  }

  if(x - r - 1 < x1)
    x1 = x - r - 1;
  if(y - r - 1 < y1)
    y1 = y - r - 1;
  if(x + r + 1 > x2)
    x2 = x + r + 1;
  if(y + r + 1 > y2)
    y2 = y + r + 1;

  make_blitrect(x, y, lastx, lasty, ox, oy, brush->size, zoom);
  lastx = x;
  lasty = y;
}

void Stroke::end(Brush *brush, Map *map, int xx, int yy, int ox, int oy, float zoom, int type)
{
  apply(map);
  active = 0;
}

void Stroke::preview(Map *map, Bitmap *backbuf, int ox, int oy, float zoom)
{
  int x, y;

  clip();

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
        backbuf->xor_rectfill(xx1 - ox * zoom, yy1 - oy * zoom, xx2 - ox * zoom, yy2 - oy * zoom);
      }
      p++;
    }
  }
}

void Stroke::apply(Map *map)
{
  int x, y;

  for(y = y1; y <= y2; y++)
  {
    for(x = x1; x <= x2; x++)
    {
      if(map->getpixel(x, y) > 0)
        Bmp::main->setpixel_solid(x, y, makecol(0, 255, 0), 192);
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

