/*
Copyright (c) 2025 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
state WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "Bitmap.H"
#include "CheckBox.H"
#include "ColorOptions.H"
#include "Gui.H"
#include "GetColorOptions.H"
#include "Inline.H"
#include "Palette.H"
#include "Project.H"
#include "Separator.H"
#include "Widget.H"

#include <FL/Fl_Group.H>

namespace
{
  Widget *getcolor_color;
  CheckBox *getcolor_best;
}

GetColorOptions::GetColorOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  getcolor_color = new Widget(this, 8, pos, 160, 160, "Selected Color", 0, 0, 0);
  getcolor_color->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  pos += 160 + Gui::SPACING + 16;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  getcolor_best = new CheckBox(this, 8, pos, 16, 16, "Best Match", 0);
  getcolor_best->tooltip("Use nearest color in palette");
  getcolor_best->center();
  getcolor_best->value(0);

  resizable(0);
  end();
}

GetColorOptions::~GetColorOptions()
{
}

void GetColorOptions::getcolorUpdate(int c)
{
  Palette *pal = Project::palette;
//  const int index = palette_swatches->var;
  const int index = Gui::colors->paletteSwatchesIndex();

  if (getcolor_best->value())
  {
    getcolor_color->bitmap->clear(pal->data[index]);
  }
    else
  {
    getcolor_color->bitmap->clear(c);
  }

  getcolor_color->bitmap->rect(0,
                               0,
                               getcolor_color->bitmap->w - 1,
                               getcolor_color->bitmap->h - 1,
                               makeRgb(0, 0, 0), 0);
  getcolor_color->redraw();
}

int GetColorOptions::getcolorUseBest()
{
  return getcolor_best->value();
}

