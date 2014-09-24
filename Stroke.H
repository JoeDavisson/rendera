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

class Bitmap;

#include "Rendera.H"

class Stroke
{
public:
  Stroke();
  virtual ~Stroke();

  void clip();
  void sizeLinear(int, int, int, int);
  void makeBlitRect(int, int, int, int, int, int, int, float);
  void size(int, int, int, int);
  void drawBrush(int, int, int);
  void drawBrushLine(int, int, int, int, int);
  void drawBrushRect(int, int, int, int, int);
  void drawBrushOval(int, int, int, int, int);
  void begin(int, int, int, int, float);
  void draw(int, int, int, int, float);
  void end();
  void polyline(int, int, int, int, float);
  void preview(Bitmap *,int, int, float);

  int x1, y1, x2, y2;
  int origin, constrain;
  int blitx, blity, blitw, blith;
  int beginx, beginy;
  int lastx, lasty;
  int oldx, oldy;
  int active;
  int type;
  int *polycachex;
  int *polycachey;
  int *edgecachex;
  int *edgecachey;
  int polycount;
};

#endif

