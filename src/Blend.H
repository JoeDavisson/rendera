/*
Copyright (c) 2021 Joe Davisson.

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

class Blend
{
public:
  enum
  {
    TRANS,
    LIGHTEN,
    DARKEN,
    COLORIZE,
    LUMINOSITY,
    ALPHA_SUB,
    SMOOTH,
    FAST,
    TRANS_ALPHA,
    TRANS_NO_ALPHA,
  };
 
  static void set(const int);
  static void target(Bitmap *, const int, const int);
  static int current(const int, const int, const int);
  static int transAlpha(const int, const int, const int);
  static int transNoAlpha(const int, const int, const int);
  static int trans(const int, const int, const int);
  static int lighten(const int, const int, const int);
  static int darken(const int, const int, const int);
  static int colorize(const int, const int, const int);
  static int luminosity(const int, const int, const int);
  static int keepLum(const int, const int);
  static int alphaSub(const int, const int, const int);
  static int smooth(const int, const int, const int);
  static int fast(const int, const int, const int);
  static int invert(const int, const int, const int);
  static void rgbToHsv(const int, const int, const int, int *, int *, int *);
  static void hsvToRgb(const int, const int, const int, int *, int *, int *);
  static void rgbToYcc(const int, const int, const int,
                       float *, float *, float *);
  static void yccToRgb(const float, const float, const float,
                       int *, int *, int *);
  static void wheelToRgb(const int, const int, const int,
                       int *, int *, int *);

private:
  static int (*current_blend)(const int, const int, const int);
  static Bitmap *bmp;
  static int xpos;
  static int ypos;

  Blend() { }
  ~Blend() { }
};

#endif

