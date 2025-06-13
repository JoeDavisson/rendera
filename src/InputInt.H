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

#ifndef INPUT_INT_H
#define INPUT_INT_H

#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Repeat_Button.H>

class Fl_Group;
class Fl_Repeat_Button;

class InputInt : public Fl_Group
{
public:
  InputInt(Fl_Group *, int, int, int, int, const char *,
           Fl_Callback *, int, int);
  ~InputInt();

  const char *value();
  void value(const char *);
  void maximum_size(const int);
  void center();
  void textsize(const int);

  int var;
  int min;
  int max;
  Fl_Group *group;
  Fl_Int_Input input;
  Fl_Repeat_Button dec;
  Fl_Repeat_Button inc;
  void (*cb)(Fl_Widget *, void *);
};

#endif

