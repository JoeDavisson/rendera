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

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Group.H>

#include "CheckBox.H"

CheckBox::CheckBox(Fl_Group *g, int x, int y, int w, int h,
               const char *label, Fl_Callback *cb)
: Fl_Check_Button(x, y, w, h, label)
{
  group = g;

  if(cb)
    callback(cb, &var);

  resize(group->x() + x, group->y() + y, w, h);
}

CheckBox::~CheckBox()
{
}

void CheckBox::center()
{
  int ww = 0, hh = 0;

  measure_label(ww, hh);

  resize((parent()->w() / 2) - ((ww + w()) / 2), y(), w(), h());

  redraw();
}

