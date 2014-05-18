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

#ifndef BITMAP_H
#define BITMAP_H

#include "rendera.h"

class Bitmap
{
public:
  Bitmap(int, int);
  virtual ~Bitmap();

  int w, h;
  int cl, cr, ct, cb, cw, ch;
  int *data;
  int **row;

  void clear(int);
  void hline(int, int, int, int, int);
  void rect(int, int, int, int, int, int);
  void setpixel_solid(int, int, int ,int);
  void setpixel_wrap(int, int, int ,int);
  void setpixel_clone(int, int, int ,int);
  void setpixel_wrap_clone(int, int, int ,int);
  int getpixel(int, int);
  void clip(int *, int *, int *, int *);
  void set_clip(int, int, int, int);
  void blit(Bitmap *, int, int, int, int, int, int);
  void point_stretch(Bitmap *, int, int, int, int, int, int, int, int, int, int);
  void integer_stretch(Bitmap *, int, int, int, int, int, int, int, int, int, int);
  void stretch_line(Bitmap *, int, int, int, int, int, int);
  void fast_stretch(Bitmap *, int, int, int, int, int, int, int, int);
};

#endif
