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

Bitmap::Bitmap(int width, int height)
{
  int i;

  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int[width * height];
  row = new int [height];

  w = width;
  h = height;

  set_clip(0, 0, w, h);

  for(i = 0; i < height; i++)
    row[i] = width * i;
}

Bitmap::~Bitmap()
{
  delete row;
  delete data;
}

void Bitmap::clear(int c)
{
  int i;
  for(i = 0; i < w * h; i++)
    data[i] = c;
}

void Bitmap::hline(int x1, int y, int x2, int c, int t)
{
  clip(&x1, &y, &x2, &y);

  if(x1 > x2)
    return;

  int *x = &data[row[y] + x2];
  int *z = &data[row[y] + x1];

  do
  {
    *x = blend->current(*x, c, t);
    x--;
  }
  while(x >= z);
}

void Bitmap::rect(int x1, int y1, int x2, int y2, int c, int t)
{
  clip(&x1, &y1, &x2, &y2);

  if(x1 > x2)
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

  int *y = &data[row[y2] + x2];
  int *z = &data[row[y1] + x1];
  int d = x2 - x1;
  int e = w - d;

  hline(x1 + 1, y2, x2 - 1, c, t);

  do
  {
    *y = blend->current(*y, c, t);
    y -= d;
    *y = blend->current(*y, c, t);
    y -= e;
  }
  while(y > z);

  hline(x1 + 1, y1, x2 - 1, c, t);
}

void Bitmap::setpixel_solid(int x, int y, int c2, int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int c1 = data[row[y] + x];

  data[row[y] + x] = blend->current(c1, c2, t);
}

void Bitmap::setpixel_wrap(int x, int y, int c2, int t)
{
  while(x < cl)
    x += cw;
  while(x > cr)
    x -= cw;
  while(y < ct)
    y += ch;
  while(y > cb)
    y -= ch;

  int c1 = data[row[y] + x];

  data[row[y] + x] = blend->current(c1, c2, t);
}

void Bitmap::setpixel_clone(int x, int y, int c2, int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int c1 = getpixel(x, y);

  int x1 = x - var->deltax;
  int y1 = y - var->deltay;

  int w1 = w - 1;
  int h1 = h - 1;

  switch(var->mirror)
  {
    case 0:
      x1 = x1;
      y1 = y1;
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = y1;
      break;
    case 2:
      x1 = x1;
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
  }

  if(x1 >= stroke->x1 && x1 < stroke->x2 && y1 >= stroke->y1 && y1 < stroke->y2)
    c2 = bmp->clone->getpixel(x1 - stroke->x1, y1 - stroke->y1);
  else
    c2 = bmp->main->getpixel(x1, y1);

  data[row[y] + x] = blend->current(c1, c2, t);
}

void Bitmap::setpixel_wrap_clone(int x, int y, int c2, int t)
{
  while(x < cl)
    x += cw;
  while(x > cr)
    x -= cw;
  while(y < ct)
    y += ch;
  while(y > cb)
    y -= ch;

  int c1 = data[row[y] + x];

  int x1 = x - var->deltax;
  int y1 = y - var->deltay;

  int w1 = w - 1;
  int h1 = h - 1;

  switch(var->mirror)
  {
    case 0:
      x1 = x1;
      y1 = y1;
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = y1;
      break;
    case 2:
      x1 = x1;
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
  }

  if(x1 >= stroke->x1 && x1 < stroke->x2 && y1 >= stroke->y1 && y1 < stroke->y2)
    c2 = bmp->clone->getpixel(x1 - stroke->x1, y1 - stroke->y1);
  else
    c2 = bmp->clone->getpixel(x1, y1);

  data[row[y] + x] = blend->current(c1, c2, t);
}

int Bitmap::getpixel(int x, int y)
{
  if(var->wrap)
  {
    while(x < cl)
      x += cw;
    while(x > cr)
      x -= cw;
    while(y < ct)
      y += ch;
    while(y > cb)
      y -= ch;
  }
  else
  {
    if(x < cl)
      x = cl;
    if(x > cr)
      x = cr;
    if(y < ct)
      y = ct;
    if(y > cb)
      y = cb;
  }

  return data[row[y] + x];
}

void Bitmap::clip(int *x1, int *y1, int *x2, int *y2)
{
  if(*x1 < cl)
    *x1 = cl;
  if(*x2 > cr)
    *x2 = cr;
  if(*y1 < ct)
    *y1 = ct;
  if(*y2 > cb)
    *y2 = cb;
}

void Bitmap::set_clip(int x, int y, int w, int h)
{
  cl = x;
  cr = w - 1;
  ct = y;
  cb = h - 1;
  cw = w;
  ch = h;
}

void Bitmap::blit(Bitmap *dest, int sx, int sy, int dx, int dy, int ww, int hh)
{
  int x, y;

  if((sx >= w) || (sy >= h) || (dx >= dest->cr) || (dy >= dest->cb))
    return;

  // clip src left
  if(sx < 0)
  {
    ww += sx;
    dx -= sx;
    sx = 0;
  }

  // clip src top
  if(sy < 0)
  {
    hh += sy;
    dy -= sy;
    sy = 0;
  }

  // clip src right
  if((sx + ww) > w)
    ww = w - sx;

  // clip src bottom
  if((sy + hh) > h)
    hh = h - sy;

  // clip dest left
  if(dx < dest->cl)
  {
    dx -= dest->cl;
    ww += dx;
    sx -= dx;
    dx = dest->cl;
  }

  // clip dest top
  if(dy < dest->ct)
  {
    dy -= dest->ct;
    hh += dy;
    sy -= dy;
    dy = dest->ct;
  }

  // clip dest right
  if((dx + ww - 1) > dest->cr)
    ww = dest->cr - dx;

  // clip dest bottom
  if((dy + hh - 1) > dest->cb)
    hh = dest->cb - dy;

  if(ww < 1 || hh < 1)
    return;

  int sy1 = sy;
  int dy1 = dy;
  for(y = 0; y < hh; y++)
  {
    int sx1 = sx + row[sy1];
    int dx1 = dx + dest->row[dy1];
    for(x = 0; x < ww; x++, sx1++, dx1++)
      dest->data[dx1] = data[sx1];
    sy1++;
    dy1++;
  }
}

void Bitmap::point_stretch(Bitmap *dest, int sx, int sy, int sw, int sh,
                                         int dx, int dy, int dw, int dh,
                                         int overx, int overy)
{
  const int ax = ((double)dw / sw) * 256;
  const int ay = ((double)dh / sh) * 256;
  const int bx = ((double)sw / dw) * 256;
  const int by = ((double)sh / dh) * 256;

  dw -= overx;
  dh -= overy;
  if(dw < 1 || dh < 1)
    return;

  if(dx < dest->cl)
  {
    const int d = dest->cl - dx;
    dx = dest->cl;
    dw -= d;
    sx += (d * ax) >> 8;
    sw -= (d * ax) >> 8;
  }

  if(dx + dw > dest->cr)
  {
    const int d = dx + dw - dest->cr;
    dw -= d;
    sw -= (d * ax) >> 8;
  }

  if(dy < dest->ct)
  {
    const int d = dest->ct - dy;
    dy = dest->ct;
    dh -= d;
    sy += (d * ay) >> 8;
    sh -= (d * ay) >> 8;
  }

  if(dy + dh > dest->cb)
  {
    const int d = dy + dh - dest->cb;
    dh -= d;
    sh -= (d * ay) >> 8;
  }

  if(sw < 1 || sh < 1)
    return;

  if(dw < 1 || dh < 1)
    return;

  int y = 0;
  do
  {
    const int y1 = sy + ((y * by) >> 8);
    int x = 0;
    int *s = &dest->data[dest->row[dy + y]];
    do
    {
      const int x1 = sx + ((x * bx) >> 8);
      *s++ = data[row[y1] + x1];
      x++;
    }
    while(x < dw);
    y++;
  }
  while(y < dh);
}


