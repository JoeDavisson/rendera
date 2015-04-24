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

#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>

#include "InputInt.H"

InputInt::InputInt(Fl_Group *g, int x, int y, int w, int h,
                   const char *text, Fl_Callback *cb)
: Fl_Int_Input(x, y, w, h, 0)
{
  group = g;
  var = 0;
  if(cb)
    callback(cb, &var);
  maximum_size(5);
  labelsize(12);
  textsize(12);
  label(text);
  when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
  resize(group->x() + x, group->y() + y, w, h);
}

InputInt::~InputInt()
{
}

void InputInt::center()
{
  int ww = 0, hh = 0;

  this->measure_label(ww, hh);

  resize(parent()->w() / 2 - (ww + w()) / 2 + ww, y(), w(), h());
}

int InputInt::limitValue(int min, int max)
{
  char str[8];
  int val = std::atoi(value());

  if(val < min)
  {
    snprintf(str, sizeof(str), "%d", min);
    value(str);
    return -1;
  }

  if(val > max)
  {
    snprintf(str, sizeof(str), "%d", max);
    value(str);
    return -1;
  }

  return 0;
}

