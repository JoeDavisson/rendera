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
    case 0:
      current_blend = trans;
      break;
    case 1:
      current_blend = sub;
      break;
    case 2:
      current_blend = add;
      break;
    case 3:
      current_blend = colorize;
      break;
    case 4:
      current_blend = alphaAdd;
      break;
    case 5:
      current_blend = alphaSub;
      break;
    case 6:
      current_blend = smooth;
      break;
    default:
      current_blend = trans;
      break;
  }
}

void Blend::setTarget(Bitmap *b, int x, int y)
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

  return makecola(r, g, b, geta(c1));
}

int Blend::sub(int c1, int c2, int t)
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

  // preconversion to save iterations
  int y, u, v;

  rgbToYuv(n[1], n[0], n[2], &y, &u, &v);
  yuvToRgb(dest, u, v, &n[1], &n[0], &n[2]);

  // iterate to find similar color with same luminance
  // count is there to prevent an infinite loop, although it
  // shouldn't ever happen
  int i;
  int count = 0;

  while(src < dest)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] < 255)
      {
        n[i]++;
        src = getl(makecol(n[1], n[0], n[2]));
        if(src == dest)
          break;
      }
    }

    count++;
    if(count > 255)
      break;
  }

  count = 0;

  while(src > dest)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] > 0)
      {
        n[i]--;
        src = getl(makecol(n[1], n[0], n[2]));
        if(src == dest)
          break;
      }
    }

    count++;
    if(count > 255)
      break;
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

// cheap and fake "gaussian blur" :)
int Blend::smooth(int c1, int, int t)
{
  int x = xpos;
  int y = ypos;
  int c[9];

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

  r += getr(c[0]) * 1;
  r += getr(c[1]) * 2;
  r += getr(c[2]) * 1;
  r += getr(c[3]) * 2;
  r += getr(c[4]) * 3;
  r += getr(c[5]) * 2;
  r += getr(c[6]) * 1;
  r += getr(c[7]) * 2;
  r += getr(c[8]) * 1;
  r /= 15;

  g += getg(c[0]) * 1;
  g += getg(c[1]) * 2;
  g += getg(c[2]) * 1;
  g += getg(c[3]) * 2;
  g += getg(c[4]) * 3;
  g += getg(c[5]) * 2;
  g += getg(c[6]) * 1;
  g += getg(c[7]) * 2;
  g += getg(c[8]) * 1;
  g /= 15;

  b += getb(c[0]) * 1;
  b += getb(c[1]) * 2;
  b += getb(c[2]) * 1;
  b += getb(c[3]) * 2;
  b += getb(c[4]) * 3;
  b += getb(c[5]) * 2;
  b += getb(c[6]) * 1;
  b += getb(c[7]) * 2;
  b += getb(c[8]) * 1;
  b /= 15;

  a += geta(c[0]) * 1;
  a += geta(c[1]) * 2;
  a += geta(c[2]) * 1;
  a += geta(c[3]) * 2;
  a += geta(c[4]) * 3;
  a += geta(c[5]) * 2;
  a += geta(c[6]) * 1;
  a += geta(c[7]) * 2;
  a += geta(c[8]) * 1;
  a /= 15;

  return Blend::transAll(c1, makecola(r, g, b, a), t);
}

// RGB<->HSV conversions use the following ranges:
// hue 0-1535
// sat 0-255
// val 0-255
// the additional hue resolution ensures that no information is lost so an
// image's hue may be rotated indefinately with no loss of information

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

// JPEG formula for YUV (really YCbCr..)
void Blend::rgbToYuv(int r, int g, int b, int *y, int *u, int *v)
{
  *y = 0   + .299 * r + .587 * g + .114 * b;
  *u = 128 - .168 * r - .331 * g +   .5 * b;
  *v = 128 +   .5 * r - .418 * g - .081 * b;
}

void Blend::yuvToRgb(int y, int u, int v, int *r, int *g, int *b)
{
  u -= 128;
  v -= 128;

  *r = y             + 1.402 * v;
  *g = y -  .344 * u -  .714 * v;
  *b = y + 1.772 * u;

  *r = MAX(0, MIN(*r, 255));
  *g = MAX(0, MIN(*g, 255));
  *b = MAX(0, MIN(*b, 255));
}

