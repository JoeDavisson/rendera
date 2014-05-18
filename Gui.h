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

#ifndef GUI_H
#define GUI_H

#include "rendera.h"

class Gui
{
public:
  Gui();
  virtual ~Gui();

  // window
  Fl_Double_Window *window;
  Fl_Menu_Bar *menubar;

  // containers
  Fl_Group *group_top;
  Fl_Group *group_left;

  // panels
  Fl_Group *top_left;
  Fl_Group *top_right;
  Fl_Group *left_top;
  Fl_Group *left_bottom;
  Fl_Group *right;
  Fl_Group *bottom;
  Fl_Group *middle;

  // top left
  Widget *logo;

  //top right
  ToggleButton *zoom_fit;
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  Widget *display;
  ToggleButton *grid;
  Field *gridx;
  Field *gridy;

  // left top
  Widget *brush;
  Widget *size;
  Widget *stroke;
  Widget *shape;
  Widget *edge;

  // left bottom
  Button *offset;
  Button *getcolor;
  Button *crop;

  // right
  Widget *palette;
  Widget *trans;
  Widget *blend;

  // bottom
  ToggleButton *wrap;
  ToggleButton *clone;
  Widget *mirror;
  Widget *origin;
  Widget *constrain;

  // view
  View *view;

  // bitmaps
  Bitmap *bitmap_brush;
  Bitmap *bitmap_palette;
  Bitmap *bitmap_view;
};

#endif

