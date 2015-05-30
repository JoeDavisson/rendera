/*
Copyright (c) 2015 Joe Davisson.

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
#include <cstdlib>
#include <stdint.h>

#include "Map.H"
#include "Math.H"

namespace
{
  // add weighted value to real pixel
  inline void _blendAA(const Map *map,
                       const int &x, const int &y, const int &c)
  {
    int c1 = *(map->row[y] + x);

    c1 += c;

    if(c1 > 255)
      c1 = 255;

    *(map->row[y] + x) = c1;
  }

  // draw antialiased pixel
  // each real pixel is treated like 4x4 virtual pixels
  inline void _setpixelAA(const Map *map,
                          const int &x, const int &y, const int &c)
  {
    if(c == 0 ||
      x < 0 || x >= ((map->w - 1) << 2) ||
      y < 0 || y >= ((map->h - 1) << 2))
      return;

    int shift1 = 4;
    int shift2 = 20;

    if(map->thick_aa)
    {
      shift1 = 2;
      shift2 = 18;
    }

    const int uu = x << 2;
    const int u = uu - ((uu >> 4) << 4);
    const int u16 = 16 - u;
    const int vv = y << 2;
    const int v = vv - ((vv >> 4) << 4);
    const int v16 = 16 - v;
    const int a = (u16 | (u << 8)) * (v16 | (v16 << 8));
    const int b = (u16 | (u << 8)) * (v | (v << 8));
    const int xx = (x >> 2);
    const int yy = (y >> 2);

    _blendAA(map, xx, yy, (a & 0x000001FF) >> shift1);
    _blendAA(map, xx + 1, yy, (a & 0x01FF0000) >> shift2);
    _blendAA(map, xx, yy + 1, (b & 0x000001FF) >> shift1);
    _blendAA(map, xx + 1, yy + 1, (b & 0x01FF0000) >> shift2);
  }

  // draw horizontal antialised line (used by filled oval/rectangle)
  inline void _hlineAA(const Map *map,
                       const int &x1, const int &y, const int &x2, const int &c)
  {
    for(int x = x1; x <= x2; x++)
      _setpixelAA(map, x, y, c);
  }
}

// The "Map" is an 8-bit image used to buffer brushstrokes
// before being rendered.
Map::Map(int width, int height)
{
  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new unsigned char [width * height];
  row = new unsigned char *[height];

  w = width;
  h = height;

  for(int i = 0; i < height; i++)
    row[i] = &data[width * i];

  thick_aa = 0;
}

Map::~Map()
{
  delete[] row;
  delete[] data;
}

void Map::clear(int c)
{
  for(int i = 0; i < w * h; i++)
    data[i] = c & 0xff;
}

void Map::setpixel(const int &x, const int &y, const int &c)
{
  if(x < 0 || x >= w || y < 0 || y >= h)
    return;

  *(row[y] + x) = c & 0xff;
}

int Map::getpixel(const int &x, const int &y)
{
  if(x < 0 || x >= w || y < 0 || y >= h)
    return 0;

  return *(row[y] + x) & 0xff;
}

void Map::line(int x1, int y1, int x2, int y2, int c)
{
  int dx, dy, inx, iny, e;

  dx = x2 - x1;
  dy = y2 - y1;
  inx = dx > 0 ? 1 : -1;
  iny = dy > 0 ? 1 : -1;

  dx = Math::abs(dx);
  dy = Math::abs(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      setpixel(x1, y1, c);

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
      setpixel(x1, y1, c);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  setpixel(x1, y1, c);
}

void Map::oval(int x1, int y1, int x2, int y2, int c)
{
  int w = Math::abs(x2 - x1);
  int h = Math::abs(y2 - y1);
  int x, y;
  int ex, ey;

  int64_t a = w / 2;
  int64_t b = h / 2;
  int64_t a2, b2;
  int64_t s, t;

  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  ex = (w & 1);
  ey = (h & 1);

  if(w <= 1 && h <= 1)
  {
    setpixel(x1, y1, c);
    setpixel(x1 + ex, y1, c);
    setpixel(x1, y1 + ey, c);
    setpixel(x1 + ex, y1 + ey, c);
    return;
  }

  if(h <= 0 && w >= 0)
  {
    hline(x1, y1, x2, c);
    return;
  }

  x1 += a;
  y1 += b;

  a2 = a * a;
  b2 = b * b;

  x = 0;
  y = b;

  s = a2 * (1 - 2 * b) + 2 * b2;
  t = b2 - 2 * a2 * (2 * b - 1);

  setpixel(x1 + x + ex, y1 + y + ey, c);
  setpixel(x1 - x, y1 + y + ey, c);
  setpixel(x1 - x, y1 - y, c);
  setpixel(x1 + x + ex, y1 - y, c);

  do
  {
    if(s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
    else
    {
      if(t < 0)
      {
        s += 2 * b2 * (2 * x + 3) - 4 * a2 * (y - 1);
        t += 4 * b2 * (x + 1) - 2 * a2 * (2 * y - 3);
        x++;
        y--;
      }
      else
      {
        s -= 4 * a2 * (y - 1);
        t -= 2 * a2 * (2 * y - 3);
        y--;
      }
    }

    setpixel(x1 + x + ex, y1 + y + ey, c);
    setpixel(x1 - x, y1 + y + ey, c);
    setpixel(x1 - x, y1 - y, c);
    setpixel(x1 + x + ex, y1 - y, c);
  }
  while(y > 0);

  hline(x1 - w / 2, y1, x1 - x, c);
  hline(x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if(ey)
  {
    hline(x1 - w / 2, y1 + 1, x1 - x, c);
    hline(x1 + x + ex, y1 + 1, x1 + w / 2 + ex, c);
  }
}

void Map::ovalfill(int x1, int y1, int x2, int y2, int c)
{
  int w = Math::abs(x2 - x1);
  int h = Math::abs(y2 - y1);
  int x, y;
  int ex, ey;

  int64_t a = w / 2;
  int64_t b = h / 2;
  int64_t a2, b2;
  int64_t s, t;

  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  ex = (w & 1);
  ey = (h & 1);

  if(w <= 1 && h <= 1)
  {
    setpixel(x1, y1, c);
    setpixel(x1 + ex, y1, c);
    setpixel(x1, y1 + ey, c);
    setpixel(x1 + ex, y1 + ey, c);
    return;
  }

  if(h <= 0 && w >= 0)
  {
    hline(x1, y1, x2, c);
    return;
  }

  x1 += a;
  y1 += b;

  a2 = a * a;
  b2 = b * b;

  x = 0;
  y = b;

  s = a2 * (1 - 2 * b) + 2 * b2;
  t = b2 - 2 * a2 * (2 * b - 1);

  setpixel(x1 + x + ex, y1 + y + ey, c);
  setpixel(x1 - x, y1 + y + ey, c);
  setpixel(x1 - x, y1 - y, c);
  setpixel(x1 + x + ex, y1 - y, c);

  do
  {
    if(s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
    else
    {
      if(t < 0)
      {
        s += 2 * b2 * (2 * x + 3) - 4 * a2 * (y - 1);
        t += 4 * b2 * (x + 1) - 2 * a2 * (2 * y - 3);
        x++;
        y--;
      }
      else
      {
        s -= 4 * a2 * (y - 1);
        t -= 2 * a2 * (2 * y - 3);
        y--;
      }
    }

    hline(x1 - x, y1 + y + ey, x1 + x + ex, c);
    hline(x1 - x, y1 - y, x1 + x + ex, c);
  }
  while(y > 0);

  hline(x1 - w / 2, y1, x1 - x, c);
  hline(x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if(ey)
  {
    hline(x1 - w / 2, y1 + 1, x1 - x, c);
    hline(x1 + x + ex, y1 + 1, x1 + w / 2 + ex, c);
  }
}

void Map::rect(int x1, int y1, int x2, int y2, int c)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  hline(x1, y1, x2, c);

  for(int y = y1 + 1; y < y2; y++)
  {
    setpixel(x1, y, c);
    setpixel(x2, y, c);
  }

  hline(x1, y2, x2, c);
}

void Map::rectfill(int x1, int y1, int x2, int y2, int c)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  for(; y1 <= y2; y1++)
    hline(x1, y1, x2, c);
}

void Map::hline(int x1, int y, int x2, int c)
{
  if(x1 < 0)
    x1 = 0;
  if(x1 > w - 1)
    return;
  if(x2 > w - 1)
    x2 = w - 1;
  if(x2 < 0)
    return;
  if(y < 0)
    return;
  if(y > h - 1)
    return;

  unsigned char *x = row[y] + x2;
  unsigned char *z = row[y] + x1;

  do
  {
    *x = c;
    x--;
  }
  while(x >= z);
}

void Map::vline(int y1, int x, int y2, int c)
{
  if(y1 < 0)
    y1 = 0;
  if(y2 > h - 1)
    y2 = h - 1;

  unsigned char *y = row[y2] + x;

  do
  {
    *y = c;
    y -= w;
    y2--;
  }
  while(y2 >= y1);
}

void Map::polyfill(int *polycachex, int *polycachey, int polycount,
                   int x1, int y1, int x2, int y2, int c)
{
  for(int y = y1; y < y2; y++)
  {
    for(int x = x1; x < x2; x++)
    {
      int inside = 0;
      int *px1 = &polycachex[0];
      int *py1 = &polycachey[0];
      int *px2 = &polycachex[1];
      int *py2 = &polycachey[1];

      for(int i = 0; i < polycount - 1; i++)
      {
        if(*py1 <= y)
        {
          if((*py2 > y) &&
             ((*px2 - *px1) * (y - *py1) - (x - *px1) * (*py2 - *py1)) > 0)
            inside++;
        }
        else
        {
          if((*py2 <= y) &&
             ((*px2 - *px1) * (y - *py1) - (x - *px1) * (*py2 - *py1)) < 0)
            inside++;
        }

        px1++;
        py1++;
        px2++;
        py2++;
      }

      if(inside & 1)
        setpixel(x, y, c);
    }
  }
}

void Map::setpixelAA(int x, int y, int c)
{
  _setpixelAA(this, x, y, c);
}

void Map::lineAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= 2;
  y1 <<= 2;
  x2 <<= 2;
  y2 <<= 2;

  int dx, dy, inx, iny, e;

  dx = x2 - x1;
  dy = y2 - y1;
  inx = dx > 0 ? 1 : -1;
  iny = dy > 0 ? 1 : -1;

  dx = Math::abs(dx);
  dy = Math::abs(dy);

  if(dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while(x1 != x2)
    {
      _setpixelAA(this, x1, y1, c);

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
      _setpixelAA(this, x1, y1, c);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }
}

void Map::ovalAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= 2;
  y1 <<= 2;
  x2 <<= 2;
  y2 <<= 2;

  int w = Math::abs(x2 - x1);
  int h = Math::abs(y2 - y1);
  int x, y;
  int ex, ey;

  int64_t a = w / 2;
  int64_t b = h / 2;
  int64_t a2, b2;
  int64_t s, t;

  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  ex = (w & 1);
  ey = (h & 1);

  if(w <= 1 && h <= 1)
  {
    _setpixelAA(this, x1, y1, c);
    _setpixelAA(this, x1 + ex, y1, c);
    _setpixelAA(this, x1, y1 + ey, c);
    _setpixelAA(this, x1 + ex, y1 + ey, c);
    return;
  }

  if(h <= 0 && w >= 0)
  {
    _hlineAA(this, x1, y1, x2, c);
    return;
  }

  x1 += a;
  y1 += b;

  a2 = a * a;
  b2 = b * b;

  x = 0;
  y = b;

  s = a2 * (1 - 2 * b) + 2 * b2;
  t = b2 - 2 * a2 * (2 * b - 1);

  _setpixelAA(this, x1 + x + ex, y1 + y + ey, c);
  _setpixelAA(this, x1 - x, y1 + y + ey, c);
  _setpixelAA(this, x1 - x, y1 - y, c);
  _setpixelAA(this, x1 + x + ex, y1 - y, c);

  do
  {
    if(s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
    else
    {
      if(t < 0)
      {
        s += 2 * b2 * (2 * x + 3) - 4 * a2 * (y - 1);
        t += 4 * b2 * (x + 1) - 2 * a2 * (2 * y - 3);
        x++;
        y--;
      }
      else
      {
        s -= 4 * a2 * (y - 1);
        t -= 2 * a2 * (2 * y - 3);
        y--;
      }
    }

    _setpixelAA(this, x1 + x + ex, y1 + y + ey, c);
    _setpixelAA(this, x1 - x, y1 + y + ey, c);
    _setpixelAA(this, x1 - x, y1 - y, c);
    _setpixelAA(this, x1 + x + ex, y1 - y, c);
  }
  while(y > 1);

  y--;

  _hlineAA(this, x1 - w / 2, y1, x1 - x, c);
  _hlineAA(this, x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if(ey)
  {
    _hlineAA(this, x1 - w / 2, y1 + 1, x1 - x, c);
    _hlineAA(this, x1 + x + ex, y1 + 1, x1 + w / 2 + ex, c);
  }
}

void Map::ovalfillAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= 2;
  y1 <<= 2;
  x2 <<= 2;
  y2 <<= 2;

  int ww = Math::abs(x2 - x1);
  int hh = Math::abs(y2 - y1);
  int x, y;
  int ex, ey;

  int64_t a = ww / 2;
  int64_t b = hh / 2;
  int64_t a2, b2;
  int64_t s, t;

  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  ex = (ww & 1);
  ey = (hh & 1);

  if(ww <= 1 && hh <= 1)
    return;

  if(hh <= 0 && ww >= 0)
    return;

  x1 += a;
  y1 += b;

  a2 = a * a;
  b2 = b * b;

  x = 0;
  y = b;

  s = a2 * (1 - 2 * b) + 2 * b2;
  t = b2 - 2 * a2 * (2 * b - 1);

  do
  {
    if(s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
    else
    {
      if(t < 0)
      {
        s += 2 * b2 * (2 * x + 3) - 4 * a2 * (y - 1);
        t += 4 * b2 * (x + 1) - 2 * a2 * (2 * y - 3);
        x++;
        y--;
      }
      else
      {
        s -= 4 * a2 * (y - 1);
        t -= 2 * a2 * (2 * y - 3);
        y--;
      }

      _hlineAA(this, x1 - x, y1 + y + ey, x1 + x + ex, c);
      _hlineAA(this, x1 - x, y1 - y, x1 + x + ex, c);
    }
  }
  while(y > 1);

  y--;

  if(ey)
    _hlineAA(this, x1 - x, y1 + y + ey, x1 + x + ex, c);

  _hlineAA(this, x1 - x, y1 - y, x1 + x + ex, c);
}

void Map::rectAA(int x1, int y1, int x2, int y2, int c)
{
  lineAA(x1, y1, x2, y1, c);
  lineAA(x2, y1, x2, y2, c);
  lineAA(x2, y2, x1, y2, c);
  lineAA(x1, y2, x1, y1, c);
}

void Map::rectfillAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= 2;
  y1 <<= 2;
  x2 <<= 2;
  y2 <<= 2;

  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  for(; y1 <= y2; y1++)
    _hlineAA(this, x1, y1, x2, c);
}

void Map::polyfillAA(int *polycachex, int *polycachey, int polycount,
                     int x1, int y1, int x2, int y2, int c)
{
  x1 <<= 2;
  y1 <<= 2;
  x2 <<= 2;
  y2 <<= 2;

  for(int i = 0; i < polycount; i++)
  {
    polycachex[i] <<= 2;
    polycachey[i] <<= 2;
  }

  for(int y = y1; y < y2; y++)
  {
    for(int x = x1; x < x2; x++)
    {
      int inside = 0;
      int *px1 = &polycachex[0];
      int *py1 = &polycachey[0];
      int *px2 = &polycachex[1];
      int *py2 = &polycachey[1];

      for(int i = 0; i < polycount - 1; i++)
      {
        if(*py1 <= y)
        {
          if((*py2 > y) &&
             ((*px2 - *px1) * (y - *py1) - (x - *px1) * (*py2 - *py1)) > 0)
            inside++;
        }
        else
        {
          if((*py2 <= y) &&
             ((*px2 - *px1) * (y - *py1) - (x - *px1) * (*py2 - *py1)) < 0)
            inside++;
        }

        px1++;
        py1++;
        px2++;
        py2++;
      }

      if(inside & 1)
        _setpixelAA(this, x, y, c);
    }
  }
}

