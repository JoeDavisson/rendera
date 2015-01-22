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

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <FL/Fl_Check_Button.H>

class Fl_Group;

class CheckBox : public Fl_Check_Button
{
public:
  CheckBox(Fl_Group *, int, int, int, int,
         const char *, Fl_Callback *);
  ~CheckBox();
  void center();

  int var;
  Fl_Group *group;
};

#endif

