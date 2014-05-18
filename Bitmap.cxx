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
  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int[width * height];
  row = new int[height];

  w = width;
  h = height;

  set_clip(0, 0, w - 1, h - 1);

  int i;

  for(i = 0; i < height; i++)
    row[i] = width * i;
}

Bitmap::~Bitmap()
{
  delete[] row;
  delete[] data;
}

void Bitmap::clear(int c)
{
  int i;
  for(i = 0; i < w * h; i++)
    data[i] = c;
}

void Bitmap::hline(int x1, int y, int x2, int c, int t)
{
  if(x1 > x2)
    SWAP(x1, x2);

  if(y < ct || y > cb)
    return;
  if(x1 > cr)
    return;
  if(x2 < cl)
    return;

  clip(&x1, &y, &x2, &y);

  int x;

  for(x = x1; x <= x2; x++)
    data[row[y] + x] = blend->current(data[row[y] + x], c, t);
}

void Bitmap::rect(int x1, int y1, int x2, int y2, int c, int t)
{
  if(x1 > x2)
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

  if(x1 > cr)
    return;
  if(x2 < cl)
    return;
  if(y1 > cb)
    return;
  if(y2 < ct)
    return;

  clip(&x1, &y1, &x2, &y2);

  hline(x1, y1, x2, c, t);
  hline(x1, y2, x2, c, t);
  if(y1 == y2)
    return;

  int x, y;

  for(y = y1 + 1; y < y2; y++)
  {
    data[row[y] + x1] = blend->current(data[row[y] + x1], c, t);
    data[row[y] + x2] = blend->current(data[row[y] + x2], c, t);
  }
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
  if(*y1 < ct)
    *y1 = ct;
  if(*x2 > cr)
    *x2 = cr;
  if(*y2 > cb)
    *y2 = cb;
}

void Bitmap::set_clip(int x1, int y1, int x2, int y2)
{
  cl = x1;
  ct = y1;
  cr = x2;
  cb = y2;
  cw = w;
  ch = h;
}

void Bitmap::blit(Bitmap *dest, int sx, int sy, int dx, int dy, int ww, int hh)
{
  int x, y;

  if((sx >= w) || (sy >= h) || (dx >= dest->cr) || (dy >= dest->cb))
    return;

  if(sx < 0)
  {
    ww += sx;
    dx -= sx;
    sx = 0;
  }

  if(sy < 0)
  {
    hh += sy;
    dy -= sy;
    sy = 0;
  }

  if((sx + ww) > w)
    ww = w - sx;

  if((sy + hh) > h)
    hh = h - sy;

  if(dx < dest->cl)
  {
    dx -= dest->cl;
    ww += dx;
    sx -= dx;
    dx = dest->cl;
  }

  if(dy < dest->ct)
  {
    dy -= dest->ct;
    hh += dy;
    sy -= dy;
    dy = dest->ct;
  }

  if((dx + ww - 1) > dest->cr)
    ww = dest->cr - dx;

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
  const int ax = ((float)dw / sw) * 256;
  const int ay = ((float)dh / sh) * 256;
  const int bx = ((float)sw / dw) * 256;
  const int by = ((float)sh / dh) * 256;

  dw -= overx;
  dh -= overy;

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

  dw -= (dw - ((sw * ax) >> 8));
  dh -= (dh - ((sh * ay) >> 8));

  if(sw < 1 || sh < 1)
    return;

  if(dw < 1 || dh < 1)
    return;

  int x, y;

  for(y = 0; y < dh; y++)
  {
    const int y1 = sy + ((y * by) >> 8);
    int *s = &dest->data[dest->row[dy + y]];
    for(x = 0; x < dw; x++)
    {
      const int x1 = sx + ((x * bx) >> 8);
      *s++ = data[row[y1] + x1];
    }
  }
}

void Bitmap::integer_stretch(Bitmap *dest, int sx, int sy, int sw, int sh,
                                           int dx, int dy, int dw, int dh,
                                           int overx, int overy)
{
  const int ax = ((float)dw / sw) * 256;
  const int ay = ((float)dh / sh) * 256;
  const int bx = ((float)sw / dw) * 256;
  const int by = ((float)sh / dh) * 256;

  dw -= overx;
  dh -= overy;

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

  dw -= (dw - ((sw * ax) >> 8));
  dh -= (dh - ((sh * ay) >> 8));

  if(sw < 1 || sh < 1)
    return;

  if(dw < 1 || dh < 1)
    return;

  int y;
  for(y = 0; y < dh; y++)
  {
    int vv = (y * by);
    int v1 = vv >> 8;
    int v = ((vv - (v1 << 8)) << 4) >> 8;
    if(sy + v1 >= h - 1)
      break;
    int v2 = (v1 < (sh - 1) ? v1 + 1 : v1);

    int *c[4];
    c[0] = c[1] = &data[row[sy + v1] + sx];
    c[2] = c[3] = &data[row[sy + v2] + sx];

    int *d = &dest->data[dest->row[dy + y] + dx];

    int x;
    for(x = 0; x < dw; x++)
    {
      int uu = (x * bx);
      int u1 = uu >> 8;
      int u = ((uu - (u1 << 8)) << 4) >> 8;
      if(sx + u1 >= w - 1)
        break;
      int u2 = (u1 < (sw - 1) ? u1 + 1 : u1);

      c[0] += u1;
      c[1] += u2;
      c[2] += u1;
      c[3] += u2;

      int f[4];

      int u16 = 16 - u;
      int v16 = 16 - v;

      int a = (u16 | (u << 8)) * (v16 | (v16 << 8));
      int b = (u16 | (u << 8)) * (v | (v << 8));

      f[0] = (a & 0x00000FFF);
      f[1] = (a & 0x0FFF0000) >> 16;
      f[2] = (b & 0x00000FFF);
      f[3] = (b & 0x0FFF0000) >> 16;

      int rb = 0;
      int g = 0;
      int i;
      for(i = 0; i < 4; i++)
      {
        rb += (((*c[i] & 0xFF00FF) * f[i]) >> 8) & 0xFF00FF;
        g += (((*c[i] & 0xFF00) * f[i]) >> 8) & 0xFF00;
        i++;
      }

      *d++ = rb | g | 0xFF000000;

      c[0] -= u1;
      c[1] -= u2;
      c[2] -= u1;
      c[3] -= u2;
    }
  }
}

void Bitmap::stretch_line(Bitmap *dest, int x1, int x2, int y1, int y2,
                                        int yr, int yw)
{
  int dx, dy, e, d, dx2;
  int sx, sy;
  int p, q;

  dx = ABS(x2 - x1);
  dy = ABS(y2 - y1);
  sx = SIGN(x2 - x1);
  sy = SIGN(y2 - y1);
  dy <<= 1;
  e = dy - dx;
  dx2 = dx << 1;

  p = dest->row[yw] + x1;
  q = row[yr] + y1;
  for(d = 0; d <= dx; d++)
  {
    dest->data[p] = data[q];
    while(e >= 0)
    {
      q += sy;
      e -= dx2;
    }
    p += sx;
    e += dy;
  }
}

void Bitmap::fast_stretch(Bitmap *dest,
                          int xs1, int ys1, int xs2, int ys2,
                          int xd1, int yd1, int xd2, int yd2)
{
  xs2 += xs1;
  xs2--;
  ys2 += ys1;
  ys2--;
  xd2 += xd1;
  xd2--;
  yd2 += yd1;
  yd2--;

  int dx, dy, e, d, dx2;
  int sx, sy;

  dx = ABS(yd2 - yd1);
  dy = ABS(ys2 - ys1);
  sx = SIGN(yd2 - yd1);
  sy = SIGN(ys2 - ys1);
  dy <<= 1;
  e = dy - dx;
  dx2 = dx << 1;

  for(d = 0; d <= dx; d++)
  {
    stretch_line(dest, xd1, xd2, xs1, xs2, ys1, yd1);
    while(e >= 0)
    {
      ys1 += sy;
      e -= dx2;

    }
    yd1 += sx;
    e += dy;
  }
}

