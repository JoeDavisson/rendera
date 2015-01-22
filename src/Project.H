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

#ifndef PROJECT_H
#define PROJECT_H

#include <FL/Fl.H>

#include "SP.H"

class Bitmap;
class Brush;
class Map;
class Palette;
class Tool;
class Stroke;

namespace Project
{
  enum
  {
    THEME_LIGHT,
    THEME_DARK
  };

  extern Bitmap *bmp;
  extern Map *map;

  extern SP<Brush> brush;
  extern SP<Palette> palette;
  extern SP<Stroke> stroke;
  extern Tool *tool;

  extern int overscroll;
  extern int theme;
  extern char theme_path[];
  extern int theme_color;
  extern Fl_Color fltk_theme_color;

  void init();
  void setTool(int);
  void newImage(int, int);
}

#endif

