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

  Fl_Double_Window *about;
  Fl_Double_Window *new_image;
  Fl_Double_Window *create_palette;
  Fl_Double_Window *save_bmp;
  Fl_Double_Window *save_tga;
  Fl_Double_Window *save_jpg;
  Fl_Double_Window *save_png;
  Fl_Double_Window *filter;

  Field *new_image_width;
  Field *new_image_height;
  Fl_Button *new_image_ok;
  Fl_Button *new_image_cancel;

  Field *create_palette_colors;
  Fl_Button *create_palette_ok;
  Fl_Button *create_palette_cancel;

  Widget *about_logo;
  Fl_Button *about_ok;
};

#endif

