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

#include "Project.H"
#include "Bitmap.H"
#include "Map.H"
#include "Palette.H"
#include "Brush.H"

namespace Project
{
  Bitmap *bmp = 0;
  Map *map = 0;
  Brush *brush = 0;
  Palette *palette = 0;
  int overscroll = 64;

  void init()
  {
    newImage(640, 480);
    brush = new Brush();
    palette = new Palette();
  }

  void newImage(int x, int y)
  {
    if(bmp)
      delete bmp;

    bmp = new Bitmap(x, y, overscroll);

    if(map)
      delete map;

    map = new Map(x + overscroll * 2, y + overscroll * 2);
    map->clear(0);
  }
}

