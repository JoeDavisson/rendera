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

class Palette;

#include "Rendera.H"

class Bitmap
{
public:
  Bitmap(int, int);
  Bitmap(int, int, int, int, int);
  virtual ~Bitmap();

  int x, y, w, h;
  int cl, cr, ct, cb, cw, ch;
  int overscroll;
  int *data;
  int **row;

  void clear(int);
  void hline(int, int, int, int, int);
  void vline(int, int, int, int, int);
  void line(int, int, int, int, int, int);
  void rect(int, int, int, int, int, int);
  void rectfill(int, int, int, int, int, int);
  void xorLine(int, int, int, int);
  void xorHline(int, int, int);
  void xorRect(int, int, int, int);
  void xorRectfill(int, int, int, int);
  void setpixel(int, int, int ,int);
  void setpixelSolid(int, int, int ,int);
  void setpixelWrap(int, int, int ,int);
  void setpixelClone(int, int, int ,int);
  void setpixelWrapClone(int, int, int ,int);
  int getpixel(int, int);
  void clip(int *, int *, int *, int *);
  void setClip(int, int, int, int);
  void blit(Bitmap *, int, int, int, int, int, int);
  void pointStretch(Bitmap *, int, int, int, int, int, int, int, int, int, int, int);
  void fastStretch(Bitmap *, int, int, int, int, int, int, int, int, int);

  static Bitmap *main;
  static Bitmap *preview;
  static Bitmap *clone_buffer;
  static Bitmap *offset_buffer;
  static int wrap;
  static int clone;
  static int clone_moved;
  static int clone_x;
  static int clone_y;
  static int clone_dx;
  static int clone_dy;
  static int clone_mirror;
};

#endif
