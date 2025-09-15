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

  void paintChangeSize(int);
  void paintMode();
  void paintSize(Widget *, void *);
  void paintSizeValue(Widget *, void *);
  void paintShape(Widget *, void *);
  void paintStroke(Widget *, void *);
  void paintCoarseEdge(Widget *, void *);
  void paintFineEdge(Widget *, void *);
  void paintBlurryEdge(Widget *, void *);
  void paintWatercolorEdge(Widget *, void *);
  void paintChalkEdge(Widget *, void *);
  void paintTextureEdge(Widget *, void *);
  void paintTextureMarb(Widget *, void *);
  void paintTextureTurb(Widget *, void *);
  void paintAverageEdge(Widget *, void *);
  void paintUpdateBrush();
  int getPaintMode();
};

#endif

