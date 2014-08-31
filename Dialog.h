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

class Field;
class Widget;

#include "rendera.h"

namespace Dialog
{
  void init();
  void jpegQualityCloseCallback(Fl_Widget *, void *);
  void showJpegQuality();
  void showProgress(float);
  void updateProgress();
  void hideProgress();
  void showAbout();
  void hideAbout();
  void showNewImage();
  void hideNewImage();
  void cancelNewImage();
  void showCreatePalette();
  void hideCreatePalette();
  void cancelCreatePalette();
  void showLoadPalette();
  void showSavePalette();
  void showEditor();
  void hideEditor();
  void doEditorPalette(Widget *, void *);
  void doEditorSetHsv(bool);
  void doEditorSetHsvSliders();
  void doEditorGetH();
  void doEditorGetSV();
  void doEditorInsert();
  void doEditorDelete();
  void doEditorReplace();
  void doEditorStoreUndo();
  void doEditorGetUndo();
  void doEditorRgbRamp();
  void doEditorHsvRamp();

  extern Fl_Double_Window *jpeg_quality;
  extern Field *jpeg_quality_amount;
  extern Fl_Button *jpeg_quality_ok;

  extern Fl_Double_Window *progress;
  extern Fl_Progress *progress_bar;

  extern Fl_Double_Window *about;
  extern Widget *about_logo;
  extern Fl_Button *about_ok;

  extern Fl_Double_Window *new_image;
  extern Field *new_image_width;
  extern Field *new_image_height;
  extern Fl_Button *new_image_ok;
  extern Fl_Button *new_image_cancel;

  extern Fl_Double_Window *create_palette;
  extern Field *create_palette_colors;
  extern Fl_Button *create_palette_ok;
  extern Fl_Button *create_palette_cancel;

  extern Fl_Double_Window *editor;
  extern Widget *editor_h;
  extern Widget *editor_sv;
  extern Fl_Button *editor_insert;
  extern Fl_Button *editor_delete;
  extern Fl_Button *editor_replace;
  extern Fl_Button *editor_undo;
  extern Fl_Button *editor_rgb_ramp;
  extern Fl_Button *editor_hsv_ramp;
  extern Widget *editor_palette;
  extern Widget *editor_color;
  extern Fl_Button *editor_done;
}

#endif

