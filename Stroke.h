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

#ifndef STROKE_H
#define STROKE_H

#include "rendera.h"

class Stroke
{
public:
  Stroke();
  virtual ~Stroke();

  void clip();
  void make_blitrect(int, int, int, int, int, int, int, float);

  void begin(Brush *, Map *, int, int, int, int, float, int);
  void draw(Brush *, Map *, int, int, int, int, float, int);
  void end(Brush *, Map *, int, int, int, int, float, int);
  void preview(Map *, Bitmap *,int, int, float);
  void apply(Map *);

  void freehand();
  void region();
  void line();
  void polygon();
  void rect();
  void rectfill();
  void oval();
  void ovalfill();

  int x1, y1, x2, y2;
  int blitx, blity, blitw, blith;
  int beginx, beginy;
  int lastx, lasty;
  int active;
  Map *map;
};

#endif

