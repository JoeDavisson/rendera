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
  int getJpegQualityValue();
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
}

#endif

