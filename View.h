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
  void draw_move();
  void draw_main();
  void begin_move();
  void move();
  void zoom_in(int, int);
  void zoom_out(int, int);
  void zoom_fit(int);
  void zoom_one();

  Fl_Group *group;
  Fl_RGB_Image *image;
  Bitmap *backbuf;
  int mousex, mousey;
  int imgx, imgy;
  int ox, oy;
  float zoom;
  int fit;
  int moving;
  int px, py, pw, ph;
  int bx, by, bw, bh;
  float aspect, winaspect;
protected:
  virtual void draw();
};

#endif

