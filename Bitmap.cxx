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

#include <algorithm>
#include <cmath>

#include "Bitmap.H"
#include "Blend.H"
#include "Gamma.H"
#include "Gui.H"
#include "Palette.H"
#include "Project.H"
#include "Stroke.H"
#include "Tool.H"
#include "View.H"

Bitmap *Bitmap::clone_buffer = 0;

namespace
{
  inline int xorValue(const int &x, const int &y)
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

  wrap = false;
  clone = false;
  clone_moved = false;
  clone_x = 0;
  clone_y = 0;
  clone_dx = 0;
  clone_dy = 0;
  clone_mirror = 0;
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
  setClip(0, 0, w - 1, h - 1);

  for(i = 0; i < 4; i++)
    rect(overscroll - 1 - i, overscroll - 1 - i,
         w - overscroll + i, h - overscroll + i,
         getFltkColor(FL_BACKGROUND_COLOR), 0);

/*
  rectfill(overscroll + 4, h - overscroll + 4,
           w - overscroll + 4, h - overscroll + 4 + 7,
           makeRgb(48, 48, 48), 0);

  rectfill(w - overscroll + 4, overscroll + 4,
           w - overscroll + 4 + 7, h - overscroll + 4 + 7,
           makeRgb(48, 48, 48), 0);
*/

  setClip(overscroll, overscroll, w - overscroll - 1, h - overscroll - 1);

  wrap = false;
  clone = false;
  clone_moved = false;
  clone_x = 0;
  clone_y = 0;
  clone_dx = 0;
  clone_dy = 0;
  clone_mirror = 0;
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

void Bitmap::setpixel(const int &x, const int &y, const int &c2, const int &t)
{
  Blend::target(this, x, y);

  switch(wrap | (clone << 1))
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

void Bitmap::setpixelSolid(const int &x, const int &y,
                           const int &c2, const int &t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixelWrap(const int &x, const int &y,
                          const int &c2, const int &t)
{
  int x1 = x; 
  int y1 = y; 

  while(x1 < cl)
    x1 += cw;
  while(x1 > cr)
    x1 -= cw;
  while(y1 < ct)
    y1 += ch;
  while(y1 > cb)
    y1 -= ch;

  int *c1 = row[y1] + x1;

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixelClone(const int &x, const int &y,
                           const int &, const int &t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  int x1 = x - Bitmap::clone_dx;
  int y1 = y - Bitmap::clone_dy;

  const int w1 = w - 1;
  const int h1 = h - 1;

  switch(clone_mirror)
  {
    case 0:
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      break;
    case 2:
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      y1 = (h1 - y1) - (h1 - Bitmap::clone_y * 2);
      break;
  }

  Stroke *stroke = Project::stroke.get();

  int c2;

  if(x1 > stroke->x1 && x1 < stroke->x2 &&
     y1 > stroke->y1 && y1 < stroke->y2)
  {
    c2 = Bitmap::clone_buffer->getpixel(x1 - stroke->x1 - 1,
                                        y1 - stroke->y1 - 1);
  }
  else
  {
    c2 = getpixel(x1, y1);
  }

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixelWrapClone(const int &x, const int &y,
                               const int &, const int &t)
{
  int x1 = x; 
  int y1 = y;

  while(x1 < cl)
    x1 += cw;
  while(x1 > cr)
    x1 -= cw;
  while(y1 < ct)
    y1 += ch;
  while(y1 > cb)
    y1 -= ch;

  int *c1 = row[y1] + x1;

  x1 -= Bitmap::clone_dx;
  y1 -= Bitmap::clone_dy;

  const int w1 = w - 1;
  const int h1 = h - 1;

  switch(Bitmap::clone_mirror)
  {
    case 0:
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - Bitmap::clone_x * 2);
      break;
    case 2:
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

  Stroke *stroke = Project::stroke.get();

  int c2;

  if(x1 > stroke->x1 && x1 < stroke->x2 &&
     y1 > stroke->y1 && y1 < stroke->y2)
  {
    c2 = Bitmap::clone_buffer->getpixel(x1 - stroke->x1 - 1,
                                        y1 - stroke->y1 - 1);
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

  if((sx >= w) || (sy >= h) || (dx > dest->cr) || (dy > dest->cb))
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
    ww = dest->cr - dx + 1;

  if((dy + hh - 1) > dest->cb)
    hh = dest->cb - dy + 1;

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
                          int overx, int overy, bool bgr_order)
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
      const int checker = ((x >> 4) & 1) ^ ((y >> 4) & 1) ? 0xA0A0A0 : 0x606060;

      *s++ = convertFormat(blendFast(checker, c, 255 - geta(c)), bgr_order);
    }
  }
}

void Bitmap::scaleBilinear(Bitmap *dest,
                           int sx, int sy, int sw, int sh,
                           int dx, int dy, int dw, int dh,
                           bool wrap_edges)
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
  const int div = mipx * mipy;

  if((mipx > 1) | (mipy > 1))
  {
    for(y = 0; y <= sh - mipy; y += mipy)
    {
      for(x = 0; x <= sw - mipx; x += mipx)
      {
        int r = 0, g = 0, b = 0, a = 0;
        int i, j, c;

        for(j = 0; j < mipy; j++)
        {
          for(i = 0; i < mipx; i++)
          {
            c = getpixel(sx + x + i, sy + y + j);
            struct rgba_t rgba = getRgba(c);
            r += Gamma::fix(rgba.r);
            g += Gamma::fix(rgba.g);
            b += Gamma::fix(rgba.b);
            a += rgba.a;
          }
        }

        r /= div;
        g /= div;
        b /= div;
        a /= div;

        r = Gamma::unfix(r);
        g = Gamma::unfix(g);
        b = Gamma::unfix(b);
        c = makeRgba(r, g, b, a);

        for(j = 0; j < mipy; j++)
        {
          for(i = 0; i < mipx; i++)
          {
            *(row[sy + y + j] + sx + x + i) = c;
          }
        }
      }
    }
  }

  y = 0;

  do
  {
    int *d = dest->row[dy + y] + dx;
    const float vv = (y * ay);
    const int v1 = vv;
    const float v = vv - v1;

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
      const float uu = (x * ax);
      const int u1 = uu;
      const float u = uu - u1;

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

      float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
      int i = 0;

      do
      {
        struct rgba_t rgba = getRgba(*c[i]);
        r += (float)Gamma::fix(rgba.r) * f[i];
        g += (float)Gamma::fix(rgba.g) * f[i];
        b += (float)Gamma::fix(rgba.b) * f[i];
        a += rgba.a * f[i];
        i++;
      }
      while(i < 4);

      r = Gamma::unfix((int)r);
      g = Gamma::unfix((int)g);
      b = Gamma::unfix((int)b);

      *d = makeRgba((int)r, (int)g, (int)b, (int)a);
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

Bitmap *Bitmap::rotate(float angle, float scale, int overscroll, bool tile)
{
  // angle correction
  angle += 90;

  // rotation
  int du_col = (int)((std::sin(angle * (3.14159f / 180)) * scale) * 65536);
  int dv_col = (int)((std::sin((angle + 90) * (3.14159f / 180)) * scale) * 65536);
  int du_row = -dv_col;
  int dv_row = du_col;

  const int ww = (cr - cl) / 2;
  const int hh = (cb - ct) / 2;

  const int xx = cl;
  const int yy = ct;

  // origin
  const int ox = xx + ww;
  const int oy = yy + hh;
	
  // project new corners
  int x0 = xx - ox;
  int y0 = yy - oy;
  int x1 = xx + (ww * 2) - ox;
  int y1 = yy - oy;
  int x2 = xx - ox;
  int y2 = yy + (hh * 2) - oy;
  int x3 = xx + (ww * 2) - ox;
  int y3 = yy + (hh * 2) - oy;

  const int oldx0 = x0;
  const int oldy0 = y0;
  const int oldx1 = x1;
  const int oldy1 = y1;
  const int oldx2 = x2;
  const int oldy2 = y2;
  const int oldx3 = x3;
  const int oldy3 = y3;

  // rotate
  x0 = xx + ((oldx0 * du_col + oldy0 * du_row) >> 16);
  y0 = yy + ((oldx0 * dv_col + oldy0 * dv_row) >> 16);
  x1 = xx + ((oldx1 * du_col + oldy1 * du_row) >> 16);
  y1 = yy + ((oldx1 * dv_col + oldy1 * dv_row) >> 16);
  x2 = xx + ((oldx2 * du_col + oldy2 * du_row) >> 16);
  y2 = yy + ((oldx2 * dv_col + oldy2 * dv_row) >> 16);
  x3 = xx + ((oldx3 * du_col + oldy3 * du_row) >> 16);
  y3 = yy + ((oldx3 * dv_col + oldy3 * dv_row) >> 16);

  // find new bounding box
  const int bx1 = std::min(x0, std::min(x1, std::min(x2, x3))) - scale * 2;
  const int by1 = std::min(y0, std::min(y1, std::min(y2, y3))) - scale * 2;
  const int bx2 = std::max(x0, std::max(x1, std::max(x2, x3))) + scale * 2;
  const int by2 = std::max(y0, std::max(y1, std::max(y2, y3))) + scale * 2;
  int bw = (bx2 - bx1) + 1;
  int bh = (by2 - by1) + 1;

  // create image with new size
  Bitmap *dest = new Bitmap(bw, bh, overscroll);
  dest->rectfill(dest->cl, dest->ct, dest->cr, dest->cb,
                 makeRgba(0, 0, 0, 0), 0);

  bw /= 2;
  bh /= 2;

  // rotation
  du_col = (int)((std::sin(angle * (3.14159f / 180)) / scale) * 65536);
  dv_col = (int)((std::sin((angle + 90) * (3.14159f / 180)) / scale) * 65536);
  du_row = -dv_col;
  dv_row = du_col;

  int row_u = (w / 2) << 16;
  int row_v = (h / 2) << 16;

  row_u -= bw * du_col + bh * du_row;
  row_v -= bw * dv_col + bh * dv_row;

  // draw image
  int x, y;

  for(y = by1; y <= by2; y++)
  {
    int u = row_u;
    int v = row_v;

    row_u += du_row;
    row_v += dv_row;

    const int yy = ((dest->ch) / 2) + y;
    if(yy < dest->ct || yy > dest->cb)
      continue;

    for(x = bx1; x <= bx2; x++)
    {
      int uu = u >> 16;
      int vv = v >> 16;

      u += du_col;
      v += dv_col;

      // clip source image
      if(tile)
      {
        while(uu < cl)
          uu += (cr - cl) + 1;
        while(vv < ct)
          vv += (cb - ct) + 1;
        while(uu > cr)
          uu -= (cr - cl) + 1;
        while(vv > cb)
          vv -= (cb - ct) + 1;
      }
      else
      {
        if(uu < cl || uu > cr || vv < ct || vv > cb)
          continue;
      }

      const int xx = ((dest->cw) / 2) + x;
      if(xx < dest->cl || xx > dest->cr)
        continue;

      int c = *(row[vv] + uu);
      *(dest->row[yy] + xx) = c;
    }
  }

  // return pointer to rotated image
  return dest;
}

void Bitmap::fastStretch(Bitmap *dest,
                         int xs1, int ys1, int xs2, int ys2,
                         int xd1, int yd1, int xd2, int yd2, bool bgr_order)
{
  xs2 += xs1;
  xs2--;
  ys2 += ys1;
  ys2--;
  xd2 += xd1;
  xd2--;
  yd2 += yd1;
  yd2--;

  const int dx = std::abs(yd2 - yd1);
  const int dy = std::abs(ys2 - ys1) << 1;
  const int sx = Common::sign(yd2 - yd1);
  const int sy = Common::sign(ys2 - ys1);
  const int dx2 = dx << 1;

  int e = dy - dx;
  int d;

  for(d = 0; d <= dx; d++)
  {
    const int dx_1 = std::abs(xd2 - xd1);
    const int dy_1 = std::abs(xs2 - xs1) << 1;
    const int sx_1 = Common::sign(xd2 - xd1);
    const int sy_1 = Common::sign(xs2 - xs1);
    const int dx2_1 = dx_1 << 1;

    int e_1 = dy_1 - dx_1;
    int *p = dest->row[yd1] + xd1;
    int *q = row[ys1] + xs1;
    int d_1;

    for(d_1 = 0; d_1 <= dx_1; d_1++)
    {
      const int checker = ((d_1 >> 4) & 1) ^ ((yd1 >> 4) & 1)
                            ? 0xA0A0A0 : 0x606060;

      *p = convertFormat(blendFast(checker, *q, 255 - geta(*q)), bgr_order);

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

void Bitmap::invert()
{
  int i;

  for(i = 0; i < w * h; i++)
  {
    rgba_t rgba = getRgba(data[i]);
    data[i] = makeRgba(255 - rgba.r, 255 - rgba.g, 255 - rgba.b, rgba.a);
  }
}

