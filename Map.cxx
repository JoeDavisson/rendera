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

#include "Map.H"

Map *Map::main;

namespace
{
  // helper for polyfill
  inline int isLeft(const int *x1, const int *y1, const int *x2,
                           const int *y2, const int &x3, const int &y3)
  {
    return ((*x2 - *x1) * (y3 - *y1) - (x3 - *x1) * (*y2 - *y1));
  }
}

// The "Map" is an 8-bit image used to buffer brushstrokes
// before being rendered.
Map::Map(int width, int height)
{
  int i;

  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new unsigned char [width * height];
  row = new unsigned char *[height];

  w = width;
  h = height;

  for(i = 0; i < height; i++)
    row[i] = &data[width * i];
}

Map::~Map()
{
  delete[] row;
  delete[] data;
}

void Map::clear(int c)
{
  int i;

  for(i = 0; i < w * h; i++)
    data[i] = c & 0xff;
}

void Map::setpixel(int x, int y, int c)
{
  if(x < 0 || x >= w || y < 0 || y >= h)
    return;

  *(row[y] + x) = c & 0xff;
}

int Map::getpixel(int x, int y)
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

  dx = std::abs(dx);
  dy = std::abs(dy);

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
  int w = std::abs(x2 - x1);
  int h = std::abs(y2 - y1);
  int x, y;
  int ex, ey;

  // need 64-bit values here, 32-bit will overflow on very large ovals
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
  int w = std::abs(x2 - x1);
  int h = std::abs(y2 - y1);
  int x, y;
  int ex, ey;

  // need 64-bit values here, 32-bit will overflow on very large ovals
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

  int y;

  for(y = y1 + 1; y < y2; y++)
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

void Map::polyfill(int *polycachex, int *polycachey, int polycount, int x1, int y1, int x2, int y2, int c)
{
  int x, y, i;

  for(y = y1; y < y2; y++)
  {
    for(x = x1; x < x2; x++)
    {
      int inside = 0;
      int *px1 = &polycachex[0];
      int *py1 = &polycachey[0];
      int *px2 = &polycachex[1];
      int *py2 = &polycachey[1];

      for(i = 0; i < polycount - 1; i++)
      {
        if(*py1 <= y)
        {
          if((*py2 > y) && (isLeft(px1, py1, px2, py2, x, y) > 0))
            inside++;
        }
        else
        {
          if((*py2 <= y) && (isLeft(px1, py1, px2, py2, x, y) < 0))
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

void Map::blend(int x, int y, int c)
{
  if(x < 0 || x >= w || y < 0 || y >= h)
    return;

  int c1 = *(row[y] + x);
  c1 += c;
  if(c1 > 255)
    c1 = 255;

  *(row[y] + x) = c1;
}

void Map::setpixelAA(int x, int y, int c)
{
  if(c == 0 || x < 0 || x >= (w << AA_SHIFT) || y < 0 || (y >= (h << AA_SHIFT)))
    return;

  int uu = (x << 8) >> AA_SHIFT;
  int u1 = uu >> 8;
  int u = ((uu - (u1 << 8)) << 4) >> 8;
  int vv = (y << 8) >> AA_SHIFT;
  int v1 = vv >> 8;
  int v = ((vv - (v1 << 8)) << 4) >> 8;

  int u16 = 16 - u;
  int v16 = 16 - v;

  int a = (u16 | (u << 8)) * (v16 | (v16 << 8));
  int b = (u16 | (u << 8)) * (v | (v << 8));

  int f[4];

  f[0] = (a & 0x000001FF);
  f[1] = (a & 0x01FF0000) >> 16;
  f[2] = (b & 0x000001FF);
  f[3] = (b & 0x01FF0000) >> 16;

  int i;
  for(i = 0; i < 4; i++)
    f[i] = std::min(((f[i] << 4) >> 8), 255);

  int xx = (x >> AA_SHIFT);
  int yy = (y >> AA_SHIFT);

  blend(xx, yy, f[0]);
  blend(xx + 1, yy, f[1]);
  blend(xx, yy + 1, f[2]);
  blend(xx + 1, yy + 1, f[3]);
}

void Map::lineAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= AA_SHIFT;
  y1 <<= AA_SHIFT;
  x2 <<= AA_SHIFT;
  y2 <<= AA_SHIFT;

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
      setpixelAA(x1, y1, c);

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
      setpixelAA(x1, y1, c);

      if(e >= 0)
      {
        x1 += inx;
        e -= dy;
      }

      e += dx;
      y1 += iny;
    }
  }

  setpixelAA(x1, y1, c);
}

void Map::ovalAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= AA_SHIFT;
  y1 <<= AA_SHIFT;
  x2 <<= AA_SHIFT;
  y2 <<= AA_SHIFT;

  int w = std::abs(x2 - x1);
  int h = std::abs(y2 - y1);
  int x, y;
  int ex, ey;

  // need 64-bit values here, 32-bit will overflow on very large ovals
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
    setpixelAA(x1, y1, c);
    setpixelAA(x1 + ex, y1, c);
    setpixelAA(x1, y1 + ey, c);
    setpixelAA(x1 + ex, y1 + ey, c);
    return;
  }

  if(h <= 0 && w >= 0)
  {
    hlineAA(x1, y1, x2, c);
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

  setpixelAA(x1 + x + ex, y1 + y + ey, c);
  setpixelAA(x1 - x, y1 + y + ey, c);
  setpixelAA(x1 - x, y1 - y, c);
  setpixelAA(x1 + x + ex, y1 - y, c);

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

    setpixelAA(x1 + x + ex, y1 + y + ey, c);
    setpixelAA(x1 - x, y1 + y + ey, c);
    setpixelAA(x1 - x, y1 - y, c);
    setpixelAA(x1 + x + ex, y1 - y, c);
  }
  while(y > 1);

  y--;

  hlineAA(x1 - w / 2, y1, x1 - x, c);
  hlineAA(x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if(ey)
  {
    hline(x1 - w / 2, y1 + 1, x1 - x, c);
    hline(x1 + x + ex, y1 + 1, x1 + w / 2 + ex, c);
  }
}

void Map::ovalfillAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= AA_SHIFT;
  y1 <<= AA_SHIFT;
  x2 <<= AA_SHIFT;
  y2 <<= AA_SHIFT;

  int ww = std::abs(x2 - x1);
  int hh = std::abs(y2 - y1);
  int x, y;
  int ex, ey;

  // need 64-bit values here, 32-bit will overflow on very large ovals
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

      hlineAA(x1 - x, y1 + y + ey, x1 + x + ex, c);
      hlineAA(x1 - x, y1 - y, x1 + x + ex, c);
    }
  }
  while(y > 1);

  y--;

  if(ey)
    hlineAA(x1 - x, y1 + y + ey, x1 + x + ex, c);

  hlineAA(x1 - x, y1 - y, x1 + x + ex, c);
}

void Map::rectAA(int x1, int y1, int x2, int y2, int c)
{
  lineAA(x1, y1, x2, y1, c);
  lineAA(x1, y2, x2, y2, c);
  lineAA(x1, y1, x1, y2, c);
  lineAA(x2, y1, x2, y2, c);
}

void Map::rectfillAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= AA_SHIFT;
  y1 <<= AA_SHIFT;
  x2 <<= AA_SHIFT;
  y2 <<= AA_SHIFT;

  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  for(; y1 <= y2; y1++)
    hlineAA(x1, y1, x2, c);
}

void Map::hlineAA(int x1, int y, int x2, int c)
{
  int x;

  for(x = x1; x <= x2; x++)
    setpixelAA(x, y, c);
}

void Map::polyfillAA(int *polycachex, int *polycachey, int polycount, int x1, int y1, int x2, int y2, int c)
{
  x1 <<= AA_SHIFT;
  y1 <<= AA_SHIFT;
  x2 <<= AA_SHIFT;
  y2 <<= AA_SHIFT;

  int x, y, i;

  for(i = 0; i < polycount; i++)
  {
    polycachex[i] <<= AA_SHIFT;
    polycachey[i] <<= AA_SHIFT;
  }

  for(y = y1; y < y2; y++)
  {
    for(x = x1; x < x2; x++)
    {
      int inside = 0;
      int *px1 = &polycachex[0];
      int *py1 = &polycachey[0];
      int *px2 = &polycachex[1];
      int *py2 = &polycachey[1];

      for(i = 0; i < polycount - 1; i++)
      {
        if(*py1 <= y)
        {
          if((*py2 > y) && (isLeft(px1, py1, px2, py2, x, y) > 0))
            inside++;
        }
        else
        {
          if((*py2 <= y) && (isLeft(px1, py1, px2, py2, x, y) < 0))
            inside++;
        }

        px1++;
        py1++;
        px2++;
        py2++;
      }

      if(inside & 1)
        setpixelAA(x, y, c);
    }
  }
}

