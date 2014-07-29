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

Dialog::Dialog()
{
  // about
  about = new Fl_Double_Window(336, 112, "About");
  about_logo = new Widget(about, 8, 8, 320, 64, "Logo", "data/logo_large.png", 0, 0, 0);
  about_ok = new Fl_Button(336 / 2 - 32, 80, 64, 24, "OK");
  about_ok->callback((Fl_Callback *)hide_about);
  about->set_modal();
  about->end(); 

  // new image
  new_image = new Fl_Double_Window(256, 184, "New Image");
  new_image_width = new Field(new_image, 120, 32, 72, 24, "Width:", 0);
  new_image_height = new Field(new_image, 120, 72, 72, 24, "Height:", 0);
  new_image_width->maximum_size(8);
  new_image_height->maximum_size(8);
  new_image_width->value("640");
  new_image_height->value("480");
  new Separator(new_image, 16, 128, 226, 2, "");
  new_image_ok = new Fl_Button(96, 144, 64, 24, "OK");
  new_image_ok->callback((Fl_Callback *)hide_new_image);
  new_image_cancel = new Fl_Button(176, 144, 64, 24, "Cancel");
  new_image_cancel->callback((Fl_Callback *)cancel_new_image);
  new_image->set_modal();
  new_image->end(); 

  // create palette from image
  create_palette = new Fl_Double_Window(256, 144, "Create Palette From Image");
  create_palette_colors = new Field(create_palette, 120, 32, 72, 24, "Colors:", 0);
  new_image_width->value("256");
  new Separator(new_image, 16, 88, 226, 2, "");
  create_palette_ok = new Fl_Button(96, 104, 64, 24, "OK");
  create_palette_ok->callback((Fl_Callback *)hide_create_palette);
  create_palette_cancel = new Fl_Button(176, 104, 64, 24, "Cancel");
  create_palette_cancel->callback((Fl_Callback *)cancel_create_palette);
  create_palette->set_modal();
  create_palette->end(); 
}

Dialog::~Dialog()
{
}

