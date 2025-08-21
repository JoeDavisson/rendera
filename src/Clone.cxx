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

#include <FL/Fl_Double_Window.H>

#include "Bitmap.H"
#include "Brush.H"
#include "Clone.H"
#include "Inline.H"
#include "Map.H"
#include "Project.H"
#include "Stroke.H"
#include "View.H"
#include "Widget.H"

int Clone::x = 0;
int Clone::y = 0;
int Clone::dx = 0;
int Clone::dy = 0;
int Clone::state = 0;
bool Clone::active = false;
bool Clone::moved = false;
Bitmap *Clone::buffer_bmp = 0;

// change the clone target
void Clone::move(int xx, int yy)
{
  if (moved)
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

  delete buffer_bmp;
  buffer_bmp = new Bitmap(w, h);
  Project::bmp->blit(buffer_bmp, x1, y1, 0, 0, w, h);
}

