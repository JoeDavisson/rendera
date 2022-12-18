/*
Copyright (c) 2021 Joe Davisson.

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

#ifndef PROJECT_H
#define PROJECT_H

#include <FL/Fl.H>

class Bitmap;
class Brush;
class Map;
class Palette;
class Tool;
class Stroke;
class Undo;

class Paint;
class GetColor;
class Selection;
class Offset;
class Text;
class Fill;

namespace Project
{
  enum
  {
    THEME_LIGHT,
    THEME_DARK
  };

  extern Bitmap *bmp;
  extern Bitmap *select_bmp;
  extern Map *map;

  extern Brush *brush;
  extern Palette *palette;
  extern Stroke *stroke;
  extern Undo *undo;

  extern Bitmap **bmp_list;
  extern Undo **undo_list;
  extern int current;
  extern int last;

  extern Paint *paint;
  extern GetColor *getcolor;
  extern Selection *selection;
  extern Offset *offset;
  extern Text *text;
  extern Fill *fill;

  extern Tool *tool;

  extern int mode;
  extern int overscroll;
  extern int theme;
  extern char theme_path[];
  extern int theme_highlight_color;
  extern Fl_Color fltk_theme_highlight_color;
  extern Fl_Color fltk_theme_bevel_up;
  extern Fl_Color fltk_theme_bevel_down;

  void init();
  void setTool(int);
  int newImage(int, int);
  int newImageFromBitmap(Bitmap *);
  void replaceImage(int, int);
  void replaceImageFromBitmap(Bitmap *);
  void resizeImage(int, int);
  void switchImage(int);
  bool removeImage();
  void pop();
  void popRedo();
}

#endif

