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

#include <cstdlib>

#include "Button.H"
#include "Bitmap.H"
#include "CheckBox.H"
#include "FillOptions.H"
#include "Gui.H"
#include "Images.H"
#include "InputInt.H"
#include "Project.H"
#include "RepeatButton.H"
#include "Selection.H"
#include "Separator.H"
#include "StaticText.H"
#include "Tool.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Group.H>

namespace
{
  void cb_reset(Fl_Widget *w, void *data) { FillOptions *temp = (FillOptions *)data; temp->reset(); }
}

FillOptions::FillOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  fill_range = new InputInt(this, 8, pos, 160, 32, "Range (0-31)", 0, 0, 31);
  fill_range->align(FL_ALIGN_BOTTOM);
  fill_range->value(0);
  pos += 32 + 32;

  fill_feather = new InputInt(this, 8, pos, 160, 32, "Feather (0-255)", 0, 0, 255);
  fill_feather->align(FL_ALIGN_BOTTOM);
  fill_feather->value(0);
  pos += 32 + 32;

  fill_color_only = new CheckBox(this, 8, pos, 16, 16, "Color Only", 0);
  fill_color_only->center();
  fill_color_only->value(0);
  pos += 32;
  
  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  fill_reset = new Fl_Button(this->x() + 8, this->y() + pos, 160, 48, "Reset");
  fill_reset->callback(cb_reset, (void *)this);

  resizable(0);
  end();
}

FillOptions::~FillOptions()
{
}

int FillOptions::getRange()
{
  return fill_range->value();
}

int FillOptions::getFeather()
{
  return fill_feather->value();
}

int FillOptions::getColorOnly()
{
  return fill_color_only->value();
}

void FillOptions::reset()
{
  fill_range->value(0);
  fill_feather->value(0);
}

