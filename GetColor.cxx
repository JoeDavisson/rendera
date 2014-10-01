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

#include "GetColor.H"
#include "Bitmap.H"
#include "Palette.H"
#include "View.H"
#include "Gui.H"

namespace
{
  bool inbox(int x, int y, int x1, int y1, int x2, int y2)
  {
    using std::swap ;
    if(x1 > x2)
      swap(x1, x2);
    if(y1 > y2)
      swap(y1, y2);

    if(x >= x1 && x <= x2 && y >= y1 && y <= y2)
      return 1;
    else
      return 0;
  }
}

GetColor::GetColor()
{
}

GetColor::~GetColor()
{
}

void GetColor::push(View *view)
{
  if(inbox(view->imgx, view->imgy, Bitmap::main->cl, Bitmap::main->ct,
                                   Bitmap::main->cr, Bitmap::main->cb))
  {
    int c = Bitmap::main->getpixel(view->imgx, view->imgy);
    Gui::updateColor(c);
    Gui::updateGetColor(c);
  }
}

void GetColor::drag(View *view)
{
  push(view);
}

void GetColor::release(View *)
{
}

void GetColor::move(View *)
{
}

void GetColor::done(View *)
{
}

void GetColor::redraw(View *)
{
}

