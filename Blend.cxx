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

int (*Blend::current)(int, int, int) = &trans;
int Blend::xpos;
int Blend::ypos;
Bitmap *Blend::bmp;

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
    case 4:
      current = alpha_add;
      break;
    case 5:
      current = alpha_sub;
      break;
    case 6:
      current = smooth;
      break;
    default:
      current = trans;
      break;
  }
}

int Blend::invert(int c1, int /* c2 */, int /* t */)
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

int Blend::trans_all(int c1, int c2, int t)
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

  return makecola(r, g, b, geta(c1));
}

int Blend::colorize(int c1, int c2, int t)
{
  int c3 = trans(c1, c2, t);

  return force_lum(c3, getl(c1));
}

// forces a color to a similar one with the specified luminance
int Blend::force_lum(int c, int dest_lum)
{
/*
  int y, u, v;
  int r, g, b;

  rgb_to_yuv(getr(c), getg(c), getb(c), &y, &u, &v);
  yuv_to_rgb(dest_lum, u, v, &r, &g, &b);

  return makecola(r, g, b, geta(c));
*/
  int i;
  int n[3];
  int src_lum = getl(c);
  int a = geta(c);

  n[0] = getr(c);
  n[1] = getg(c);
  n[2] = getb(c);

  while(src_lum < dest_lum)
  {
    for(i = 0; i < 3; i++)
    {
      if(n[i] < 255)
      {
        n[i]++;
        src_lum = getl(makecola(n[0], n[1], n[2], a));
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
        src_lum = getl(makecola(n[0], n[1], n[2], a));
        if(src_lum == dest_lum)
          break;
      }
    }
  }

  return makecola(n[0], n[1], n[2], a);
}

int Blend::alpha_add(int c1, int /* c2 */, int t)
{
  return makecola(getr(c1), getg(c1), getb(c1), (geta(c1) * t) / 255);
}

int Blend::alpha_sub(int c1, int /* c2 */, int t)
{
  return makecola(getr(c1), getg(c1), getb(c1), 255 - ((255 - geta(c1)) * t) / 255);
}

int Blend::smooth(int c1, int /* c2 */, int t)
{
  int x = xpos;
  int y = ypos;
  int c[9];

  c[0] = Blend::bmp->getpixel(x - 1, y - 1);
  c[1] = Blend::bmp->getpixel(x, y - 1);
  c[2] = Blend::bmp->getpixel(x + 1, y - 1);
  c[3] = Blend::bmp->getpixel(x - 1, y);
  c[4] = Blend::bmp->getpixel(x, y);
  c[5] = Blend::bmp->getpixel(x + 1, y);
  c[6] = Blend::bmp->getpixel(x - 1, y + 1);
  c[7] = Blend::bmp->getpixel(x, y + 1);
  c[8] = Blend::bmp->getpixel(x + 1, y + 1);

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

  return Blend::trans_all(c1, makecola(r, g, b, a), t);
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

void Blend::yuv_to_rgb(int y, int u, int v, int *r, int *g, int *b)
{
  int c = y - 16;
  int d = u - 128;
  int e = v - 128;

  *r = ((298 * c           + 409 * e + 128) >> 8);
  *g = ((298 * c - 100 * d - 208 * e + 128) >> 8);
  *b = ((298 * c + 516 * d           + 128) >> 8);

  if(*r < 0)
    *r = 0;
  if(*r > 255)
    *r = 255;
  if(*g < 0)
    *g = 0;
  if(*g > 255)
    *g = 255;
  if(*b < 0)
    *b = 0;
  if(*b > 255)
    *b = 255;
}

void Blend::rgb_to_yuv(int r, int g, int b, int *y, int *u, int *v)
{
  *y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
  *u = ( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
  *v = ( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
}

