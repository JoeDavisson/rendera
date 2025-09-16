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

#ifndef PAINT_OPTIONS_H
#define PAINT_OPTIONS_H

#include "Group.H"

class Group;
class Widget;

class PaintOptions : public Group
{
public:
  PaintOptions(int, int, int, int, const char *);
  ~PaintOptions();

  void changeSize(int);
  void mode();
  void size(Widget *, void *);
  void sizeValue(Widget *, void *);
  void shape(Widget *, void *);
  void stroke(Widget *, void *);
  void coarseEdge(Widget *, void *);
  void fineEdge(Widget *, void *);
  void blurryEdge(Widget *, void *);
  void watercolorEdge(Widget *, void *);
  void chalkEdge(Widget *, void *);
  void textureEdge(Widget *, void *);
  void textureMarb(Widget *, void *);
  void textureTurb(Widget *, void *);
  void averageEdge(Widget *, void *);
  void updateBrush();
  int getMode();
};

#endif

