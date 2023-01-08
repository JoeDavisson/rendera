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

#ifndef STROKE_H
#define STROKE_H

#include <vector>

class Bitmap;
class View;

class Stroke
{
public:
  enum
  {
    FREEHAND,
    REGION,
    LINE,
    POLYGON,
    RECT,
    FILLED_RECT,
    OVAL,
    FILLED_OVAL
  };

  Stroke();
  ~Stroke();

  void clip();
  void sizeLinear(int, int, int, int);
  void makeBlitRect(int, int, int, int, int, int, int, float);
  void size(int, int, int, int);
  void drawBrush(int, int, int);
  void drawBrushLine(int, int, int, int, int);
  void drawBrushRect(int, int, int, int, int);
  void drawBrushOval(int, int, int, int, int);
  void drawBrushAA(int, int, int);
  void drawBrushLineAA(int, int, int, int, int);
  void drawBrushRectAA(int, int, int, int, int);
  void drawBrushOvalAA(int, int, int, int, int);
  void begin(int, int, int, int, float);
  void draw(int, int, int, int, float);
  void end(int, int);
  void polyLine(int, int, int, int, float);
  void previewPaint(View *);
  void previewSelection(View *);

  int x1, y1, x2, y2;
  int origin, constrain;
  int origin_always, constrain_always;
  int blitx, blity, blitw, blith;
  int beginx, beginy;
  int lastx, lasty;
  int oldx, oldy;
  int type;
  int *poly_x;
  int *poly_y;
  int *edge_x;
  int *edge_y;
  int poly_count;
};

#endif

