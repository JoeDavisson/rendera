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

#include <cmath>

#include "Bitmap.H"
#include "Brush.H"
#include "FastRnd.H"
#include "Gui.H"
#include "Map.H"
#include "Project.H"
#include "Realtime.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"

namespace
{
  Bitmap *bmp;
  Brush *brush;
  View *view;

  void drawSolid()
  {
    int size = brush->size;
    int x = view->imgx - size / 2;
    int y = view->imgy - size / 2;

    bmp->rectfill(x, y, x + size - 1, y + size - 1,
                  Project::brush->color, Project::brush->trans);
  }

  void drawAirbrush()
  {
  }
}

void Realtime::callback()
{
  view = Gui::getView();
  bmp = Project::bmp;
  brush = Project::brush2.get();

  switch(Gui::getArtMode())
  {
    case SOLID:
      drawSolid();
      break;
    case AIRBRUSH:
      drawAirbrush();
      break;
    default:
      break;
  }

  view->drawMain(true);
}

