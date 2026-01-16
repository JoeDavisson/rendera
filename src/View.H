/*
Copyright (c) 2025 Joe Davisson.

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

#include <FL/Fl_Widget.H>

class Bitmap;
class Tool;

class View : public Fl_Widget
{
public:
  enum
  {
    ASPECT_NORMAL,
    ASPECT_WIDE,
    ASPECT_TALL
  };

  View(Fl_Group *, int, int, int, int, const char *);
  ~View();

  int handle(int);
  void resize(int, int, int, int);

  void changeAspect(int);
  void drawMain(bool);
  void drawGrid();
  void changeCursor();
  void drawCustomCursor();
  void drawCloneCursor();
  void zoomIn(int, int);
  void zoomOut(int, int);
  void zoomOne();
  void scroll(int, int);
  void clipOrigin();
  void saveCoords();

  Fl_Group *group;
  Bitmap *backbuf;
  int mousex, mousey;
  int last_mousex, last_mousey;
  int imgx, imgy;
  int oldimgx, oldimgy;
  int last_ox, last_oy;
  int ox, oy;
  float zoom;
  int aspect;
  bool panning;
  bool resizing;
  bool grid, gridsnap;
  int gridx, gridy;
  int px, py, pw, ph;
  int bx, by, bw, bh;
  bool rendering;
  bool mouse_timer_ready;
  bool mouse_in_viewport;
  bool bgr_order;
  int button;
  int button1, button2, button3;
  bool dclick;
  bool shift;
  bool ctrl;
  bool alt;

protected:
  void draw();
};

#endif

