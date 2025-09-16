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
#include "Inline.H"
#include "Palette.H"
#include "PickerOptions.H"
#include "Project.H"
#include "Separator.H"
#include "Widget.H"

#include <FL/Fl_Group.H>

namespace
{
  Widget *picker_color;
  CheckBox *picker_best;
}

PickerOptions::PickerOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  picker_color = new Widget(this, 8, pos, 160, 160, "Selected Color", 0, 0, 0);
  picker_color->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  pos += 160 + Gui::SPACING + 16;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  picker_best = new CheckBox(this, 8, pos, 16, 16, "Best Match", 0);
  picker_best->tooltip("Use nearest color in palette");
  picker_best->center();
  picker_best->value(0);

  resizable(0);
  end();
}

PickerOptions::~PickerOptions()
{
}

void PickerOptions::update(int c)
{
  Palette *pal = Project::palette;
  const int index = Gui::colors->paletteSwatchesIndex();

  if (picker_best->value())
  {
    picker_color->bitmap->clear(pal->data[index]);
  }
    else
  {
    picker_color->bitmap->clear(c);
  }

  picker_color->bitmap->rect(0,
                               0,
                               picker_color->bitmap->w - 1,
                               picker_color->bitmap->h - 1,
                               makeRgb(0, 0, 0), 0);

  picker_color->redraw();
}

int PickerOptions::useBest()
{
  return picker_best->value();
}

