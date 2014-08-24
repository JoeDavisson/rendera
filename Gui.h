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
  static void init();
  static void setMenuItem(const char *);
  static void clearMenuItem(const char *);

  static void updateColor(int);
  static void checkPalette(Widget *, void *);
  static void checkZoomIn(Button *, void *);
  static void checkZoomOut(Button *, void *);
  static void checkZoomFit(ToggleButton *, void *);
  static void checkZoomOne(Button *, void *);
  static void checkZoom();
  static void checkGrid(ToggleButton *, void *);
  static void checkGridX(Field *, void *);
  static void checkGridY(Field *, void *);
  static void checkPaintSize(Widget *, void *);
  static void checkPaintShape(Widget *, void *);
  static void checkPaintStroke(Widget *, void *);
  static void checkPaintEdge(Widget *, void *);
  static void checkPaintSmooth(Widget *, void *);
  static void checkTool(Widget *, void *);
  static void checkColor(Widget *, void *);
  static void checkHue(Widget *, void *);
  static void checkSatVal(Widget *, void *);
  static void checkTrans(Widget *, void *);
  static void checkBlend(Widget *, void *);
  static void checkWrap(Widget *, void *);
  static void checkClone(Widget *, void *);
  static void checkMirror(Widget *, void *);
  static void checkOrigin(Widget *, void *);
  static void checkConstrain(Widget *, void *);
  static void checkCropDo();
  static void checkCropValues();
  static void checkRGBA();
  static void checkIndexed();

  // window
  static Fl_Double_Window *window;
  //Fl_Group *group_main;
  static Fl_Menu_Bar *menubar;

  // containers
  static Fl_Group *group_top;
  static Fl_Group *group_left;

  // panels
  static Fl_Group *top_left;
  static Fl_Group *top_right;
  static Fl_Group *tools;
  static Fl_Group *paint;
  static Fl_Group *crop;
  static Fl_Group *getcolor;
  static Fl_Group *offset;
  static Fl_Group *right;
  static Fl_Group *bottom;
  static Fl_Group *middle;

  // top left
  static Widget *logo;

  //top right
  static ToggleButton *zoom_fit;
  static Button *zoom_one;
  static Button *zoom_in;
  static Button *zoom_out;
  static Field *zoom;
  static ToggleButton *grid;
  static Field *gridx;
  static Field *gridy;

  // tools
  static Widget *tool;

  // options
  static Widget *paint_brush;
  static Widget *paint_size;
  static Widget *paint_stroke;
  static Widget *paint_shape;
  static Widget *paint_edge;
  static Widget *paint_smooth;

  static Field *crop_x;
  static Field *crop_y;
  static Field *crop_w;
  static Field *crop_h;
  static Fl_Button *crop_do;

  // right
  static Widget *pal_preview;
  static Widget *palette;
  static Widget *hue;
  static Widget *satval;
  static Widget *trans;
  static Fl_Choice *blend;

  // bottom
  static ToggleButton *wrap;
  static ToggleButton *clone;
  static Widget *mirror;
  static Widget *origin;
  static Widget *constrain;

  // view
  static View *view;
};

#endif

