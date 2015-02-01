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

#include "Bitmap.H"
#include "Blend.H"
#include "Inline.H"
#include "Math.H"
#include "Palette.H"

namespace
{
  int (*current_blend)(const int &, const int &, const int &) = &Blend::trans;
  Bitmap *bmp;
  Palette *pal;
  int xpos, ypos;
}

void Blend::set(const int &mode)
{
  switch(mode)
  {
    case TRANS:
      current_blend = trans;
      break;
    case TRANS_NO_ALPHA:
      current_blend = transNoAlpha;
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
    case COLORIZE_HIGHLIGHTS:
      current_blend = colorizeHighlights;
      break;
    case COLORIZE_MIDTONES:
      current_blend = colorizeMidtones;
      break;
    case COLORIZE_SHADOWS:
      current_blend = colorizeShadows;
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
    case SHARPEN:
      current_blend = sharpen;
      break;
    case PALETTE_COLORS:
      current_blend = paletteColors;
      break;
    default:
      current_blend = trans;
      break;
  }
}

void Blend::target(Bitmap *b, Palette *p, const int &x, const int &y)
{
  bmp = b;
  pal = p;
  xpos = x;
  ypos = y;
}

int Blend::current(const int &c1, const int &c2, const int &t)
{
  return (*current_blend)(c1, c2, t);
}

int Blend::transNoAlpha(const int &c1, const int &c2, const int &t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  return makeRgba(rgba2.r + (t * (rgba1.r - rgba2.r)) / 255,
                  rgba2.g + (t * (rgba1.g - rgba2.g)) / 255,
                  rgba2.b + (t * (rgba1.b - rgba2.b)) / 255,
                  rgba1.a);
}

int Blend::trans(const int &c1, const int &c2, const int &t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  return makeRgba(rgba2.r + (t * (rgba1.r - rgba2.r)) / 255,
                  rgba2.g + (t * (rgba1.g - rgba2.g)) / 255,
                  rgba2.b + (t * (rgba1.b - rgba2.b)) / 255,
                  rgba2.a + (t * (rgba1.a - rgba2.a)) / 255);
}

int Blend::darken(const int &c1, const int &c2, const int &t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  int r, g, b;
  int h = 0, s = 0, v = 0;

  rgbToHsv(rgba2.r, rgba2.g, rgba2.b, &h, &s, &v);
  h += 768;
  if(h >= 1536)
        h -= 1536;
  hsvToRgb(h, s, v, &r, &g, &b);

  r = rgba1.r - r * (255 - t) / 255;
  g = rgba1.g - g * (255 - t) / 255;
  b = rgba1.b - b * (255 - t) / 255;

  r = std::max(r, 0);
  g = std::max(g, 0);
  b = std::max(b, 0);

  return makeRgba(r, g, b, rgba1.a);
}

int Blend::lighten(const int &c1, const int &c2, const int &t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  int r = rgba1.r + (rgba2.r * (255 - t)) / 255;
  int g = rgba1.g + (rgba2.g * (255 - t)) / 255;
  int b = rgba1.b + (rgba2.b * (255 - t)) / 255;

  r = std::min(r, 255);
  g = std::min(g, 255);
  b = std::min(b, 255);

  return makeRgba(r, g, b, rgba1.a);
}

int Blend::colorize(const int &c1, const int &c2, const int &t)
{
  int c3 = transNoAlpha(c1, c2, t);

  return keepLum(c3, getl(c1));
}

int Blend::colorizeHighlights(const int &c1, const int &c2, const int &t)
{
  const int dist = Math::abs(255 - getl(c1));
  int c3 = transNoAlpha(c1, c2, std::min(t + dist * 2, 255));

  return keepLum(c3, getl(c1));
}

int Blend::colorizeMidtones(const int &c1, const int &c2, const int &t)
{
  const int dist = Math::abs(128 - getl(c1));
  int c3 = transNoAlpha(c1, c2, std::min(t + dist * 2, 255));

  return keepLum(c3, getl(c1));
}

int Blend::colorizeShadows(const int &c1, const int &c2, const int &t)
{
  const int dist = Math::abs(0 - getl(c1));
  int c3 = transNoAlpha(c1, c2, std::min(t + dist * 2, 255));

  return keepLum(c3, getl(c1));
}

int Blend::keepLum(const int &c, const int &dest)
{
  // these have to be in order of importance in the luminance calc: G, R, B
  const rgba_type rgba = getRgba(c);
  int n[3];
  n[1] = rgba.r;
  n[0] = rgba.g;
  n[2] = rgba.b;

  // iterate to find similar color with same luminance
  int src = getlUnpacked(n[1], n[0], n[2]);

  while(src < dest)
  {
    for(int i = 0; i < 3; i++)
    {
      if(n[i] < 255)
      {
        n[i]++;
        src = getlUnpacked(n[1], n[0], n[2]);
        if(src >= dest)
          break;
      }
    }
  }

  while(src > dest)
  {
    for(int i = 0; i < 3; i++)
    {
      if(n[i] > 0)
      {
        n[i]--;
        src = getlUnpacked(n[1], n[0], n[2]);
        if(src <= dest)
          break;
      }
    }
  }

  return makeRgba(n[1], n[0], n[2], rgba.a);
}

int Blend::alphaSub(const int &c1, const int &, const int &t)
{
  const rgba_type rgba = getRgba(c1);
  return makeRgba(rgba.r, rgba.g, rgba.b, (rgba.a * t) / 255);
}

int Blend::alphaAdd(const int &c1, const int &, const int &t)
{
  const rgba_type rgba = getRgba(c1);
  return makeRgba(rgba.r, rgba.g, rgba.b, 255 - ((255 - rgba.a) * t) / 255);
}

int Blend::smooth(const int &c1, const int &, const int &t)
{
  const int matrix[3][3] =
  {
    { 1, 2, 1 },
    { 2, 3, 2 },
    { 1, 2, 1 }
  };

  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;

  for(int j = 0; j < 3; j++)
  {
    for(int i = 0; i < 3; i++)
    {
      const rgba_type rgba = getRgba(bmp->getpixel(xpos + i - 1, ypos + j - 1));
      r += rgba.r * matrix[i][j];
      g += rgba.g * matrix[i][j];
      b += rgba.b * matrix[i][j];
      a += rgba.a * matrix[i][j];
    }
  }

  r /= 15;
  g /= 15;
  b /= 15;
  a /= 15;

  return Blend::trans(c1, makeRgba(r, g, b, a), t);
}

int Blend::smoothColor(const int &c1, const int &, const int &t)
{
  const int matrix[3][3] =
  {
    { 1, 2, 1 },
    { 2, 3, 2 },
    { 1, 2, 1 }
  };

  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;

  for(int j = 0; j < 3; j++)
  {
    for(int i = 0; i < 3; i++)
    {
      const rgba_type rgba = getRgba(bmp->getpixel(xpos + i - 1, ypos + j - 1));
      r += rgba.r * matrix[i][j];
      g += rgba.g * matrix[i][j];
      b += rgba.b * matrix[i][j];
      a += rgba.a * matrix[i][j];
    }
  }

  r /= 15;
  g /= 15;
  b /= 15;
  a /= 15;

  int c3 = Blend::trans(c1, makeRgba(r, g, b, a), t);
  return keepLum(c3, getl(c1));
}

int Blend::sharpen(const int &c1, const int &, const int &t)
{
  const int matrix[3][3] =
  {
    { -1, -1, -1 },
    { -1,  9, -1 },
    { -1, -1, -1 }
  };

  int lum = 0;

  for(int j = 0; j < 3; j++)
  {
    for(int i = 0; i < 3; i++)
    {
      lum += getl(bmp->getpixel(xpos + i - 1, ypos + j - 1)) * matrix[i][j];
    }
  }

  lum = std::min(std::max(lum, 0), 255);

  return Blend::trans(c1, keepLum(c1, lum), 255 - (255 - t) / 16);
}

int Blend::paletteColors(const int &c1, const int &c2, const int &t)
{
  int c = pal->data[pal->lookup(transNoAlpha(c1, c2, t))];
  c &= 0xFFFFFF;
  c |= c1 & 0xFF000000;

  return c;
}

int Blend::invert(const int &c1, const int &, const int &)
{
  return makeRgba(255 - getr(c1), 255 - getg(c1), 255 - getb(c1), geta(c1));
}

// RGB<->HSV conversions use the following ranges:
// hue 0-1535
// sat 0-255
// val 0-255
void Blend::rgbToHsv(const int &r, const int &g, const int &b, int *h, int *s, int *v)
{
  int max = std::max(r, std::max(g, b));
  int min = std::min(r, std::min(g, b));
  int delta = max - min ;

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

void Blend::hsvToRgb(const int &h, const int &s, const int &v, int *r, int *g, int *b)
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

