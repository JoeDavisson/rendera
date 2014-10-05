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

#include <math.h>

#include "Bitmap.H"
#include "Blend.H"
#include "Palette.H"
#include "Stroke.H"
#include "Gui.H"
#include "View.H"
#include "Tool.H"

#include <algorithm>

extern int *fix_gamma;
extern int *unfix_gamma;

Bitmap *Bitmap::main;
Bitmap *Bitmap::preview;
Bitmap *Bitmap::clone_buffer;
Bitmap *Bitmap::offset_buffer;

int Bitmap::wrap = 0;
int Bitmap::clone = 0;
int Bitmap::clone_moved = 0;
int Bitmap::clone_x = 0;
int Bitmap::clone_y = 0;
int Bitmap::clone_dx = 0;
int Bitmap::clone_dy = 0;
int Bitmap::clone_mirror = 0;

namespace
{
  inline int xorValue(const int x, const int y)
  {
    static const int c[2] = { 0x00FFFFFF, 0x00808080 };
    return c[(x & 1) ^ (y & 1)];
  }
}

Bitmap::Bitmap(int width, int height)
{
  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int [width * height];
  row = new int *[height];

  x = 0;
  y = 0;
  w = width;
  h = height;
  overscroll = 0;

  setClip(0, 0, w - 1, h - 1);

  int i;

  for(i = 0; i < height; i++)
    row[i] = &data[width * i];
}

Bitmap::Bitmap(int width, int height, int overscroll)
{
  width += overscroll * 2;
  height += overscroll * 2;

  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int [width * height];
  row = new int *[height];

  x = 0;
  y = 0;
  w = width;
  h = height;
  this->overscroll = overscroll;

  int i;

  for(i = 0; i < height; i++)
    row[i] = &data[width * i];

  setClip(overscroll, overscroll, w - overscroll - 1, h - overscroll - 1);
  clear(getFltkColor(FL_BACKGROUND2_COLOR));
  rectfill(cl, ct, cr, cb, makeRgb(0, 0, 0), 0);

  // draw page border
  setClip(0, 0, w - 1, h - 1);

  for(i = 0; i < 4; i++)
    rect(overscroll - 1 - i, overscroll - 1 - i, w - overscroll + i, h - overscroll + i, makeRgb(56, 56, 56), 0);
  rectfill(overscroll + 4, h - overscroll + 4, w - overscroll + 4, h - overscroll + 4 + 7, makeRgb(48, 48, 48), 0);
  rectfill(w - overscroll + 4, overscroll + 4, w - overscroll + 4 + 7, h - overscroll + 4 + 7, makeRgb(48, 48, 48), 0);

  setClip(overscroll, overscroll, w - overscroll - 1, h - overscroll - 1);
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
    std::swap(x1, x2);

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

  dx = std::abs(dx);
  dy = std::abs(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      setpixelSolid(x1, y1, c, t);

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
      setpixelSolid(x1, y1, c, t);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  setpixelSolid(x1, y1, c, t);
}

void Bitmap::rect(int x1, int y1, int x2, int y2, int c, int t)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

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

  for( int y = y1 + 1; y < y2; y++)
  {
    *(row[y] + x1) = Blend::current(*(row[y] + x1), c, t);
    *(row[y] + x2) = Blend::current(*(row[y] + x2), c, t);
  }
}

void Bitmap::rectfill(int x1, int y1, int x2, int y2, int c, int t)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

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

void Bitmap::xorLine(int x1, int y1, int x2, int y2)
{
  int dx, dy, inx, iny, e;

  dx = x2 - x1;
  dy = y2 - y1;
  inx = dx > 0 ? 1 : -1;
  iny = dy > 0 ? 1 : -1;

  dx = std::abs(dx);
  dy = std::abs(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      *(row[y1] + x1) ^= xorValue(x1, y1);

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
      *(row[y1] + x1) ^= xorValue(x1, y1);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  *(row[y1] + x1) ^= xorValue(x1, y1);
}

void Bitmap::xorHline(int x1, int y, int x2)
{
  if(x1 > x2)
    std::swap(x1, x2);

  if(y < ct || y > cb)
    return;
  if(x1 > cr)
    return;
  if(x2 < cl)
    return;

  clip(&x1, &y, &x2, &y);

  int *p = row[y] + x1;

  for(; x1 <= x2; x1++)
    *p++ ^= xorValue(x1, y);
}

void Bitmap::xorRect(int x1, int y1, int x2, int y2)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  if(x1 > cr)
    return;
  if(x2 < cl)
    return;
  if(y1 > cb)
    return;
  if(y2 < ct)
    return;

  clip(&x1, &y1, &x2, &y2);

  xorHline(x1, y1, x2);
  xorHline(x1, y2, x2);
  if(y1 == y2)
    return;

  y1++;

  for(; y1 < y2; y1++)
  {
    *(row[y1] + x1) ^= xorValue(x1, y1);
    *(row[y1] + x2) ^= xorValue(x1, y1);
  }
}

void Bitmap::xorRectfill(int x1, int y1, int x2, int y2)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  if(x1 > cr)
    return;
  if(x2 < cl)
    return;
  if(y1 > cb)
    return;
  if(y2 < ct)
    return;

  clip(&x1, &y1, &x2, &y2);

  for(; y1 <= y2; y1++)
    xorHline(x1, y1, x2);
}

void Bitmap::setpixel(int x, int y, int c2, int t)
{
  int mode = Bitmap::wrap | (Bitmap::clone << 1);
  Blend::target(Bitmap::main, x, y);

  switch(mode)
  {
    case 0:
      setpixelSolid(x, y, c2, t);
      break;
    case 1:
      setpixelWrap(x, y, c2, t);
      break;
    case 2:
      setpixelClone(x, y, c2, t);
      break;
    case 3:
      setpixelWrapClone(x, y, c2, t);
      break;
    default:
      setpixelSolid(x, y, c2, t);
      break;
  }
}

void Bitmap::setpixelSolid(int x, int y, int c2, int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixelWrap(int x, int y, int c2, int t)
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

void Bitmap::setpixelClone(int x, int y, int c2, int t)
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
      /* x1 = x1; */
      /* y1 = y1; */
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      /* y1 = y1; */
      break;
    case 2:
      /* x1 = x1; */
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
  }

  Stroke *stroke = Gui::getView()->tool->stroke;

  if(x1 >= stroke->x1 && x1 <= stroke->x2 &&
     y1 >= stroke->y1 && y1 <= stroke->y2)
  {
    c2 = Bitmap::clone_buffer->getpixel(x1 - stroke->x1, y1 - stroke->y1);
  }
  else
  {
    c2 = getpixel(x1, y1);
  }

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixelWrapClone(int x, int y, int c2, int t)
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
      /* x1 = x1; */
      /* y1 = y1; */
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      /* y1 = y1; */
      break;
    case 2:
      /* x1 = x1; */
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
  }

  while(x1 < cl)
    x1 += cw;
  while(x1 > cr)
    x1 -= cw;
  while(y1 < ct)
    y1 += ch;
  while(y1 > cb)
    y1 -= ch;

  Stroke *stroke = Gui::getView()->tool->stroke;

  if(x1 >= stroke->x1 && x1 <= stroke->x2 &&
     y1 >= stroke->y1 && y1 <= stroke->y2)
  {
    c2 = Bitmap::clone_buffer->getpixel(x1 - stroke->x1, y1 - stroke->y1);
  }
  else
  {
    c2 = getpixel(x1, y1);
  }

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

void Bitmap::setClip(int x1, int y1, int x2, int y2)
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

void Bitmap::pointStretch(Bitmap *dest,
                           int sx, int sy, int sw, int sh,
                           int dx, int dy, int dw, int dh,
                           int overx, int overy, int bgr_order)
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
    int *s = dest->row[dy + y] + dx;

    for(x = 0; x < dw; x++)
    {
      const int x1 = sx + ((x * bx) >> 8);
      const int c = *(row[y1] + x1);

      *s++ = convertFormat(blendFast(((x >> 4) & 1) ^ ((y >> 4) & 1)
                            ? 0xA0A0A0 : 0x606060, c,
                            255 - geta(c)), bgr_order);
    }
  }
}

// warning: does not clip
void Bitmap::scaleBilinear(Bitmap *dest,
                           int sx, int sy, int sw, int sh,
                           int dx, int dy, int dw, int dh,
                           int wrap_edges)
{
  const float ax = ((float)sw / dw);
  const float ay = ((float)sh / dh);

  if(sw < 1 || sh < 1)
    return;

  if(dw < 1 || dh < 1)
    return;

  // average colors if scaling down
  int mipx = 1, mipy = 1;
  if(sw > dw)
    mipx = (sw / dw);
  if(sh > dh)
    mipy = (sh / dh);

  int x, y;
  int div = mipx * mipy;

  if((mipx > 1) | (mipy > 1))
  {
    for(y = 0; y <= sh - mipy; y += mipy)
    {
      for(x = 0; x <= sw - mipx; x += mipx)
      {
        int r = 0, g = 0, b = 0;
        int i, j, c;
        for(j = 0; j < mipy; j++)
        {
          for(i = 0; i < mipx; i++)
          {
            c = getpixel(sx + x + i, sy + y + j);
            struct rgba_t rgba = getRgba(c);
            r += fix_gamma[rgba.r];
            g += fix_gamma[rgba.g];
            b += fix_gamma[rgba.b];
          }
        }
        r /= div;
        g /= div;
        b /= div;
        r = unfix_gamma[r];
        g = unfix_gamma[g];
        b = unfix_gamma[b];
        c = makeRgb(r, g, b);
        for(j = 0; j < mipy; j++)
        {
          for(i = 0; i < mipx; i++)
          {
            setpixel(sx + x + i, sy + y + j, c, 0);
          }
        }
      }
    }
  }

  y = 0;

  do
  {
    int *d = dest->row[dy + y] + dx;

    float vv = (y * ay);
    int v1 = vv;
    float v = vv - v1;
    if(sy + v1 >= h - 1)
      break;
    int v2 = v1 + 1;
    if(v2 >= sh)
    {
      if(wrap_edges)
        v2 -= sh;
      else
        v2--;
    }

    int *c[4];
    c[0] = c[1] = row[sy + v1] + sx;
    c[2] = c[3] = row[sy + v2] + sx;

   x = 0;
    do
    {
      float uu = (x * ax);
      int u1 = uu;
      float u = uu - u1;
      if(sx + u1 >= w - 1)
        break;
      int u2 = u1 + 1;
      if(u2 >= sw)
      {
        if(wrap_edges)
          u2 -= sw;
        else
          u2--;
      }

      c[0] += u1;
      c[1] += u2;
      c[2] += u1;
      c[3] += u2;

      float f[4];

      f[0] = (1.0f - u) * (1.0f - v);
      f[1] = u * (1.0f - v);
      f[2] = (1.0f - u) * v;
      f[3] = u * v;

      float r = 0.0f, g = 0.0f, b = 0.0f;
      int i = 0;
      do
      {
        struct rgba_t rgba = getRgba(*c[i]);
        r += (float)fix_gamma[rgba.r] * f[i];
        g += (float)fix_gamma[rgba.g] * f[i];
        b += (float)fix_gamma[rgba.b] * f[i];
        i++;
      }
      while(i < 4);
      r = unfix_gamma[(int)r];
      g = unfix_gamma[(int)g];
      b = unfix_gamma[(int)b];

      *d = makeRgb((int)r, (int)g, (int)b);
      d++;

      c[0] -= u1;
      c[1] -= u2;
      c[2] -= u1;
      c[3] -= u2;

      x++;
    }
    while(x < dw);

    y++;
  }
  while(y < dh);
}

void Bitmap::mirror()
{
  int x, y;

  for(y = ct; y <= cb; y++)
  {
    for(x = cl; x < w / 2; x++)
    {
      const int temp = *(row[y] + x);
      *(row[y] + x) = *(row[y] + w - 1 - x);
      *(row[y] + w - 1 - x) = temp;
    }
  }
}

void Bitmap::flip()
{
  int x, y;

  for(y = ct; y < h / 2; y++)
  {
    for(x = cl; x <= cr; x++)
    {
      const int temp = *(row[y] + x);
      *(row[y] + x) = *(row[h - 1 - y] + x);
      *(row[h - 1 - y] + x) = temp;
    }
  }
}

void Bitmap::fastStretch(Bitmap *dest,
                          int xs1, int ys1, int xs2, int ys2,
                          int xd1, int yd1, int xd2, int yd2, int bgr_order)
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

  dx = std::abs(yd2 - yd1);
  dy = std::abs(ys2 - ys1);
  sx = sign(yd2 - yd1);
  sy = sign(ys2 - ys1);
  dy <<= 1;
  e = dy - dx;
  dx2 = dx << 1;

  for(d = 0; d <= dx; d++)
  {
    int dx_1, dy_1, e_1, d_1, dx2_1;
    int sx_1, sy_1;
    int *p, *q;

    dx_1 = std::abs(xd2 - xd1);
    dy_1 = std::abs(xs2 - xs1);
    sx_1 = sign(xd2 - xd1);
    sy_1 = sign(xs2 - xs1);
    dy_1 <<= 1;
    e_1 = dy_1 - dx_1;
    dx2_1 = dx_1 << 1;

    p = dest->row[yd1] + xd1;
    q = row[ys1] + xs1;

    for(d_1 = 0; d_1 <= dx_1; d_1++)
    {
      *p = convertFormat(blendFast(((d_1 >> 4) & 1) ^ ((yd1 >> 4) & 1)
                            ? 0xA0A0A0 : 0x606060, *q,
                            255 - geta(*q)), bgr_order);

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

