/*
Copyright (c) 2024 Joe Davisson.

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
#include "Gamma.H"
#include "Inline.H"
#include "Palette.H"

int (*Blend::current_blend)(const int, const int, const int) = &Blend::trans;
Bitmap *Blend::bmp;
int Blend::xpos;
int Blend::ypos;

// sets the blending mode for future operations
void Blend::set(const int mode)
{
  switch (mode)
  {
    case TRANS:
      current_blend = trans;
      break;
    case NON_LINEAR:
      current_blend = nonLinear;
      break;
    case LIGHTEN:
      current_blend = lighten;
      break;
    case DARKEN:
      current_blend = darken;
      break;
    case COLORIZE:
      current_blend = colorize;
      break;
    case LUMINOSITY:
      current_blend = luminosity;
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
    case FAST:
      current_blend = fast;
      break;
    case TRANS_ALPHA:
      current_blend = transAlpha;
      break;
    case TRANS_NO_ALPHA:
      current_blend = transNoAlpha;
      break;
    default:
      current_blend = trans;
      break;
  }
}

// sets the target coordinates used by some blending modes
void Blend::target(Bitmap *b, const int x, const int y)
{
  bmp = b;
  xpos = x;
  ypos = y;
}

// returns the current blending mode
int Blend::current(const int c1, const int c2, const int t)
{
  return (*current_blend)(c1, c2, t);
}

int Blend::transAlpha(const int c1, const int c2, const int t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  return makeRgba(rgba1.r, rgba1.g, rgba1.b,
                  rgba2.a + (t * (rgba1.a - rgba2.a)) / 255);
}

int Blend::transNoAlpha(const int c1, const int c2, const int t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  return makeRgba(rgba2.r + (t * (rgba1.r - rgba2.r)) / 255,
                  rgba2.g + (t * (rgba1.g - rgba2.g)) / 255,
                  rgba2.b + (t * (rgba1.b - rgba2.b)) / 255,
                  rgba1.a);
}

int Blend::trans(const int c1, const int c2, const int t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  return makeRgba(rgba2.r + (t * (rgba1.r - rgba2.r)) / 255,
                  rgba2.g + (t * (rgba1.g - rgba2.g)) / 255,
                  rgba2.b + (t * (rgba1.b - rgba2.b)) / 255,
                  rgba2.a + (t * (rgba1.a - rgba2.a)) / 255);
}

int Blend::nonLinear(const int c1, const int c2, const int t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  const int r1 = Gamma::fix(rgba1.r);
  const int g1 = Gamma::fix(rgba1.g);
  const int b1 = Gamma::fix(rgba1.b);
  const int a1 = rgba1.a;

  const int r2 = Gamma::fix(rgba2.r);
  const int g2 = Gamma::fix(rgba2.g);
  const int b2 = Gamma::fix(rgba2.b);
  const int a2 = rgba2.a;

  return makeRgba(Gamma::unfix(r2 + (t * (r1 - r2)) / 255),
                  Gamma::unfix(g2 + (t * (g1 - g2)) / 255),
                  Gamma::unfix(b2 + (t * (b1 - b2)) / 255),
                  a2 + (t * (a1 - a2)) / 255);
}

int Blend::lighten(const int c1, const int c2, const int t)
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

int Blend::darken(const int c1, const int c2, const int t)
{
  const rgba_type rgba1 = getRgba(c1);
  const rgba_type rgba2 = getRgba(c2);

  int r = 0, g = 0, b = 0;
  int h = 0, s = 0, v = 0;

  rgbToHsv(rgba2.r, rgba2.g, rgba2.b, &h, &s, &v);
  h += 768;

  if (h >= 1536)
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

int Blend::colorize(const int c1, const int c2, const int t)
{
  int c3 = transNoAlpha(c1, c2, t);

  return keepLum(c3, getl(c1));
}

int Blend::luminosity(const int c1, const int c2, const int t)
{
  return trans(c1, keepLum(c1, getl(c2)), t);
}

int Blend::keepLum(const int c, const int lum)
{
  int r, g, b;
  float y, cb, cr;
  const rgba_type rgba1 = getRgba(c);

  rgbToYcc(rgba1.r, rgba1.g, rgba1.b, &y, &cb, &cr);
  yccToRgb(lum, cb, cr, &r, &g, &b);

  const rgba_type rgba = getRgba(makeRgba(r, g, b, rgba1.a));
  int n[3];

  n[1] = rgba.r;
  n[0] = rgba.g;
  n[2] = rgba.b;

  // iterate to exact luminosity
  int src = getlUnpacked(n[1], n[0], n[2]);
  int count = 0;

  while (src < lum && count < 1000)
  {
    for (int i = 0; i < 3; i++)
    {
      if (n[i] < 255)
      {
        n[i]++;
        src = getlUnpacked(n[1], n[0], n[2]);

        if (src >= lum)
          break;
      }
    }

    count++;
  }

  count = 0;

  while (src > lum && count < 1000)
  {
    for (int i = 0; i < 3; i++)
    {
      if (n[i] > 0)
      {
        n[i]--;
        src = getlUnpacked(n[1], n[0], n[2]);

        if (src <= lum)
          break;
      }
    }

    count++;
  }

  return makeRgba(n[1], n[0], n[2], rgba.a);
}

int Blend::alphaAdd(const int c1, const int, const int t)
{
  const rgba_type rgba = getRgba(c1);
  return makeRgba(rgba.r, rgba.g, rgba.b, 255 - ((255 - rgba.a) * t) / 255);
}

int Blend::alphaSub(const int c1, const int, const int t)
{
  const rgba_type rgba = getRgba(c1);
  return makeRgba(rgba.r, rgba.g, rgba.b, (rgba.a * t) / 255);
}

int Blend::smooth(const int c1, const int, const int t)
{
  int r = 0, g = 0, b = 0, a = 0;

  const int matrix[3][3] =
  {
    {  1,  2,  1 },
    {  2,  4,  2 },
    {  1,  2,  1 }
  };

  for (int j = 0; j < 3; j++)
  {
    for (int i = 0; i < 3; i++)
    {
      const rgba_type rgba = getRgba(bmp->getpixel(xpos + i - 1, ypos + j - 1));

      r += rgba.r * matrix[i][j];
      g += rgba.g * matrix[i][j];
      b += rgba.b * matrix[i][j];
      a += rgba.a * matrix[i][j];
    }
  }

  r /= 16;
  g /= 16;
  b /= 16;
  a /= 16;

  return Blend::trans(c1, makeRgba(r, g, b, a), t);
}

int Blend::fast(const int c1, const int c2, const int t)
{
  const int rb =
    (((((c1 & 0xff00ff) - (c2 & 0xff00ff)) * t) >> 8) + c2) & 0xff00ff;
  const int g = (((((c1 & 0xff00) - (c2 & 0xff00)) * t) >> 8) + c2) & 0xff00;

  return rb | g | 0xff000000;
}

int Blend::invert(const int c1, const int, const int)
{
  return makeRgba(255 - getr(c1), 255 - getg(c1), 255 - getb(c1), geta(c1));
}

// integer-based RGB<->HSV conversions use the following ranges:
// hue 0-1535
// sat 0-255
// val 0-255
void Blend::rgbToHsv(const int r, const int g, const int b, int *h, int *s, int *v)
{
  int max = std::max(r, std::max(g, b));
  int min = std::min(r, std::min(g, b));
  int delta = max - min ;

  *v = max;

  if (max)
    *s = (delta * 255) / max;
  else
    *s = 0;

  if (*s == 0)
  {
    *h = 0;
  }
    else
  {
    if (r == max)
      *h = ((g - b) * 255) / delta;
    else if (g == max)
      *h = 512 + ((b - r) * 255) / delta;
    else if (b == max)
      *h = 1024 + ((r - g) * 255) / delta;

    if (*h < 0)
      *h += 1536;
  }
}

// integer-based RGB<->HSV conversions use the following ranges:
// hue 0-1535
// sat 0-255
// val 0-255
void Blend::hsvToRgb(const int h, const int s, const int v, int *r, int *g, int *b)
{
  if (s == 0)
  {
    *r = *g = *b = v;
    return;
  }

  int i = h / 256;
  int f = (h & 255);

  int x = (v * (255 - s)) / 255;
  int y = (v * ((65025 - s * f) / 255)) / 255;
  int z = (v * ((65025 - s * (255 - f)) / 255)) / 255;

  switch (i)
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

// from JFIF specification: https://www.w3.org/Graphics/JPEG/jfif3.pdf
void Blend::rgbToYcc(const int r, const int g, const int b, float *y, float *cb, float *cr)
{
  *y = r * 0.299 + g * 0.587 + b * 0.114;
  *cb = r * -0.1687 + g * -0.3313 + b * 0.500 + 128;
  *cr = r * 0.500 + g * -0.4187 + b * -0.0813 + 128;
}

void Blend::yccToRgb(const float y, const float cb, const float cr, int *r, int *g, int *b)
{
  *r = y + (cr - 128) * 1.402;
  *g = y + (cb - 128) * -0.34414 + (cr - 128) * -0.71414;
  *b = y + (cb - 128) * 1.772;

  *r = clamp(*r, 255);
  *g = clamp(*g, 255);
  *b = clamp(*b, 255);
}

// same as RGB<->HSV, except hues are arranged like an artist's color wheel
// hue 0-1535
// sat 0-255
// val 0-255
void Blend::wheelToRgb(int h, int s, int v, int *r, int *g, int *b)
{
  switch (h / 256)
  {
    case 0:
      h /= 2;
      break;
    case 1:
      h = 128 + (h - 256) / 2;
      break;
    case 2:
      h -= 256;
      break;
    case 3:
      h = 512 + (h - 768) * 2;
      break;
    case 4:
      break;
    case 5:
      break;
  }

  Blend::hsvToRgb(h, s, v, r, g, b);
}

