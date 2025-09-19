/*
Copyright (c) 2025 Joe Davisson.

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

#ifndef FILL_OPTIONS_H
#define FILL_OPTIONS_H

#include "Group.H"

class CheckBox;
class InputInt;
class Group;
class Fl_Button;

class FillOptions : public Group
{
public:
  FillOptions(int, int, int, int, const char *);
  ~FillOptions();

  int getRange();
  int getFeather();
  int getColorOnly();
  void reset();

private:
  InputInt *fill_range;
  InputInt *fill_feather;
  CheckBox *fill_color_only;
  Fl_Button *fill_reset;
};

#endif

