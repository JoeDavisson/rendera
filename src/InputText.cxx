/*
Copyright (c) 2024 Joe Davisson.

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

#include <cstdio>
#include <cstdlib>

#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

#include "InputText.H"

InputText::InputText(Fl_Group *g, int x, int y, int w, int h,
                     const char *text, Fl_Callback *cb)
: Fl_Input(x, y, w, h, 0)
{
  group = g;
  var = 0;
  if (cb)
    callback(cb, &var);
  maximum_size(256);
  labelsize(12);
  textsize(12);
  copy_label(text);
  when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
  resize(group->x() + x, group->y() + y, w, h);
}

InputText::~InputText()
{
}

void InputText::center()
{
  int ww = 0, hh = 0;

  measure_label(ww, hh);

  resize(group->x() + group->w() / 2 - (ww + w()) / 2 + ww, y(), w(), h());
}

