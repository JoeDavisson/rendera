/*
Copyright (c) 2025 Joe Davisson.

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
class Picker;
class Selection;
class Offset;
class Text;
class Fill;
class Gradient;

class Project
{
public:
  enum
  {
    THEME_LIGHT,
    THEME_DARK
  };

  static Bitmap *bmp;
  static Map *map;
  static Bitmap *select_bmp;

  static Brush *brush;
  static Palette *palette;
  static Stroke *stroke;
  static Undo *undo;

  static int max_images;
  static Bitmap **bmp_list;
  static Undo **undo_list;
  static int ox_list[256];
  static int oy_list[256];
  static float zoom_list[256];
  static int current;
  static int last;
  static int mem_max;
  static int undo_max;

  static Paint *paint;
  static Picker *picker;
  static Selection *selection;
  static Offset *offset;
  static Text *text;
  static Fill *fill;
  static Gradient *gradient;

  static Tool *tool;

  static int theme;
  static int theme_highlight_color;
  static Fl_Color fltk_theme_highlight_color;
  static Fl_Color fltk_theme_bevel_up;
  static Fl_Color fltk_theme_bevel_down;

  static void init(int, int);
  static void setTool(int);
  static bool enoughMemory(int, int);
  static int newImage(int, int);
  static int newImageFromBitmap(Bitmap *);
  static void replaceImage(int, int);
  static void replaceImageFromBitmap(Bitmap *);
  static void resizeImage(int, int);
  static void switchImage(int);
  static bool removeImage();
  static double getImageMemory();
  static bool swapImage(const int, const int);
  static void pop();
  static void popRedo();

private:
  Project() { }
  ~Project() { }
};

#endif

