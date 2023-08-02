/*
Copyright (c) 2023 Joe Davisson.

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
#include <cmath>
#include <vector>
#include <stdint.h>

#include "Gamma.H"
#include "Inline.H"
#include "Map.H"

namespace
{
  // qsort callback for polyfill
  int node_cmp(const void *a, const void *b)
  {
    return *(int *)a - *(int *)b;
  }
}

// The "Map" is an 8-bit image used to buffer brushstrokes
// before being rendered.
Map::Map(int width, int height)
{
  if (width < 1)
    width = 1;
  if (height < 1)
    height = 1;

  data = new unsigned char [width * height];
  row = new unsigned char *[height];

  w = width;
  h = height;

  for (int i = 0; i < height; i++)
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
  for (int i = 0; i < w * h; i++)
    data[i] = c & 0xff;
}

void Map::invert()
{
  for (int i = 0; i < w * h; i++)
    data[i] = 255 - data[i];
}

void Map::setpixel(int x, int y, int c)
{
  if (x < 0 || x >= w || y < 0 || y >= h)
    return;

  *(row[y] + x) = c & 0xff;
}

int Map::getpixel(int x, int y)
{
  if (x < 0 || x >= w || y < 0 || y >= h)
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

  if (dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while (x1 != x2)
    {
      setpixel(x1, y1, c);

      if (e >= 0)
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

    while (y1 != y2)
    {
      setpixel(x1, y1, c);

      if (e >= 0)
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

  int64_t a = w / 2;
  int64_t b = h / 2;
  int64_t a2, b2;
  int64_t s, t;

  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  ex = (w & 1);
  ey = (h & 1);

  if (w <= 1 && h <= 1)
  {
    setpixel(x1, y1, c);
    setpixel(x1 + ex, y1, c);
    setpixel(x1, y1 + ey, c);
    setpixel(x1 + ex, y1 + ey, c);
    return;
  }

  if (h <= 0)
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
    if (s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
      else
    {
      if (t < 0)
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
  while (y > 0);

  hline(x1 - w / 2, y1, x1 - x, c);
  hline(x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if (ey)
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

  int64_t a = w / 2;
  int64_t b = h / 2;
  int64_t a2, b2;
  int64_t s, t;

  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  ex = (w & 1);
  ey = (h & 1);

  if (w <= 1 && h <= 1)
  {
    setpixel(x1, y1, c);
    setpixel(x1 + ex, y1, c);
    setpixel(x1, y1 + ey, c);
    setpixel(x1 + ex, y1 + ey, c);
    return;
  }

  if (h <= 0)
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
    if (s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
      else
    {
      if (t < 0)
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
  while (y > 0);

  hline(x1 - w / 2, y1, x1 - x, c);
  hline(x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if (ey)
  {
    hline(x1 - w / 2, y1 + 1, x1 - x, c);
    hline(x1 + x + ex, y1 + 1, x1 + w / 2 + ex, c);
  }
}

void Map::rect(int x1, int y1, int x2, int y2, int c)
{
  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  hline(x1, y1, x2, c);

  for (int y = y1 + 1; y < y2; y++)
  {
    setpixel(x1, y, c);
    setpixel(x2, y, c);
  }

  hline(x1, y2, x2, c);
}

void Map::rectfill(int x1, int y1, int x2, int y2, int c)
{
  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  for (; y1 <= y2; y1++)
    hline(x1, y1, x2, c);
}

void Map::hline(int x1, int y, int x2, int c)
{
  if (x1 < 0)
    x1 = 0;
  if (x1 > w - 1)
    return;
  if (x2 > w - 1)
    x2 = w - 1;
  if (x2 < 0)
    return;
  if (y < 0)
    return;
  if (y > h - 1)
    return;

  unsigned char *x = row[y] + x2;
  unsigned char *z = row[y] + x1;

  do
  {
    *x = c;
    x--;
  }
  while (x >= z);
}

void Map::vline(int y1, int x, int y2, int c)
{
  if (y1 < 0)
    y1 = 0;
  if (y2 > h - 1)
    y2 = h - 1;

  unsigned char *y = row[y2] + x;

  do
  {
    *y = c;
    y -= w;
    y2--;
  }
  while (y2 >= y1);
}

void Map::polyfill(int *px, int *py, int count, int y1, int y2, int c)
{
  std::vector<int> nodex(65536);

  for (int y = y1; y <= y2; y++)
  {
    int nodes = 0;
    int j = count - 1;

    for (int i = 0; i < count; i++)
    {
      if ((py[i] < y && py[j] >= y) || (py[j] < y && py[i] >= y))
      {
        nodex[nodes++] =
          (px[i] + (float)(y - py[i]) / (py[j] - py[i]) * (px[j] - px[i]));
      }

      j = i;
    }

    std::qsort(&nodex[0], nodes, sizeof(int), node_cmp);

    for (int i = 0; i < nodes; i += 2)
    {
      for (int x = nodex[i]; x < nodex[i + 1]; x++)
        setpixel(x + 1, y, c);
    }
  }
}

// add weighted value to real pixel
void Map::blendAA(int x, int y, int c2)
{
  int c1 = *(row[y] + x);

  c1 += c2;

  if (c1 > 255)
    c1 = 255;

  *(row[y] + x) = c1;
}

// draw antialiased pixel
// each real pixel is treated like 4x4 virtual pixels
void Map::setpixelAA(int x, int y, int c)
{
  if (c == 0 ||
    x < 0 || x >= ((w - 1) << 2) ||
    y < 0 || y >= ((h - 1) << 2))
    return;

  int shift1 = 4;
  int shift2 = 20;

  if (thick_aa)
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

  blendAA(xx, yy, (a & 0x000001ff) >> shift1);
  blendAA(xx + 1, yy, (a & 0x01ff0000) >> shift2);
  blendAA(xx, yy + 1, (b & 0x000001ff) >> shift1);
  blendAA(xx + 1, yy + 1, (b & 0x01ff0000) >> shift2);
}

// draw horizontal antialised line (used by filled oval/rectangle)
void Map::hlineAA(int x1, int y, int x2, int c)
{
  for (int x = x1; x <= x2; x++)
    setpixelAA(x, y, c);
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

  dx = std::abs(dx);
  dy = std::abs(dy);

  if (dx >= dy)
  {
    dy <<= 1;
    e = dy - dx;
    dx <<= 1;

    while (x1 != x2)
    {
      setpixelAA(x1, y1, c);

      if (e >= 0)
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

    while (y1 != y2)
    {
      setpixelAA(x1, y1, c);

      if (e >= 0)
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

  int w = std::abs(x2 - x1);
  int h = std::abs(y2 - y1);
  int x, y;
  int ex, ey;

  int64_t a = w / 2;
  int64_t b = h / 2;
  int64_t a2, b2;
  int64_t s, t;

  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  ex = (w & 1);
  ey = (h & 1);

  if (w <= 1 && h <= 1)
  {
    setpixelAA(x1, y1, c);
    setpixelAA(x1 + ex, y1, c);
    setpixelAA(x1, y1 + ey, c);
    setpixelAA(x1 + ex, y1 + ey, c);
    return;
  }

  if (h <= 0)
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
    if (s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
      else
    {
      if (t < 0)
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
  while (y > 1);

  hlineAA(x1 - w / 2, y1, x1 - x, c);
  hlineAA(x1 + x + ex, y1, x1 + w / 2 + ex, c);

  if (ey)
  {
    hlineAA(x1 - w / 2, y1 + 1, x1 - x, c);
    hlineAA(x1 + x + ex, y1 + 1, x1 + w / 2 + ex, c);
  }
}

void Map::ovalfillAA(int x1, int y1, int x2, int y2, int c)
{
  x1 <<= 2;
  y1 <<= 2;
  x2 <<= 2;
  y2 <<= 2;

  int ww = std::abs(x2 - x1);
  int hh = std::abs(y2 - y1);
  int x, y;
  int ex, ey;

  int64_t a = ww / 2;
  int64_t b = hh / 2;
  int64_t a2, b2;
  int64_t s, t;

  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  ex = (ww & 1);
  ey = (hh & 1);

  if (ww <= 1 && hh <= 1)
    return;

  if (hh <= 0)
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
    if (s < 0)
    {
      s += 2 * b2 * (2 * x + 3);
      t += 4 * b2 * (x + 1);
      x++;
    }
      else
    {
      if (t < 0)
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
  while (y > 1);

  y--;

  if (ey)
    hlineAA(x1 - x, y1 + y + ey, x1 + x + ex, c);

  hlineAA(x1 - x, y1 - y, x1 + x + ex, c);
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

  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  for (; y1 <= y2; y1++)
    hlineAA(x1, y1, x2, c);
}

void Map::polyfillAA(int *px, int *py, int count, int y1, int y2, int c)
{
  for (int i = 0; i < count; i++)
  {
    px[i] <<= 2;
    py[i] <<= 2;
  }

  std::vector<int> nodex(65536);

  for (int y = (y1 << 2); y <= (y2 << 2); y++)
  {
    int j = count - 1;
    int nodes = 0;

    for (int i = 0; i < count; i++)
    {
      if ((py[i] < y && py[j] >= y) || (py[j] < y && py[i] >= y))
      {
        nodex[nodes++] =
          (px[i] + (double)(y - py[i]) / (py[j] - py[i]) * (px[j] - px[i]));
      }

      j = i;
    }

    std::qsort(&nodex[0], nodes, sizeof(int), node_cmp);

    for (int i = 0; i < nodes; i += 2)
    {
      for (int x = nodex[i]; x <= nodex[i + 1]; x++)
        setpixelAA(x, y, c);
    }
  }
}

void Map::blur(int radius)
{
  radius = (radius + 1) * 2;

  Map temp(w, h);
  temp.clear(0);

  std::vector<int> kernel(radius);
  int div = 0;
  int b = radius / 2;

  for (int x = 0; x < radius; x++)
  {
    kernel[x] = 255 * std::exp(-((double)((x - b) * (x - b)) / ((b * b) / 2)));
    div += kernel[x];
  }

  // x direction
  for (int y = 0; y < h; y++)
  {
    const int y1 = y;

    for (int x = 0; x < w; x++)
    {
      int xx = x - radius / 2;
      int val = 0;

      for (int i = 0; i < radius; i++)
      {
        if (xx >= 0 && xx < this->w)
          val += *(this->row[y1] + xx) * kernel[i];

        xx++;
      }

      val /= div;
      temp.setpixel(x, y, val);
    }
  }

  // y direction
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      int yy = y - radius / 2;
      int val = 0;

      for (int i = 0; i < radius; i++)
      {
        if (yy >= 0 && yy < h)
          val += *(temp.row[yy] + x) * kernel[i];

        yy++;
      }

      val /= div;
      this->setpixel(x, y, val);
    }
  }
}

/*
// scale with bilinear filtering
void Map::scale(Map *dest)
{
  const int sx = cl;
  const int sy = ct;
  const int sw = cw;
  const int sh = ch;
  const int dx = dest->cl;
  const int dy = dest->ct;
  const int dw = dest->cw;
  const int dh = dest->ch;

  const float ax = ((float)sw / dw);
  const float ay = ((float)sh / dh);

  if (sw < 1 || sh < 1)
    return;

  if (dw < 1 || dh < 1)
    return;

  for (int y = 0; y < dh; y++) 
  {
    int *d = dest->row[dy + y] + dx;
    const float vv = (y * ay);
    const int v1 = vv;
    const float v = vv - v1;

    if (sy + v1 >= h)
      break;

    int v2 = v1 + 1;

    if (v2 >= sh)
    {
      v2--;
    }

    int *c[4];
    c[0] = c[1] = row[sy + v1] + sx;
    c[2] = c[3] = row[sy + v2] + sx;

    for (int x = 0; x < dw; x++) 
    {
      const float uu = (x * ax);
      const int u1 = uu;
      const float u = uu - u1;

      if (sx + u1 >= w)
        break;

      int u2 = u1 + 1;

      if (u2 >= sw)
      {
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

      for (int i = 0; i < 4; i++)
      {
        rgba_type rgba = getRgba(*c[i]);
        r += (float)Gamma::fix(rgba.r) * f[i];
        g += (float)Gamma::fix(rgba.g) * f[i];
        b += (float)Gamma::fix(rgba.b) * f[i];
        a += rgba.a * f[i];
      }

      r = Gamma::unfix((int)r);
      g = Gamma::unfix((int)g);
      b = Gamma::unfix((int)b);

      *d++ = makeRgba((int)r, (int)g, (int)b, (int)a);

      c[0] -= u1;
      c[1] -= u2;
      c[2] -= u1;
      c[3] -= u2;
    }
  }
}
*/
