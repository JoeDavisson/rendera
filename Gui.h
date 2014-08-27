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

class Widget;
class Button;
class ToggleButton;
class Field;
class View;

#include "rendera.h"

namespace Gui
{
  void init();
  void setMenuItem(const char *);
  void clearMenuItem(const char *);

  void updateColor(int);
  void checkPalette(Widget *, void *);
  void checkZoomIn(Button *, void *);
  void checkZoomOut(Button *, void *);
  void checkZoomFit(ToggleButton *, void *);
  void checkZoomOne(Button *, void *);
  void checkZoom();
  void checkGrid(ToggleButton *, void *);
  void checkGridX(Field *, void *);
  void checkGridY(Field *, void *);
  void checkPaintSize(Widget *, void *);
  void checkPaintShape(Widget *, void *);
  void checkPaintStroke(Widget *, void *);
  void checkPaintEdge(Widget *, void *);
  void checkPaintSmooth(Widget *, void *);
  void checkTool(Widget *, void *);
  void checkColor(Widget *, void *);
  void checkHue(Widget *, void *);
  void checkSatVal(Widget *, void *);
  void checkTrans(Widget *, void *);
  void checkBlend(Widget *, void *);
  void checkWrap(Widget *, void *);
  void checkClone(Widget *, void *);
  void checkMirror(Widget *, void *);
  void checkOrigin(Widget *, void *);
  void checkConstrain(Widget *, void *);
  void checkCropDo();
  void checkCropValues();
  void checkRGBA();
  void checkIndexed();

  // window
  extern Fl_Double_Window *window;
  //Fl_Group *group_main;
  extern Fl_Menu_Bar *menubar;

  // containers
  extern Fl_Group *group_top;
  extern Fl_Group *group_left;

  // panels
  extern Fl_Group *top_left;
  extern Fl_Group *top_right;
  extern Fl_Group *tools;
  extern Fl_Group *paint;
  extern Fl_Group *crop;
  extern Fl_Group *getcolor;
  extern Fl_Group *offset;
  extern Fl_Group *right;
  extern Fl_Group *bottom;
  extern Fl_Group *middle;

  // top left
  extern Widget *logo;

  //top right
  extern ToggleButton *zoom_fit;
  extern Button *zoom_one;
  extern Button *zoom_in;
  extern Button *zoom_out;
  extern Field *zoom;
  extern ToggleButton *grid;
  extern Field *gridx;
  extern Field *gridy;

  // tools
  extern Widget *tool;

  // options
  extern Widget *paint_brush;
  extern Widget *paint_size;
  extern Widget *paint_stroke;
  extern Widget *paint_shape;
  extern Widget *paint_edge;
  extern Widget *paint_smooth;

  extern Field *crop_x;
  extern Field *crop_y;
  extern Field *crop_w;
  extern Field *crop_h;
  extern Fl_Button *crop_do;

  // right
  extern Widget *pal_preview;
  extern Widget *palette;
  extern Widget *hue;
  extern Widget *satval;
  extern Widget *trans;
  extern Fl_Choice *blend;

  // bottom
  extern ToggleButton *wrap;
  extern ToggleButton *clone;
  extern Widget *mirror;
  extern Widget *origin;
  extern Widget *constrain;

  // view
  extern View *view;
}

#endif

