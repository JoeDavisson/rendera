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

#include "Blend.h"

int (*Blend::current)(int, int, int) = &trans;

void Blend::set(int mode)
{
  switch(mode)
  {
    case 0:
      current = trans;
      break;
    case 1:
      current = sub;
      break;
    case 2:
      current = add;
      break;
    case 3:
      current = colorize;
      break;
  }
}

int Blend::invert(int c1, int c2, int t)
{
  return makecol(255 - getr(c1), 255 - getg(c1), 255 - getb(c1));
}

int Blend::trans(int c1, int c2, int t)
{
  int r = ((getr(c1) * t) + (getr(c2) * (255 - t))) / 255;
  int g = ((getg(c1) * t) + (getg(c2) * (255 - t))) / 255;
  int b = ((getb(c1) * t) + (getb(c2) * (255 - t))) / 255;

  return makecol(r, g, b);
}

int Blend::add(int c1, int c2, int t)
{
  t = 255 - t;

  int r = getr(c2);
  int g = getg(c2);
  int b = getb(c2);

  c2 = makecol(r, g, b);

  r = getr(c1) + (getr(c2) * t) / 255;
  g = getg(c1) + (getg(c2) * t) / 255;
  b = getb(c1) + (getb(c2) * t) / 255;

  r = MIN(r, 255);
  g = MIN(g, 255);
  b = MIN(b, 255);

  return makecol(r, g, b);
}

int Blend::sub(int c1, int c2, int t)
{
  int r = getr(c2);
  int g = getg(c2);
  int b = getb(c2);
  int h = 0, s = 0, v = 0;

  rgb_to_hsv(r, g, b, &h, &s, &v);
  h += 768;
  if(h >= 1536)
        h -= 1536;
  hsv_to_rgb(h, s, v, &r, &g, &b);
  c2 = makecol(r, g, b);

  t = 255 - t;
  r = getr(c1) - getr(c2) * t / 255;
  g = getg(c1) - getg(c2) * t / 255;
  b = getb(c1) - getb(c2) * t / 255;

  r = MAX(r, 0);
  g = MAX(g, 0);
  b = MAX(b, 0);

  return makecol(r, g, b);
}

int Blend::colorize(int c1, int c2, int t)
{
  int c3 = trans(c1, c2, t);

  return force_lum(c3, getl(c1));
}

// forces a color to a similar one with the specified luminance
int Blend::force_lum(int c, int dest_lum)
{
  int i;
  int n[3];
  int src_lum = getl(c);

  n[0] = getg(c);
  n[1] = getr(c);
  n[2] = getb(c);

  while(src_lum < dest_lum)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] < 255)
      {
        n[i]++;
        src_lum = getl(makecol(n[1], n[0], n[2]));
        if(src_lum == dest_lum)
          break;
      }
    }
  }

  while(src_lum > dest_lum)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] > 0)
      {
        n[i]--;
        src_lum = getl(makecol(n[1], n[0], n[2]));
        if(src_lum == dest_lum)
          break;
      }
    }
  }

  return makecol(n[1], n[0], n[2]);
}

// hue 0-1535
// sat 0-255
// val 0-255
void Blend::hsv_to_rgb(int h, int s, int v, int *r, int *g, int *b)
{
  if(!s)
  {
    *r = *g = *b = v;
    return;
  }

  int i = h / 256;
  int f = (h & 255);

  int x = (v * (255 - s)) / 255;
  int y = (v * ((65025 - s * f) / 255)) / 255;
  int z = (v * ((65025 - s * (255 - f)) / 255)) / 255;

  switch(i)
  {
    case 6:
    case 0:
      *r = v;
      *g = z;
      *b = x;
      break;
    case 1:
      *r = y;
      *g = v;
      *b = x;
      break;
    case 2:
      *r = x;
      *g = v;
      *b = z;
      break;
    case 3:
      *r = x;
      *g = y;
      *b = v;
      break;
    case 4:
      *r = z;
      *g = x;
      *b = v;
      break;
    case 5:
      *r = v;
      *g = x;
      *b = y;
      break;
  }
}

// hue 0-1535
// sat 0-255
// val 0-255
void Blend::rgb_to_hsv(int r, int g, int b, int *h, int *s, int *v)
{
  int max = MAX(r, MAX(g, b));
  int min = MIN(r, MIN(g, b));
  int delta = max - min;

  *v = max;

  if(max)
    *s = (delta * 255) / max;
  else
    *s = 0;

  if(!*s)
  {
    *h = 0;
  }
  else
  {
    if(r == max)
      *h = ((g - b) * 255) / delta;
    else if(g == max)
      *h = 512 + ((b - r) * 255) / delta;
    else if(b == max)
      *h = 1024 + ((r - g) * 255) / delta;

    if(*h < 0)
      *h += 1536;
  }
}

