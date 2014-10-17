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

#include "Rendera.H"

class Bitmap;
class Brush;
class Map;
class Palette;

namespace Project
{
  enum
  {
    THEME_LIGHT,
    THEME_DARK,
  };

  // main image
  extern Bitmap *bmp;

  // brushstroke image
  extern Map *map;

  // current brush
  extern Brush *brush;

  // palette
  extern Palette *palette;

  // overscroll amount
  extern int overscroll;

  // current theme
  extern int theme;

  // initialize stuff
  void init();

  // create main image and brushstroke map
  void newImage(int, int);
}

#endif

