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

static int testx[4] = { 20, 10, 40, 30 };
static int testy[4] = { 10, 20, 30, 40 };

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

static void rotate_point(float cx, float cy, float angle, int *x, int *y)
{
  int xnew = ((*x - cx) * cosf(angle) - (*y - cy) * sinf(angle)) + cx;
  int ynew = ((*x - cx) * sinf(angle) + (*y - cy) * cosf(angle)) + cy;

  *x = xnew;
  *y = ynew;
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

Stroke::Stroke()
{
  active = 0;
}

Stroke::~Stroke()
{
}

void Stroke::begin(Map *map, int x, int y, int ox, int oy, int size, float zoom, int type)
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
  beginx = x;
  beginy = y;
  make_blitrect(x, y, beginx, beginy, ox, oy, size, zoom);

  active = 1;
}

void Stroke::draw(Map *map, int x, int y, int ox, int oy, int size, float zoom, int type)
{
  int r = size / 2;
  int inc = size & 1;
  int xbuf[4];
  int ybuf[4];
  int i;

  switch(type)
  {
    case 0:
      float angle = atan2f(y - lasty, x - lastx);

      xbuf[0] = x;
      ybuf[0] = y - r;
      xbuf[1] = x;
      ybuf[1] = y + r + inc;
      xbuf[2] = lastx;
      ybuf[2] = lasty - r;
      xbuf[3] = lastx;
      ybuf[3] = lasty + r + inc;

      rotate_point(x, y, angle, &xbuf[0], &ybuf[0]);
      rotate_point(x, y, angle, &xbuf[1], &ybuf[1]);
      rotate_point(lastx, lasty, angle, &xbuf[2], &ybuf[2]);
      rotate_point(lastx, lasty, angle, &xbuf[3], &ybuf[3]);

      map->quad(xbuf, ybuf, 255);
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

  make_blitrect(x, y, lastx, lasty, ox, oy, size, zoom);
  lastx = x;
  lasty = y;
}

void Stroke::end(Map *map, int xx, int yy, int ox, int oy, int size, float zoom, int type)
{
  active = 0;
}

void Stroke::preview(Bitmap *backbuf, Map *map, int ox, int oy, float zoom)
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

