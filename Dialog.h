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

class Dialog
{
public:
  static void init();
  static void jpegQualityCloseCallback(Fl_Widget *, void *);
  static void showJpegQuality();
  static void showProgress(float);
  static void updateProgress();
  static void hideProgress();
  static void showAbout();
  static void hideAbout();
  static void showNewImage();
  static void hideNewImage();
  static void cancelNewImage();
  static void showCreatePalette();
  static void hideCreatePalette();
  static void cancelCreatePalette();
  static void showLoadPalette();
  static void showSavePalette();
  static void showEditor();
  static void hideEditor();
  static void doEditorPalette(Widget *, void *);
  static void doEditorSetHsv();
  static void doEditorSetHsvSliders();
  static void doEditorGetHsv();
  static void doEditorInsert();
  static void doEditorDelete();
  static void doEditorReplace();
  static void doEditorStoreUndo();
  static void doEditorGetUndo();
  static void doEditorRgbRamp();
  static void doEditorHsvRamp();

  static Fl_Double_Window *jpeg_quality;
  static Field *jpeg_quality_amount;
  static Fl_Button *jpeg_quality_ok;

  static Fl_Double_Window *progress;
  static Fl_Progress *progress_bar;

  static Fl_Double_Window *about;
  static Widget *about_logo;
  static Fl_Button *about_ok;

  static Fl_Double_Window *new_image;
  static Field *new_image_width;
  static Field *new_image_height;
  static Fl_Button *new_image_ok;
  static Fl_Button *new_image_cancel;

  static Fl_Double_Window *create_palette;
  static Field *create_palette_colors;
  static Fl_Button *create_palette_ok;
  static Fl_Button *create_palette_cancel;

  static Fl_Double_Window *editor;
  static Widget *editor_h;
  static Widget *editor_sv;
  static Fl_Button *editor_insert;
  static Fl_Button *editor_delete;
  static Fl_Button *editor_replace;
  static Fl_Button *editor_undo;
  static Fl_Button *editor_rgb_ramp;
  static Fl_Button *editor_hsv_ramp;
  static Widget *editor_palette;
  static Widget *editor_color;
  static Fl_Button *editor_done;
};

#endif

