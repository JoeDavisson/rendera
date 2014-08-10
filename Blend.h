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

#include "rendera.h"

class Blend
{
public:
  static void set(int);
  static int invert(int, int, int);
  static int trans(int, int, int);
  static int trans_all(int, int, int);
  static int add(int, int, int);
  static int sub(int, int, int);
  static int colorize(int, int, int);
  static int force_lum(int, int);
  static int alpha_add(int, int, int);
  static int alpha_sub(int, int, int);
  static int dither_random(int, int, int);
  static int dither_ordered(int, int, int);
  static void hsv_to_rgb(int, int, int, int *, int *, int *);
  static void rgb_to_hsv(int, int, int, int *, int *, int *);

  // pointer to current blender
  static int (*current)(int, int, int);

  // xy locations for dithering
  static int ditherx, dithery;
};

#endif

