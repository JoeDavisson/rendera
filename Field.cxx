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

#include "rendera.h"

Field::Field(Fl_Group *g, int x, int y, int w, int h, const char *text, Fl_Callback *cb)
: Fl_Int_Input(x, y, w, h, 0)
{
  group = g;
  var = 0;
  if(cb)
    callback(cb, &var);
  maximum_size(3);
  labelsize(12);
  textsize(12);
  label(text);
  when(FL_WHEN_CHANGED | FL_WHEN_ENTER_KEY);
  resize(group->x() + x, group->y() + y, w, h);
//  tooltip(label);
}

Field::~Field()
{
}

