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

#ifndef SELECTION_OPTIONS_H
#define SELECTION_OPTIONS_H

#include "Group.H"

class Button;
class CheckBox;
class Group;
class StaticText;
class Fl_Button;

class SelectionOptions : public Group
{
public:
  SelectionOptions(int, int, int, int, const char *);
  ~SelectionOptions();

  void alpha();
  void copy();
  void copyEnable(bool);
  void paste();
  void pasteEnable(bool);
  void crop();
  void cropEnable(bool);
  void flipX();
  void flipY();
  void reset();
  void rotate90();
  void values(int, int, int, int);
  int getAlpha();

private:
  StaticText *selection_x;
  StaticText *selection_y;
  StaticText *selection_w;
  StaticText *selection_h;
  Fl_Button *selection_reset;
  Fl_Button *selection_copy;
  CheckBox *selection_alpha;
  Button *selection_flip;
  Button *selection_mirror;
  Button *selection_rotate;
  Fl_Button *selection_paste;
  Fl_Button *selection_crop;
};

#endif

