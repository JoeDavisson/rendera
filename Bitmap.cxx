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

Bitmap *Bitmap::main;
int Bitmap::wrap = 0;
int Bitmap::clone = 0;
int Bitmap::clone_x = 0;
int Bitmap::clone_y = 0;
int Bitmap::clone_dx = 0;
int Bitmap::clone_dy = 0;
int Bitmap::clone_mirror = 0;

Bitmap::Bitmap(int width, int height)
{
  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int [width * height];
  row = new int *[height];

  w = width;
  h = height;

  set_clip(0, 0, w - 1, h - 1);

  int i;

  for(i = 0; i < height; i++)
    row[i] = &data[width * i];
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

  int *p = row[y] + x1;

  for(x = x1; x <= x2; x++)
  {
    *p = Blend::current(*p, c, t);
    p++;
  }
}

void Bitmap::vline(int y1, int x, int y2, int c, int t)
{
  if(y1 < 0)
    y1 = 0;
  if(y2 > h - 1)
    y2 = h - 1;

  int *y = row[y2] + x;

  do
  {
    *y = Blend::current(*y, c, t);
    y -= w;
    y2--;
  }
  while(y2 >= y1);
}

void Bitmap::line(int x1, int y1, int x2, int y2, int c, int t)
{
  int dx, dy, inx, iny, e;

  dx = x2 - x1;
  dy = y2 - y1;
  inx = dx > 0 ? 1 : -1;
  iny = dy > 0 ? 1 : -1;

  dx = ABS(dx);
  dy = ABS(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      setpixel_solid(x1, y1, c, t);

      if(e >= 0)
      {
        y1 += iny;
        e -= dx;
      }

      e += dy;
      x1 += inx;
    }
  }
  else
  {
    dx <<= 1;
    e = dx - dy;
    dy <<= 1;

    while(y1 != y2)
    {
      setpixel_solid(x1, y1, c, t);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  setpixel_solid(x1, y1, c, t);
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
    *(row[y] + x1) = Blend::current(*(row[y] + x1), c, t);
    *(row[y] + x2) = Blend::current(*(row[y] + x2), c, t);
  }
}

void Bitmap::rectfill(int x1, int y1, int x2, int y2, int c, int t)
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

  for(; y1 <= y2; y1++)
    hline(x1, y1, x2, c, t);
}

// helper for quad
static void store_line(int *xbuf, int *count, int x1, int y1, int x2, int y2)
{
  int dx = x2 - x1;
  int dy = y2 - y1;

  int inx = dx > 0 ? 1 : -1;
  int iny = dy > 0 ? 1 : -1;

  int e;

  dx = ABS(dx);
  dy = ABS(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      xbuf[(*count)] = x1;

      if(e >= 0)
      {
        y1 += iny;
        (*count)++;
        e -= dx;
      }

      e += dy;
      x1 += inx;
    }
  }
  else
  {
    dx <<= 1;
    e = dx - dy;
    dy <<= 1;

    while(y1 != y2)
    {
      xbuf[(*count)] = x1;

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
      (*count)++;
    }
  }

  xbuf[(*count)++] = x1;
}

void Bitmap::quad(int *px, int *py, int c, int t)
{
  // sort vertically
  if(py[0] > py[1])
  {
    SWAP(px[0], px[1]);
    SWAP(py[0], py[1]);
  }

  if(py[2] > py[3])
  {
    SWAP(px[2], px[3]);
    SWAP(py[2], py[3]);
  }

  if(py[0] > py[2])
  {
    SWAP(px[0], px[2]);
    SWAP(py[0], py[2]);
  }

  if(py[1] > py[3])
  {
    SWAP(px[1], px[3]);
    SWAP(py[1], py[3]);
  }

  if(py[1] > py[2])
  {
    SWAP(px[1], px[2]);
    SWAP(py[1], py[2]);
  }

  // figure out which inner point is left/right
  int left = 1;
  int right = 2;

  if(px[1] > px[2])
  {
    left = 2;
    right = 1;
  }

  int *left_buf = new int[(py[3] - py[0]) + 8];
  int *right_buf = new int[(py[3] - py[0]) + 8];

  // store left points
  int count = 0;
  store_line(left_buf, &count, px[0], py[0], px[left], py[left]);
  store_line(left_buf, &count, px[left], py[left], px[3], py[3]);

  // store right points
  count = 0;
  store_line(right_buf, &count, px[0], py[0], px[right], py[right]);
  store_line(right_buf, &count, px[right], py[right], px[3], py[3]);

  // render
  int i = 0;
  int y;

  for(y = py[0]; y <= py[3]; y++)
  {
    hline(left_buf[i], y, right_buf[i], c, t);
    i++;
  }

  delete[] left_buf;
  delete[] right_buf;
}

void Bitmap::xor_line(int x1, int y1, int x2, int y2)
{
  int dx, dy, inx, iny, e;

  dx = x2 - x1;
  dy = y2 - y1;
  inx = dx > 0 ? 1 : -1;
  iny = dy > 0 ? 1 : -1;

  dx = ABS(dx);
  dy = ABS(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      *(row[y1] + x1) ^= 0x00FFFFFF;

      if(e >= 0)
      {
        y1 += iny;
        e -= dx;
      }

      e += dy;
      x1 += inx;
    }
  }
  else
  {
    dx <<= 1;
    e = dx - dy;
    dy <<= 1;

    while(y1 != y2)
    {
      *(row[y1] + x1) ^= 0x00FFFFFF;

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  *(row[y1] + x1) ^= 0x00FFFFFF;
}

void Bitmap::xor_hline(int x1, int y, int x2)
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

  int *p = row[y] + x1;

  for(x = x1; x <= x2; x++)
  {
    *p ^= 0x00FFFFFF;
    p++;
  }
}

void Bitmap::xor_rect(int x1, int y1, int x2, int y2)
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

  xor_hline(x1, y1, x2);
  xor_hline(x1, y2, x2);
  if(y1 == y2)
    return;

  int x, y;

  for(y = y1 + 1; y < y2; y++)
  {
    *(row[y] + x1) ^= 0x00FFFFFF;
    *(row[y] + x2) ^= 0x00FFFFFF;
  }
}

void Bitmap::xor_rectfill(int x1, int y1, int x2, int y2)
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

  for(; y1 <= y2; y1++)
    xor_hline(x1, y1, x2);
}

void Bitmap::setpixel(int x, int y, int c2, int t)
{
  int mode = Bitmap::wrap | (Bitmap::clone << 1);

  switch(mode)
  {
    case 0:
      setpixel_solid(x, y, c2, t);
      break;
    case 1:
      setpixel_wrap(x, y, c2, t);
      break;
    case 2:
      setpixel_clone(x, y, c2, t);
      break;
    case 3:
      setpixel_wrap_clone(x, y, c2, t);
      break;
    default:
      setpixel_solid(x, y, c2, t);
      break;
  }
}

void Bitmap::setpixel_solid(int x, int y, int c2, int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  *c1 = Blend::current(*c1, c2, t);
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

  int *c1 = row[y] + x;

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixel_clone(int x, int y, int c2, int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  int x1 = x - Bitmap::clone_dx;
  int y1 = y - Bitmap::clone_dy;

  int w1 = w - 1;
  int h1 = h - 1;

  switch(Bitmap::clone_mirror)
  {
    case 0:
      x1 = x1;
      y1 = y1;
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = y1;
      break;
    case 2:
      x1 = x1;
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
  }

  c2 = getpixel(x1, y1);

  *c1 = Blend::current(*c1, c2, t);
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

  int *c1 = row[y] + x;

  int x1 = x - Bitmap::clone_dx;
  int y1 = y - Bitmap::clone_dy;

  int w1 = w - 1;
  int h1 = h - 1;

  switch(Bitmap::clone_mirror)
  {
    case 0:
      x1 = x1;
      y1 = y1;
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = y1;
      break;
    case 2:
      x1 = x1;
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
  }

  c2 = getpixel(x1, y1);

  *c1 = Blend::current(*c1, c2, t);
}

int Bitmap::getpixel(int x, int y)
{
  if(x < cl)
    x = cl;
  if(x > cr)
    x = cr;
  if(y < ct)
    y = ct;
  if(y > cb)
    y = cb;

  return *(row[y] + x);
}

int Bitmap::getpixel_wrap(int x, int y)
{
  while(x < cl)
    x += cw;
  while(x > cr)
    x -= cw;
  while(y < ct)
  y += ch;
  while(y > cb)
    y -= ch;

  return *(row[y] + x);
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
  cw = (cr - cl) + 1;
  ch = (cb - ct) + 1;
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
    int *sx1 = sx + row[sy1];
    int *dx1 = dx + dest->row[dy1];
    for(x = 0; x < ww; x++, sx1++, dx1++)
     *dx1 = *sx1;
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
    int *s = dest->row[dy + y];
    for(x = 0; x < dw; x++)
    {
      const int x1 = sx + ((x * bx) >> 8);
      *s++ = *(row[y1] + x1);
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
    const int vv = (y * by);
    const int v1 = vv >> 8;
    const int v = ((vv - (v1 << 8)) << 4) >> 8;
    if(sy + v1 >= h - 1)
      break;
    const int v2 = (v1 < (sh - 1) ? v1 + 1 : v1);

    int *c[4];
    c[0] = c[1] = row[sy + v1] + sx;
    c[2] = c[3] = row[sy + v2] + sx;

    int *d = dest->row[dy + y] + dx;
    int x;

    for(x = 0; x < dw; x++)
    {
      const int uu = (x * bx);
      const int u1 = uu >> 8;
      const int u = ((uu - (u1 << 8)) << 4) >> 8;
      if(sx + u1 >= w - 1)
        break;
      const int u2 = (u1 < (sw - 1) ? u1 + 1 : u1);

      c[0] += u1;
      c[1] += u2;
      c[2] += u1;
      c[3] += u2;

      int f[4];

      const int u16 = 16 - u;
      const int v16 = 16 - v;

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
      }

      *d++ = rb | g | 0xFF000000;

      c[0] -= u1;
      c[1] -= u2;
      c[2] -= u1;
      c[3] -= u2;
    }
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
    int dx_1, dy_1, e_1, d_1, dx2_1;
    int sx_1, sy_1;
    int *p, *q;

    dx_1 = ABS(xd2 - xd1);
    dy_1 = ABS(xs2 - xs1);
    sx_1 = SIGN(xd2 - xd1);
    sy_1 = SIGN(xs2 - xs1);
    dy_1 <<= 1;
    e_1 = dy_1 - dx_1;
    dx2_1 = dx_1 << 1;

    p = dest->row[yd1] + xd1;
    q = row[ys1] + xs1;

    for(d_1 = 0; d_1 <= dx_1; d_1++)
    {
      *p = *q;

      while(e_1 >= 0)
      {
        q += sy_1;
        e_1 -= dx2_1;
      }

      p += sx_1;
      e_1 += dy_1;
    }

    while(e >= 0)
    {
      ys1 += sy;
      e -= dx2;
    }

    yd1 += sx;
    e += dy;
  }
}

