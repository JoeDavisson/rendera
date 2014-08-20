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
  // JPEG quality
  jpeg_quality = new Fl_Double_Window(200, 80, "JPEG Quality");
  jpeg_quality->callback(jpeg_quality_close_callback);
  jpeg_quality_amount = new Field(jpeg_quality, 80, 8, 72, 24, "Quality:", 0);
  jpeg_quality_amount->value("95");
  new Separator(jpeg_quality, 2, 40, 196, 2, "");
  jpeg_quality_ok = new Fl_Button(128, 48, 64, 24, "OK");
  // no callback for ok button, see show_jpeg_quality called from save.cxx
  jpeg_quality->set_modal();
  jpeg_quality->end();

  // progress
  progress = new Fl_Double_Window(272, 80, "Progress");
  progress_bar = new Fl_Progress(8, 8, 256, 64);
  progress_bar->minimum(0);
  progress_bar->maximum(100);
  progress_bar->color(0);
  progress_bar->selection_color(0x88CC8800);
  progress_bar->labelcolor(0xFFFFFF00);
  progress->set_modal();
  progress->end();

  // about
  about = new Fl_Double_Window(336, 112, "About");
  about_logo = new Widget(about, 8, 8, 320, 64, "Logo", "data/logo_large.png", 0, 0, 0);
  about_ok = new Fl_Button(336 / 2 - 32, 80, 64, 24, "OK");
  about_ok->callback((Fl_Callback *)hide_about);
  about->set_modal();
  about->end(); 

  // new image
  new_image = new Fl_Double_Window(200, 112, "New Image");
  new_image_width = new Field(new_image, 88, 8, 72, 24, "Width:", 0);
  new_image_height = new Field(new_image, 88, 40, 72, 24, "Height:", 0);
  new_image_width->maximum_size(8);
  new_image_height->maximum_size(8);
  new_image_width->value("640");
  new_image_height->value("480");
  new Separator(new_image, 2, 72, 196, 2, "");
  new_image_ok = new Fl_Button(56, 80, 64, 24, "OK");
  new_image_ok->callback((Fl_Callback *)hide_new_image);
  new_image_cancel = new Fl_Button(128, 80, 64, 24, "Cancel");
  new_image_cancel->callback((Fl_Callback *)cancel_new_image);
  new_image->set_modal();
  new_image->end(); 

  // create palette from image
  create_palette = new Fl_Double_Window(200, 80, "Create Palette");
  create_palette_colors = new Field(create_palette, 80, 8, 72, 24, "Colors:", 0);
  new_image_width->value("256");
  new Separator(new_image, 2, 40, 196, 2, "");
  create_palette_ok = new Fl_Button(56, 48, 64, 24, "OK");
  create_palette_ok->callback((Fl_Callback *)hide_create_palette);
  create_palette_cancel = new Fl_Button(128, 48, 64, 24, "Cancel");
  create_palette_cancel->callback((Fl_Callback *)cancel_create_palette);
  create_palette->set_modal();
  create_palette->end(); 

  // palette editor
  editor = new Fl_Double_Window(608, 312, "Palette Editor");
  editor_h = new Widget(editor, 8, 8, 24, 256, "Hue", 24, 1, (Fl_Callback *)do_editor_get_hsv);
  editor_sv = new Widget(editor, 40, 8, 256, 256, "Saturation/Value", 1, 1, (Fl_Callback *)do_editor_get_hsv);
  editor_insert = new Fl_Button(304, 8, 96, 24, "Insert");
  editor_insert->callback((Fl_Callback *)do_editor_insert);
  editor_delete = new Fl_Button(304, 48, 96, 24, "Delete");
  editor_delete->callback((Fl_Callback *)do_editor_delete);
  editor_replace = new Fl_Button(304, 88, 96, 24, "Replace");
  editor_replace->callback((Fl_Callback *)do_editor_replace);
  editor_undo = new Fl_Button(304, 144, 96, 24, "Undo");
  editor_undo->callback((Fl_Callback *)do_editor_get_undo);
  editor_rgb_ramp = new Fl_Button(304, 200, 96, 24, "RGB Ramp");
  editor_rgb_ramp->callback((Fl_Callback *)do_editor_rgb_ramp);
  editor_hsv_ramp = new Fl_Button(304, 240, 96, 24, "HSV Ramp");
  editor_hsv_ramp->callback((Fl_Callback *)do_editor_hsv_ramp);
  editor_palette = new Widget(editor, 408, 8, 192, 192, "Palette", 24, 24, (Fl_Callback *)do_editor_palette);
  editor_color = new Widget(editor, 408, 208, 192, 56, "Color", 0, 0, 0);
  new Separator(editor, 2, 272, 604, 2, "");
  editor_done = new Fl_Button(504, 280, 96, 24, "Done");
  editor_done->callback((Fl_Callback *)hide_editor);
  editor->set_modal();
  editor->end(); 
}

Dialog::~Dialog()
{
}

