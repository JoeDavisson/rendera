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

#include "Bitmap.H"
#include "Clone.H"
#include "Project.H"

namespace Clone
{
  int x = 0;
  int y = 0;
  int dx = 0;
  int dy = 0;
  int state = 0;
  bool wrap = false;
  int mirror = 0;
  bool active = false;
  bool moved = false;
  Bitmap *bmp = 0;
}

// change the clone target
void Clone::move(int xx, int yy)
{
  if(moved)
  {
    dx = xx - x; 
    dy = yy - y; 
    moved = false;
  }
}

// set clone buffer bitmap to the correct size
void Clone::refresh(int x1, int y1, int x2, int y2)
{
  const int w = x2 - x1;
  const int h = y2 - y1;

  if(bmp)
    delete bmp;

  bmp = new Bitmap(w, h);
  Project::bmp->blit(bmp, x1, y1, 0, 0, w, h);

  state = 0;
}

