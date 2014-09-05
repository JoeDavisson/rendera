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

#include "rendera.h"

namespace Blend
{
  enum
  {
    TRANS,
    DARKEN,
    LIGHTEN,
    COLORIZE,
    ALPHA_ADD,
    ALPHA_SUB,
    SMOOTH,
    INVERT
  };
 
  void set(int);
  void target(Bitmap *, int, int);
  int current(int, int, int);
  int invert(int, int, int);
  int trans(int, int, int);
  int transAll(int, int, int);
  int darken(int, int, int);
  int lighten(int, int, int);
  int colorize(int, int, int);
  int keepLum(int, int);
  int alphaAdd(int, int, int);
  int alphaSub(int, int, int);
  int smooth(int, int, int);
  void rgbToHsv(int, int, int, int *, int *, int *);
  void hsvToRgb(int, int, int, int *, int *, int *);
}

#endif

