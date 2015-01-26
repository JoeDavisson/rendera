/*
Copyright (c) 2015 Joe Davisson.

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

#include <algorithm>

#include "Bitmap.H"
#include "Brush.H"
#include "Fill.H"
#include "Project.H"
#include "Undo.H"
#include "View.H"

namespace
{
  bool inbox(int x, int y, int x1, int y1, int x2, int y2)
  {
    if(x1 > x2)
      std::swap(x1, x2);
    if(y1 > y2)
      std::swap(y1, y2);

    if(x >= x1 && x <= x2 && y >= y1 && y <= y2)
      return 1;
    else
      return 0;
  }
}

Fill::Fill()
{
}

Fill::~Fill()
{
}

void Fill::push(View *view)
{
  if(inbox(view->imgx, view->imgy, Project::bmp->cl, Project::bmp->ct,
                                   Project::bmp->cr, Project::bmp->cb))
  {
    Undo::push();
    int c = Project::bmp->getpixel(view->imgx, view->imgy);
    Project::bmp->fill(view->imgx, view->imgy, Project::brush->color, c);
    view->drawMain(true);
  }
}

void Fill::drag(View *)
{
}

void Fill::release(View *)
{
}

void Fill::move(View *)
{
}

void Fill::done(View *)
{
}

void Fill::redraw(View *)
{
}

bool Fill::isActive()
{
  return false;
}

void Fill::reset()
{
}

