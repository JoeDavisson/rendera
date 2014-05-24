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

#include "rendera.h"

Fl_Menu_Item menuitems[] =
{
  { "&File", 0, 0, 0, FL_SUBMENU },
    { "&New", 0, 0, 0 },
    { 0 },
  { "&Edit", 0, 0, 0, FL_SUBMENU },
    { "&Copy", 0, 0, 0 },
    { "&Paste", 0, 0, 0 },
    { 0 },
  { "&Help", 0, 0, 0, FL_SUBMENU },
    { "&About", 0, 0, 0 },
    { 0 },
  { 0 }
};

Gui::Gui()
{
  int x1, y1;

  // window
  window = new Fl_Double_Window(800, 600, "Rendera");
  menubar = new Fl_Menu_Bar(0, 0, window->w(), 24);
  menubar->menu(menuitems);

  // top_left
  top_left = new Fl_Group(0, menubar->h(), 112, 40);
  top_left->box(FL_UP_FRAME);
  logo = new Widget(top_left, 8, 8, 96, 24, "", "data/logo.png", 0, 0);
  top_left->resizable(0);
  top_left->end();

  // top right
  top_right = new Fl_Group(top_left->w(), menubar->h(), window->w() - top_left->w(), 40);
  top_right->box(FL_UP_FRAME);
  x1 = 8;
  zoom_fit = new ToggleButton(top_right, x1, 8, 24, 24, "Fit", "data/zoom_fit.png");
  zoom_fit->callback((Fl_Callback *)check_zoom_fit, &zoom_fit->var);
  x1 += 24 + 8;
  zoom_one = new Button(top_right, x1, 8, 24, 24, "Actual Size", "data/zoom_one.png");
  zoom_one->callback((Fl_Callback *)check_zoom_one, &zoom_one->var);
  x1 += 24 + 8;
  zoom_in = new Button(top_right, x1, 8, 24, 24, "Zoom In", "data/zoom_in.png");
  zoom_in->callback((Fl_Callback *)check_zoom_in, &zoom_in->var);
  x1 += 24 + 8;
  zoom_out = new Button(top_right, x1, 8, 24, 24, "Zoom Out", "data/zoom_out.png");
  zoom_out->callback((Fl_Callback *)check_zoom_out, &zoom_out->var);
  x1 += 24;
  x1 += 6;
  new Separator(top_right, x1, 2, 2, 36, "");
  x1 += 8;
  grid = new ToggleButton(top_right, x1, 8, 24, 24, "Show Grid", "data/grid.png");
  grid->callback((Fl_Callback *)check_grid, &grid->var);
  x1 += 24 + 28 + 8;
  gridx = new Field(top_right, x1, 8, 32, 24, "Grid X:");
  gridx->callback((Fl_Callback *)check_gridx, &gridx->var);
  gridx->value("8");
  x1 += 32 + 28 + 8;
  gridy = new Field(top_right, x1, 8, 32, 24, "Grid Y:");
  gridy->callback((Fl_Callback *)check_gridy, &gridy->var);
  gridy->value("8");
  top_right->resizable(0);
  top_right->end();

  // bottom
  bottom = new Fl_Group(112, window->h() - 40, window->w() - 224, 40);
  bottom->box(FL_UP_FRAME);
  x1 = 8;
  wrap = new ToggleButton(bottom, x1, 8, 24, 24, "Wrap", "data/wrap.png");
  x1 += 24 + 8;
  clone = new ToggleButton(bottom, x1, 8, 24, 24, "Clone", "data/clone.png");
  x1 += 24 + 8;
  mirror = new Widget(bottom, x1, 8, 96, 24, "Clone Mirroring", "data/mirror.png", 24, 24);
  x1 += 96 + 8;
  origin = new Widget(bottom, x1, 8, 48, 24, "Origin", "data/origin.png", 24, 24);
  x1 += 48 + 8;
  constrain = new Widget(bottom, x1, 8, 48, 24, "Constrain", "data/constrain.png", 24, 24);
  bottom->resizable(0);
  bottom->end();

  // left top
  left_top = new Fl_Group(0, top_right->h() + menubar->h(), 112, 264 + 12);
  left_top->label("Brush");
  left_top->labelsize(12);
  left_top->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  left_top->box(FL_UP_FRAME);
  y1 = 20;
  brush = new Widget(left_top, 8, y1, 96, 96, "Brush Preview", 0, 0);
  brush->bitmap->clear(makecol(255, 255, 255));
  y1 += 96 + 8;
  size = new Widget(left_top, 8, y1, 96, 24, "Size", "data/size.png", 8, 24);
  size->callback((Fl_Callback *)check_size, &size->var);
  y1 += 24 + 8;
  stroke = new Widget(left_top, 8, y1, 96, 48, "Stroke", "data/stroke.png", 24, 24);
  y1 += 48 + 8;
  shape = new Widget(left_top, 8, y1, 96, 24, "Shape", "data/shape.png", 24, 24);
  y1 += 24 + 8;
  edge = new Widget(left_top, 8, y1, 96, 24, "Soft Edge", "data/edge.png", 8, 24);
  left_top->resizable(0);
  left_top->end();

  // left bottom
  left_bottom = new Fl_Group(0, top_right->h() + menubar->h() + left_top->h(), 112, window->h() - (menubar->h() + top_right->h() + left_top->h()));
  left_bottom->label("Tools");
  left_bottom->labelsize(12);
  left_bottom->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  left_bottom->box(FL_UP_FRAME);
  y1 = 20;
  offset = new Button(left_bottom, 8, y1, 96, 24, "Offset", "data/offset.png");
  y1 += 24 + 8;
  getcolor = new Button(left_bottom, 8, y1, 96, 24, "Get Color", "data/getcolor.png");
  y1 += 24 + 8;
  crop = new Button(left_bottom, 8, y1, 96, 24, "Crop", "data/crop.png");
  left_bottom->resizable(0);
  left_bottom->end();

  // right
  right = new Fl_Group(window->w() - 112, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  right->label("Colors");
  right->labelsize(12);
  right->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  right->box(FL_UP_FRAME);
  y1 = 20;
  palette = new Widget(right, 8, y1, 96, 96, "Color Palette", 16, 16);
  palette->callback((Fl_Callback *)check_palette, &palette->var);
  palette->bitmap->clear(makecol(255, 255, 255));
  y1 += 96 + 8;
  trans = new Widget(right, 8, y1, 96, 24, "Transparency", "data/transparency.png", 8, 24);
  y1 += 24 + 8;
  blend = new Widget(right, 8, y1, 96, 24, "Blending Mode", "data/blend.png", 24, 24);
  right->resizable(0);
  right->end();

  // middle
  middle = new Fl_Group(112, top_right->h() + menubar->h(), window->w() - 224, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  middle->box(FL_FLAT_BOX);
  view = new View(middle, 0, 0, middle->w(), middle->h(), "View");
  middle->resizable(view);
  middle->end();

  // container for top panels
  group_top = new Fl_Group(0, menubar->h(), window->w() - top_left->w(), 40);
  group_top->add(top_left);
  group_top->add(top_right);
  group_top->resizable(top_right);
  group_top->end();

  // container for left panels
  group_left = new Fl_Group(0, top_right->h() + menubar->h(), 112, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  group_left->add(left_top);
  group_left->add(left_bottom);
  group_left->resizable(left_bottom);
  group_left->end();

  window->size_range(640, 480, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();
  window->show();
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

}

Gui::~Gui()
{
}

