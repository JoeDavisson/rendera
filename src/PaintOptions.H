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

class Fl_Choice;
class InputInt;
class Group;
class Widget;

class PaintOptions : public Group
{
public:
  PaintOptions(int, int, int, int, const char *);
  ~PaintOptions();

  void changeSize(int);
  void mode();
  void size();
  void sizeValue();
  void shape();
  void stroke();
  void coarseEdge();
  void fineEdge();
  void blurryEdge();
  void watercolorEdge();
  void chalkEdge();
  void textureEdge();
  void textureMarb();
  void textureTurb();
  void averageEdge();
  void updateBrush();
  int getMode();

private:
  Widget *paint_brush_preview;
  Widget *paint_size;
  InputInt *paint_size_value;
  Widget *paint_stroke;
  Widget *paint_shape;
  Widget *paint_coarse_edge;
  Widget *paint_fine_edge;
  Widget *paint_blurry_edge;
  Widget *paint_watercolor_edge;
  Widget *paint_chalk_edge;
  Widget *paint_texture_edge;
  Widget *paint_texture_marb;
  Widget *paint_texture_turb;
  Widget *paint_average_edge;
  Fl_Choice *paint_mode;

};

#endif

