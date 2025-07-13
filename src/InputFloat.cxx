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
#include <algorithm>

#include <FL/Fl_Group.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Widget.H>

#include "InputFloat.H"

namespace
{
  char str[256];

  void change(Fl_Widget *w, InputFloat *i)
  {
    bool shift = Fl::event_shift() ? true : false;
    double val = std::atof(i->input.value());

    if (shift == true)
    {
      if (w == &i->dec)
        val -= 10;
      else if (w == &i->inc)
        val += 10;
    }
      else
    {
      if (w == &i->dec)
        val -= 1;
      else if (w == &i->inc)
        val += 1;
    }

    if (val < i->min)
      val = i->min;

    if (val > i->max)
      val = i->max;

    snprintf(str, sizeof(str), "%.5f", val);
    i->input.value(str);

    if (i->cb)
      i->cb(w, i);
  }
}

InputFloat::InputFloat(Fl_Group *g, int x, int y, int w, int h,
                   const char *text, Fl_Callback *input_cb,
                   double val_min, double val_max)
: Fl_Group(x, y, w, h, 0),
  input(x + 24, y, w - 48, h),
  dec(x, y, 24, h, "@<"),
  inc(x + w - 24, y, 24, h, "@>")
{
  resizable(input);
  end();
  align(FL_ALIGN_LEFT);
  group = g;
  var = 0;
  cb = input_cb;
  input.callback((Fl_Callback *)change, this);
  dec.callback((Fl_Callback *)change, this);
  inc.callback((Fl_Callback *)change, this);
  input.maximum_size(16);
  input.textsize(16);
  input.when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
  labelsize(16);
  copy_label(text);
  resize(group->x() + x, group->y() + y, w, h);

  min = val_min;
  max = val_max;
}

InputFloat::~InputFloat()
{
}

const char *InputFloat::value()
{
  return input.value();
}

void InputFloat::value(const char *s)
{
  input.value(s);
}

void InputFloat::maximum_size(const int size)
{
  input.maximum_size(size);
}

void InputFloat::center()
{
  int ww = 0, hh = 0;

  measure_label(ww, hh);
  resize(group->x() + group->w() / 2 - (w() + ww) / 2 + ww, y(), w(), h());
}

void InputFloat::textsize(const int size)
{
  input.textsize(size);
}

