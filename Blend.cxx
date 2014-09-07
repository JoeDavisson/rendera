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
#include "Bitmap.h"

namespace
{
  int (*current_blend)(int, int, int) = &Blend::trans;
  Bitmap *bmp;
  int xpos, ypos;
}

void Blend::set(int mode)
{
  switch(mode)
  {
    case TRANS:
      current_blend = trans;
      break;
    case DARKEN:
      current_blend = darken;
      break;
    case LIGHTEN:
      current_blend = lighten;
      break;
    case COLORIZE:
      current_blend = colorize;
      break;
    case ALPHA_ADD:
      current_blend = alphaAdd;
      break;
    case ALPHA_SUB:
      current_blend = alphaSub;
      break;
    case SMOOTH:
      current_blend = smooth;
      break;
    case SMOOTH_COLOR:
      current_blend = smoothColor;
      break;
    case INVERT:
      current_blend = invert;
      break;
    default:
      current_blend = trans;
      break;
  }
}

void Blend::target(Bitmap *b, int x, int y)
{
  bmp = b;
  xpos = x;
  ypos = y;
}

int Blend::current(int c1, int c2, int t)
{
  return (*current_blend)(c1, c2, t);
}

int Blend::invert(int c1, int, int)
{
  return makecol(255 - getr(c1), 255 - getg(c1), 255 - getb(c1));
}

int Blend::trans(int c1, int c2, int t)
{
  int r = getr(c2) + (t * (getr(c1) - getr(c2))) / 255;
  int g = getg(c2) + (t * (getg(c1) - getg(c2))) / 255;
  int b = getb(c2) + (t * (getb(c1) - getb(c2))) / 255;

  return makecola(r, g, b, geta(c1));
}

int Blend::transAll(int c1, int c2, int t)
{
  int r = getr(c2) + (t * (getr(c1) - getr(c2))) / 255;
  int g = getg(c2) + (t * (getg(c1) - getg(c2))) / 255;
  int b = getb(c2) + (t * (getb(c1) - getb(c2))) / 255;
  int a = geta(c2) + (t * (geta(c1) - geta(c2))) / 255;

  return makecola(r, g, b, a);
}

int Blend::darken(int c1, int c2, int t)
{
  int r = getr(c2);
  int g = getg(c2);
  int b = getb(c2);
  int h = 0, s = 0, v = 0;

  rgbToHsv(r, g, b, &h, &s, &v);
  h += 768;
  if(h >= 1536)
        h -= 1536;
  hsvToRgb(h, s, v, &r, &g, &b);
  c2 = makecol(r, g, b);

  t = 255 - t;
  r = getr(c1) - getr(c2) * t / 255;
  g = getg(c1) - getg(c2) * t / 255;
  b = getb(c1) - getb(c2) * t / 255;

  r = MAX(r, 0);
  g = MAX(g, 0);
  b = MAX(b, 0);

  return makecola(r, g, b, geta(c1));
}

int Blend::lighten(int c1, int c2, int t)
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

  return makecola(r, g, b, geta(c1));
}

int Blend::colorize(int c1, int c2, int t)
{
  int c3 = trans(c1, c2, t);

  return keepLum(c3, getl(c1));
}

// forces a color to a similar one with the specified luminance,
// so an image may be re-colored indefinately with no artifacts
int Blend::keepLum(int c, int dest)
{
  int src = getl(c);
  int n[3];

  // these have to be in order of importance in the luminance calc: G, R, B
  n[1] = getr(c);
  n[0] = getg(c);
  n[2] = getb(c);

  // iterate to find similar color with same luminance
  int i;

  while(src < dest)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] < 255)
      {
        n[i]++;
        src = getl(makecol(n[1], n[0], n[2]));
        if(src >= dest)
          break;
      }
    }
  }

  while(src > dest)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] > 0)
      {
        n[i]--;
        src = getl(makecol(n[1], n[0], n[2]));
        if(src <= dest)
          break;
      }
    }
  }

  return makecola(n[1], n[0], n[2], geta(c));
}

int Blend::alphaAdd(int c1, int, int t)
{
  return makecola(getr(c1), getg(c1), getb(c1), (geta(c1) * t) / 255);
}

int Blend::alphaSub(int c1, int, int t)
{
  return makecola(getr(c1), getg(c1), getb(c1), 255 - ((255 - geta(c1)) * t) / 255);
}

int Blend::smooth(int c1, int, int t)
{
  static int matrix[9] = { 1, 2, 1, 2, 3, 2, 1, 2, 1 };
  int x = xpos;
  int y = ypos;
  int c[9];

  // need to use getpixel here because of clipping
  c[0] = bmp->getpixel(x - 1, y - 1);
  c[1] = bmp->getpixel(x, y - 1);
  c[2] = bmp->getpixel(x + 1, y - 1);
  c[3] = bmp->getpixel(x - 1, y);
  c[4] = bmp->getpixel(x, y);
  c[5] = bmp->getpixel(x + 1, y);
  c[6] = bmp->getpixel(x - 1, y + 1);
  c[7] = bmp->getpixel(x, y + 1);
  c[8] = bmp->getpixel(x + 1, y + 1);

  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;

  int i;

  for(i = 0; i < 9; i++)
  {
    r += getr(c[i]) * matrix[i];
    g += getg(c[i]) * matrix[i];
    b += getb(c[i]) * matrix[i];
    a += geta(c[i]) * matrix[i];
  }

  r /= 15;
  g /= 15;
  b /= 15;
  a /= 15;

  return Blend::transAll(c1, makecola(r, g, b, a), t);
}

int Blend::smoothColor(int c1, int, int t)
{
  static int matrix[9] = { 1, 2, 1, 2, 3, 2, 1, 2, 1 };
  int x = xpos;
  int y = ypos;
  int c[9];

  // need to use getpixel here because of clipping
  c[0] = bmp->getpixel(x - 1, y - 1);
  c[1] = bmp->getpixel(x, y - 1);
  c[2] = bmp->getpixel(x + 1, y - 1);
  c[3] = bmp->getpixel(x - 1, y);
  c[4] = bmp->getpixel(x, y);
  c[5] = bmp->getpixel(x + 1, y);
  c[6] = bmp->getpixel(x - 1, y + 1);
  c[7] = bmp->getpixel(x, y + 1);
  c[8] = bmp->getpixel(x + 1, y + 1);

  int r = 0;
  int g = 0;
  int b = 0;

  int i;

  for(i = 0; i < 9; i++)
  {
    r += getr(c[i]) * matrix[i];
    g += getg(c[i]) * matrix[i];
    b += getb(c[i]) * matrix[i];
  }

  r /= 15;
  g /= 15;
  b /= 15;

  int c3 = Blend::trans(c1, makecol(r, g, b), t);
  return keepLum(c3, getl(c1));
}

// RGB<->HSV conversions use the following ranges:
// hue 0-1535
// sat 0-255
// val 0-255
void Blend::rgbToHsv(int r, int g, int b, int *h, int *s, int *v)
{
  int max = MAX(r, MAX(g, b));
  int min = MIN(r, MIN(g, b));
  int delta = max - min;

  *v = max;

  if(max)
    *s = (delta * 255) / max;
  else
    *s = 0;

  if(*s == 0)
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

void Blend::hsvToRgb(int h, int s, int v, int *r, int *g, int *b)
{
  if(s == 0)
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

