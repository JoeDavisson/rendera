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

#ifndef DIALOG_H
#define DIALOG_H

#include "rendera.h"

class Dialog
{
public:
  Dialog();
  virtual ~Dialog();

  Fl_Double_Window *progress;
  Fl_Progress *progress_bar;

  Fl_Double_Window *about;
  Widget *about_logo;
  Fl_Button *about_ok;

  Fl_Double_Window *new_image;
  Field *new_image_width;
  Field *new_image_height;
  Fl_Button *new_image_ok;
  Fl_Button *new_image_cancel;

  Fl_Double_Window *create_palette;
  Field *create_palette_colors;
  Fl_Button *create_palette_ok;
  Fl_Button *create_palette_cancel;

  Fl_Double_Window *editor;
  Widget *editor_r;
  Widget *editor_g;
  Widget *editor_b;
  Widget *editor_h;
  Widget *editor_s;
  Widget *editor_v;
  Fl_Button *editor_insert;
  Fl_Button *editor_delete;
  Fl_Button *editor_replace;
  Fl_Button *editor_undo;
  Fl_Button *editor_rgb_ramp;
  Fl_Button *editor_hsv_ramp;
  Widget *editor_palette;
  Widget *editor_color;
  Fl_Button *editor_done;
};

#endif

