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

#ifndef VIEW_OPTIONS_H
#define VIEW_OPTIONS_H

#include "Group.H"

class Button;
class CheckBox;
class Group;
class InputInt;
class StaticText;
class ToggleButton;
class Fl_Choice;

class ViewOptions : public Group
{
public:
  ViewOptions(int, int, int, int, const char *);
  ~ViewOptions();

  void zoomIn();
  void zoomOut();
  void zoomOne();
  void zoomLevel();

//  void gridEnable(ToggleButton *, void *);
//  void gridSnap(ToggleButton *, void *);
  void gridEnable();
  void gridSnap();
  void gridX();
  void gridY();

  void aspectMode();

private:
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  StaticText *zoom;
  ToggleButton *grid;
  ToggleButton *gridsnap;
  InputInt *gridx;
  InputInt *gridy;
  Fl_Choice *aspect;
};

#endif

