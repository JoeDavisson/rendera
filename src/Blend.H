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

#ifndef BLEND_H
#define BLEND_H

class Bitmap;
class Palette;

namespace Blend
{
  enum
  {
    TRANS,
    TRANS_NO_ALPHA,
    DARKEN,
    LIGHTEN,
    COLORIZE,
    COLORIZE_HIGHLIGHTS,
    COLORIZE_MIDTONES,
    COLORIZE_SHADOWS,
    ALPHA_ADD,
    ALPHA_SUB,
    SMOOTH,
    SMOOTH_COLOR,
    SHARPEN,
    PALETTE_COLORS
  };
 
  void set(const int &);
  void target(Bitmap *, Palette *, const int &, const int &);
  int current(const int &, const int &, const int &);
  int invert(const int &, const int &, const int &);
  int transNoAlpha(const int &, const int &, const int &);
  int trans(const int &, const int &, const int &);
  int darken(const int &, const int &, const int &);
  int lighten(const int &, const int &, const int &);
  int colorize(const int &, const int &, const int &);
  int colorizeHighlights(const int &, const int &, const int &);
  int colorizeMidtones(const int &, const int &, const int &);
  int colorizeShadows(const int &, const int &, const int &);
  int keepLum(const int &, const int &);
  int alphaAdd(const int &, const int &, const int &);
  int alphaSub(const int &, const int &, const int &);
  int smooth(const int &, const int &, const int &);
  int smoothColor(const int &, const int &, const int &);
  int sharpen(const int &, const int &, const int &);
  int paletteColors(const int &, const int &, const int &);
  int invert(const int &, const int &, const int &);
  void rgbToHsv(const int &, const int &, const int &, int *, int *, int *);
  void hsvToRgb(const int &, const int &, const int &, int *, int *, int *);
}

#endif

