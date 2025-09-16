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

#include <algorithm>

#include "Bitmap.H"
#include "Brush.H"
#include "ColorOptions.H"
#include "CheckBox.H"
#include "Gui.H"
#include "Inline.H"
#include "Palette.H"
#include "Picker.H"
#include "PickerOptions.H"
#include "Project.H"
#include "Undo.H"
#include "View.H"

Picker::Picker()
{
}

Picker::~Picker()
{
}

bool Picker::inbox(int x, int y, int x1, int y1, int x2, int y2)
{
  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
    return 1;
  else
    return 0;
}

void Picker::push(View *view)
{
  Bitmap *bmp = Project::bmp;
  Palette *pal = Project::palette;

  if (inbox(view->imgx, view->imgy, bmp->cl, bmp->ct, bmp->cr, bmp->cb))
  {
    const int color = bmp->getpixel(view->imgx, view->imgy);

    if (Gui::picker->useBest() == 1)
    {
      int nearest = 99999999;
      int use = 0;

      for (int i = 0; i < pal->max; i++)
      {
        int d = diff24(color, pal->data[i]);

        if (d < nearest)
        {
          nearest = d;
          use = i;
        }
      }

      Gui::colors->paletteIndex(use);
    }

    Gui::colors->colorUpdate(color);
    Gui::picker->update(color);
  }
}

void Picker::drag(View *view)
{
  push(view);
}

void Picker::release(View *)
{
}

void Picker::move(View *)
{
}

void Picker::key(View *)
{
}

void Picker::done(View *, int)
{
}

void Picker::redraw(View *view)
{
  view->drawMain(true);
}

bool Picker::isActive()
{
  return false;
}

void Picker::reset()
{
}

