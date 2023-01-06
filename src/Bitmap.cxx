/*
Copyright (c) 2021 Joe Davisson.

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
#include <cstdlib>
#include <vector>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Clone.H"
#include "Gamma.H"
#include "FilterMatrix.H"
#include "Inline.H"
#include "Map.H"
#include "Octree.H"
#include "Palette.H"
#include "Project.H"
#include "ExtraMath.H"
#include "Stroke.H"
#include "Tool.H"
#include "View.H"

namespace
{

  // checkerboard pattern
  inline int xorValue(const int x, const int y)
  {
    static const unsigned int xor_colors[2] = { 0x00000000, 0xffffffff };

    return xor_colors[(x & 1) ^ (y & 1)];
  }
}

// creates bitmap
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
  active = false;

  setClip(0, 0, w - 1, h - 1);

  for(int i = 0; i < height; i++)
    row[i] = &data[width * i];

  rectfill(0, 0, w - 1, h - 1, makeRgb(0, 0, 0), 0);
}

// creates bitmap from existing pixel data
Bitmap::Bitmap(int width, int height, int *image_data)
{
  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = image_data;
  row = new int *[height];

  x = 0;
  y = 0;
  w = width;
  h = height;
  active = false;

  for(int i = 0; i < height; i++)
    row[i] = &data[width * i];

  setClip(0, 0, w - 1, h - 1);
}

Bitmap::~Bitmap()
{
  delete[] row;
  delete[] data;
}

bool getm(int c)
{
    return (geta(c) > 0) ? true : false;
}

bool Bitmap::isEdge(int x, int y)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return false;

  if((getm(getpixel(x, y)) &&
    (!getm(getpixel(x - 1, y)) ||
    !getm(getpixel(x + 1, y)) ||
    !getm(getpixel(x, y - 1)) ||
    !getm(getpixel(x, y + 1)))))
    return true;
  else
    return false;
}

void Bitmap::clear(int c)
{
  for(int i = 0; i < w * h; i++)
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

  int *p = row[y] + x1;

  for(int x = x1; x <= x2; x++)
  {
    *p = Blend::current(*p, c, t);
    p++;
  }
}

void Bitmap::vline(int y1, int x, int y2, int c, int t)
{
  if(y1 > y2)
    std::swap(y1, y2);

  if(x < cl || x > cr)
    return;
  if(y1 > cb)
    return;
  if(y2 < ct)
    return;

  clip(&x, &y1, &x, &y2);

  int *p = row[y1] + x;

  for(int y = y1; y <= y2; y++)
  {
    *p = Blend::current(*p, c, t);
    p += w;
  }
}

void Bitmap::line(int x1, int y1, int x2, int y2, int c, int t)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  int inx = dx > 0 ? 1 : -1;
  int iny = dy > 0 ? 1 : -1;

  dx = ExtraMath::abs(dx);
  dy = ExtraMath::abs(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    int e = dy - dx;
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
    int e = dx - dy;
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

  for(int y = y1 + 1; y < y2; y++)
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
  int dx = x2 - x1;
  int dy = y2 - y1;
  int inx = dx > 0 ? 1 : -1;
  int iny = dy > 0 ? 1 : -1;

  dx = ExtraMath::abs(dx);
  dy = ExtraMath::abs(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    int e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      *(row[y1] + x1) = xorValue(x1, y1);

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
    int e = dx - dy;
    dy <<= 1;

    while(y1 != y2)
    {
      *(row[y1] + x1) = xorValue(x1, y1);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  *(row[y1] + x1) = xorValue(x1, y1);
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
    *p++ = xorValue(x1, y);
}

void Bitmap::xorVline(int y1, int x, int y2)
{
  if(y1 > y2)
    std::swap(y1, y2);

  if(x < cl || x > cr)
    return;
  if(y1 > cb)
    return;
  if(y2 < ct)
    return;

  clip(&x, &y1, &x, &y2);

  int *p = row[y1] + x;

  for(; y1 <= y2; y1++)
  {
    *p = xorValue(x, y1);
    p += w;
  }
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
    *(row[y1] + x1) = xorValue(x1, y1);
    *(row[y1] + x2) = xorValue(x1, y1);
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

// non-blending version
void Bitmap::setpixel(const int x, const int y, const int c)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  *(row[y] + x) = c;
}

void Bitmap::setpixel(const int x, const int y, const int c2, const int t)
{
  Blend::target(this, x, y);

//  if(Project::brush->alpha_mask)
//    t = scaleVal(t, 255 - geta(getpixel(x, y)));

  if(Clone::active)
    setpixelClone(x, y, c2, t);
  else
    setpixelSolid(x, y, c2, t);
}

void Bitmap::setpixelSolid(const int x, const int y, const int c2, const int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  *c1 = Blend::current(*c1, c2, t);
}

void Bitmap::setpixelClone(const int x, const int y, const int, const int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int *c1 = row[y] + x;

  int x1 = x - Clone::dx;
  int y1 = y - Clone::dy;

  Stroke *stroke = Project::stroke;

  int c2;

  if(x1 > stroke->x1 && x1 < stroke->x2 &&
     y1 > stroke->y1 && y1 < stroke->y2)
  {
    c2 = Clone::bmp->getpixel(x1 - stroke->x1 - 1, y1 - stroke->y1 - 1);
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

int Bitmap::getpixelNoClip(int x, int y)
{
  if(x < 0)
    x = 0;
  if(x > w - 1)
    x = w - 1;
  if(y < 0)
    y = 0;
  if(y > h - 1)
    y = h - 1;

  return *(row[y] + x);
}


// clips coordinates to the writable image area
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

// sets the image's writeable area
void Bitmap::setClip(int x1, int y1, int x2, int y2)
{
  cl = x1;
  ct = y1;
  cr = x2;
  cb = y2;
  cw = (cr - cl) + 1;
  ch = (cb - ct) + 1;
}

void Bitmap::swapRedBlue()
{
  for(int y = 0; y < h; y++)
  {
    int *p = row[y];

    for(int x = 0; x < w; x++)
    {
      *p = convertFormat(*p, true);
      p++;
    }
  }
}

void Bitmap::blit(Bitmap *dest, int sx, int sy, int dx, int dy, int ww, int hh)
{
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

  for(int y = 0; y < hh; y++)
  {
    int *sx1 = sx + row[sy1];
    int *dx1 = dx + dest->row[dy1];

    for(int x = 0; x < ww; x++, sx1++, dx1++)
      *dx1 = *sx1;

    sy1++;
    dy1++;
  }
}

void Bitmap::transBlit(Bitmap *dest, int sx, int sy, int dx, int dy, int ww, int hh)
{
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

  for(int y = 0; y < hh; y++)
  {
    int *sx1 = sx + row[sy1];
    int *dx1 = dx + dest->row[dy1];

    for(int x = 0; x < ww; x++, sx1++, dx1++)
    {
      const int c = *dx1;

      *dx1 = blendFast(c, *sx1, 255 - geta(*sx1));
    }

    sy1++;
    dy1++;
  }
}

// draws the main viewport
void Bitmap::pointStretch(Bitmap *dest,
                          int sx, int sy, int sw, int sh,
                          int dx, int dy, int dw, int dh,
                          int overx, int overy,
                          int ox, int oy,
                          bool bgr_order)
{
  const int ax = ((float)dw / sw) * 65536;
  const int ay = ((float)dh / sh) * 65536;
  const int bx = ((float)sw / dw) * 65536;
  const int by = ((float)sh / dh) * 65536;

  ox = (ox * ax) >> 16;
  oy = (oy * ay) >> 16;

  dw -= overx;
  dh -= overy;

  if(sx < 0)
    sx = 0;

  if(sy < 0)
    sy = 0;

  if(dx < dest->cl)
  {
    const int d = dest->cl - dx;
    dx = dest->cl;
    dw -= d;
    sx += (d * ax) >> 16;
    sw -= (d * ax) >> 16;
  }

  if(dx + dw > dest->cr)
  {
    const int d = dx + dw - dest->cr;
    dw -= d;
    sw -= (d * ax) >> 16;
  }

  if(dy < dest->ct)
  {
    const int d = dest->ct - dy;
    dy = dest->ct;
    dh -= d;
    sy += (d * ay) >> 16;
    sh -= (d * ay) >> 16;
  }

  if(dy + dh > dest->cb)
  {
    const int d = dy + dh - dest->cb;
    dh -= d;
    sh -= (d * ay) >> 16;
  }

  dw -= (dw - ((sw * ax) >> 16));
  dh -= (dh - ((sh * ay) >> 16));

  if(sw < 1 || sh < 1)
    return;

  if(dw < 1 || dh < 1)
    return;

  for(int y = 0; y < dh; y++)
  {
    const int y1 = sy + ((y * by) >> 16);

    if(y1 < 0 || y1 >= h)
      continue;
    if(dy + y < 0 || dy + y >= dest->h)
      continue;

    for(int x = 0; x < dw; x++)
    {
      const int x1 = sx + ((x * bx) >> 16);

      if(x1 < 0 || x1 >= w)
        continue;
      if(dx + x < 0 || dx + x >= dest->w)
        continue;

      const int c = *(row[y1] + x1);
      const int checker = (((dx + x + ox) >> 3) ^ ((dy + y + oy) >> 3)) & 1
                          ? 0x989898 : 0x686868;

      *(dest->row[dy + y] + dx + x) =
           convertFormat(blendFast(checker, c, 255 - geta(c)), bgr_order);
    }
  }
}

void Bitmap::flipHorizontal()
{
  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w / 2; x++)
    {
      const int temp = *(row[y] + x);
      *(row[y] + x) = *(row[y] + w - 1 - x);
      *(row[y] + w - 1 - x) = temp;
    }
  }
}

void Bitmap::flipVertical()
{
  for(int y = 0; y < h / 2; y++)
  {
    for(int x = 0; x < w; x++)
    {
      const int temp = *(row[y] + x);
      *(row[y] + x) = *(row[h - 1 - y] + x);
      *(row[h - 1 - y] + x) = temp;
    }
  }
}

void Bitmap::rotate180()
{
  const int size = (w * h) / 2;
  int count = 0;

  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
      const int temp = *(row[y] + x);
      *(row[y] + x) = *(row[h - 1 - y] + w - 1 - x);
      *(row[h - 1 - y] + w - 1 - x) = temp;
      count++;
      if(count >= size)
        break; 
    }

    if(count >= size)
      break; 
  }
}

// bresenham stretching
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

  const int dx = ExtraMath::abs(yd2 - yd1);
  const int dy = ExtraMath::abs(ys2 - ys1) << 1;
  const int sx = ExtraMath::sign(yd2 - yd1);
  const int sy = ExtraMath::sign(ys2 - ys1);
  const int dx2 = dx << 1;

  int e = dy - dx;

  for(int d = 0; d <= dx; d++)
  {
    const int dx_1 = ExtraMath::abs(xd2 - xd1);
    const int dy_1 = ExtraMath::abs(xs2 - xs1) << 1;
    const int sx_1 = ExtraMath::sign(xd2 - xd1);
    const int sy_1 = ExtraMath::sign(xs2 - xs1);
    const int dx2_1 = dx_1 << 1;

    int e_1 = dy_1 - dx_1;
    int *p = dest->row[yd1] + xd1;
    int *q = row[ys1] + xs1;

    for(int d_1 = 0; d_1 <= dx_1; d_1++)
    {
      // generate checkboard pattern for transparent areas
      const int checker = ((d_1 >> 4) ^ (yd1 >> 4)) & 1 ? 0xa0a0a0 : 0x606060;

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
  for(int i = 0; i < w * h; i++)
  {
    rgba_type rgba = getRgba(data[i]);
    data[i] = makeRgba(255 - rgba.r, 255 - rgba.g, 255 - rgba.b, rgba.a);
  }
}

