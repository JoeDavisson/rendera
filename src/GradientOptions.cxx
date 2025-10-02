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

/*
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
*/

#include "GradientOptions.H"
#include "Gui.H"

namespace
{
//  void cb_reset(Fl_Widget *w, void *data) { FillOptions *temp = (FillOptions *)data; temp->reset(); }
}

GradientOptions::GradientOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
//  int pos = Group::title_height + Gui::SPACING;

  resizable(0);
  end();
}

GradientOptions::~GradientOptions()
{
}

