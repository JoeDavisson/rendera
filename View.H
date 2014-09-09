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

class Bitmap;
class Tool;

#include "rendera.h"

class View : public Fl_Widget
{
public:
  View(Fl_Group *, int, int, int, int, const char *);
  virtual ~View();

  virtual int handle(int);
  virtual void resize(int, int, int, int);
  virtual void redraw();
  void drawMove();
  void drawMain(int);
  void drawGrid();
  void drawCloneCursor();
  void beginMove();
  void move();
  void zoomIn(int, int);
  void zoomOut(int, int);
  void zoomFit(int);
  void zoomOne();
  void scroll(int, int);

  Fl_Group *group;
  Bitmap *backbuf;
  Tool *tool;
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
  int lastbx, lastby, lastbw, lastbh;
  float aspect, winaspect;
  int bgr_order;
  int button;
  int button1, button2, button3;
  int dclick;
  int shift;
protected:
  virtual void draw();
};

#endif

