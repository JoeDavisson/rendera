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

  void set_menu_item(const char *);
  void clear_menu_item(const char *);

  // window
  Fl_Double_Window *window;
  //Fl_Group *group_main;
  Fl_Menu_Bar *menubar;

  // containers
  Fl_Group *group_top;
  Fl_Group *group_left;

  // panels
  Fl_Group *top_left;
  Fl_Group *top_right;
  Fl_Group *tools;
  Fl_Group *paint;
  Fl_Group *crop;
  Fl_Group *getcolor;
  Fl_Group *offset;
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
  Field *zoom;
  ToggleButton *grid;
  Field *gridx;
  Field *gridy;

  // tools
  Widget *tool;

  // options
  Widget *paint_brush;
  Widget *paint_size;
  Widget *paint_stroke;
  Widget *paint_shape;
  Widget *paint_edge;
  Widget *paint_smooth;

  Field *crop_x;
  Field *crop_y;
  Field *crop_w;
  Field *crop_h;
  Fl_Button *crop_do;

  // right
  Widget *pal_preview;
  Widget *palette;
  Widget *hue;
  Widget *satval;
//  Widget *val;
//  Widget *color;
  Widget *trans;
//  Widget *blend;
  Fl_Choice *blend;

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

