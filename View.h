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

#ifndef VIEW_H
#define VIEW_H

#include "rendera.h"

class View : public Fl_Widget
{
public:
  View(Fl_Group *, int, int, int, int, const char *);
  virtual ~View();

  virtual int handle(int);
  virtual void resize(int, int, int, int);
  void brush_push();
  void brush_drag();
  void brush_release();
  void brush_move();
  void offset_push();
  void offset_drag();
  void offset_release();
  void crop_push();
  void crop_drag();
  void crop_release();
  void getcolor_push();
  void draw_move();
  void draw_main(int);
  void draw_grid();
  void begin_move();
  void move();
  void zoom_in(int, int);
  void zoom_out(int, int);
  void zoom_fit(int);
  void zoom_one();
  void scroll(int, int);

  Fl_Group *group;
  Fl_RGB_Image *image_part, *image_full;
  Bitmap *backbuf;
  unsigned char *alphadata;
  int mousex, mousey;
  int imgx, imgy;
  int oldimgx, oldimgy;
  int ox, oy;
  float zoom;
  int fit;
  int moving;
  int grid, gridx, gridy;
  int px, py, pw, ph;
  int bx, by, bw, bh;
  float aspect, winaspect;
  Stroke *stroke;
  int tool;
  int tool_started;
  int crop_resize_started;
  int crop_side;
  int beginx, beginy;
  int lastx, lasty;

  int button;
  int button1, button2, button3;
  int dclick;
  int shift;
protected:
  virtual void draw();
};

#endif

