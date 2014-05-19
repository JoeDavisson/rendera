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

/*
static int is_edge(Map *map, int x, int y)
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
*/

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

  return *(row[y] + x);
}

void Map::line(int x1, int y1, int x2, int y2, int c)
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
  int w = ABS(x2 - x1);
  int h = ABS(y2 - y1);
  int x, y;
  int ex, ey;

  long long int a = w / 2;
  long long int b = h / 2;
  long long int a2, b2;
  long long int s, t;

  if(x1 > x2)
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

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
  int w = ABS(x2 - x1);
  int h = ABS(y2 - y1);
  int x, y;
  int ex, ey;

  long long int a = w / 2;
  long long int b = h / 2;
  long long int a2, b2;
  long long int s, t;

  if(x1 > x2)
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

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
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

  if(x1 < 0)
    x1 = 0;
  if(x2 > w - 1)
    x2 = w - 1;
  if(y1 < 0)
    y1 = 0;
  if(y2 > h - 1)
    y2 = h - 1;

  unsigned char *y = row[y2] + x2;
  unsigned char *z = row[y1] + x1;
  int d = x2 - x1;
  int e = w - d;

  hline(x1, y2, x2, c);

  do
  {
    *y = c;
    y -= d;
    *y = c;
    y -= e;
  }
  while(y > z);

  hline(x1, y1, x2, c);
}

void Map::rectfill(int x1, int y1, int x2, int y2, int c)
{
  if(x1 > x2)
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

  for(; y1 <= y2; y1++)
    hline(x1, y1, x2, c);
}

void Map::hline(int x1, int y, int x2, int c)
{
  if(x1 < 0)
    x1 = 0;
  if(x1 > w - 1)
    return;
  if(x2 >  w - 1)
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

