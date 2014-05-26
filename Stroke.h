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
  void draw_brush(Brush *, Map *, int, int, int);
  void draw_brush_line(Brush *, Map *, int, int, int, int, int);
  void draw_brush_rect(Brush *, Map *, int, int, int, int, int);
  void draw_brush_oval(Brush *, Map *, int, int, int, int, int);

  void begin(Brush *, Map *, int, int, int, int, float);
  void draw(Brush *, Map *, int, int, int, int, float);
  void end(Brush *, Map *, int, int, int, int, float);
  void polyline(Map *, int, int, int, int, float);
  void preview(Map *, Bitmap *,int, int, float);
  void render(Brush *, Map *);
  void render_normal(Brush *, Map *);
  void render_smooth(Brush *, Map *);
  int render_callback(Brush *, Map *, int, int, float);
  int render_callback_normal(Brush *, Map *, int, int, float);
  int render_callback_smooth(Brush *, Map *, int, int, float);

  int x1, y1, x2, y2;
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
  int render_pos, render_end, render_count;
  float soft_trans, soft_step;
  Map *map;
};

#endif

