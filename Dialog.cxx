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

// callbacks are in dialog_callback.cxx
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

  // palette editor
  editor = new Fl_Double_Window(576, 360, "Palette Editor");
  editor_r = new Widget(editor, 16, 16, 24, 256, "Red", 24, 1, (Fl_Callback *)do_editor_get_rgb);
  editor_g = new Widget(editor, 56, 16, 24, 256, "Green", 24, 1, (Fl_Callback *)do_editor_get_rgb);
  editor_b = new Widget(editor, 96, 16, 24, 256, "Blue", 24, 1, (Fl_Callback *)do_editor_get_rgb);
  editor_h = new Widget(editor, 136, 16, 24, 256, "Hue", 24, 1, (Fl_Callback *)do_editor_get_hsv);
  editor_s = new Widget(editor, 176, 16, 24, 256, "Saturation", 24, 1, (Fl_Callback *)do_editor_get_hsv);
  editor_v = new Widget(editor, 216, 16, 24, 256, "Value", 24, 1, (Fl_Callback *)do_editor_get_hsv);
  editor_insert = new Fl_Button(256, 16, 96, 24, "Insert");
  editor_delete = new Fl_Button(256, 56, 96, 24, "Delete");
  editor_replace = new Fl_Button(256, 96, 96, 24, "Replace");
  editor_begin_ramp = new Fl_Button(256, 168, 96, 24, "Begin Ramp");
  editor_rgb_ramp = new Fl_Button(256, 208, 96, 24, "RGB Ramp");
  editor_hsv_ramp = new Fl_Button(256, 248, 96, 24, "HSV Ramp");
  editor_palette = new Widget(editor, 368, 16, 192, 192, "Palette", 24, 24, (Fl_Callback *)do_editor_palette);
  editor_color = new Widget(editor, 368, 224, 192, 48, "Color", 0, 0, 0);
  new Separator(editor, 16, 302, 546, 2, "");
  editor_done = new Fl_Button(464, 320, 96, 24, "Done");
  editor_done->callback((Fl_Callback *)hide_editor);
  editor->set_modal();
  editor->end(); 
}

Dialog::~Dialog()
{
}

