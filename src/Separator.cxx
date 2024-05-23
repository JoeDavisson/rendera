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

#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>

#include "Blend.H"
#include "Common.H"
#include "Separator.H"

Separator::Separator(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Widget(x, y, w, h, label)
{
  group = g;
  resize(group->x() + x, group->y() + y, w, h);
}

Separator::~Separator()
{
}

void Separator::draw()
{
//  fl_draw_box(FL_THIN_DOWN_FRAME, x(), y(), w(), h(), FL_BACKGROUND2_COLOR); 
  int up = Blend::trans(getFltkColor(FL_BACKGROUND2_COLOR), makeRgb(0, 0, 0), 160);
  int down = Blend::trans(getFltkColor(FL_BACKGROUND2_COLOR), makeRgb(255, 255, 255), 160);
  up = fl_rgb_color(getr(up), getg(up), getb(up));
  down = fl_rgb_color(getr(down), getg(down), getb(down));

  fl_color(up);
  fl_xyline(x(), y(), x() + w() - 1);
  fl_yxline(x(), y(), y() + h() - 1);
  fl_color(down);
  fl_xyline(x(), y() + h() - 1, x() +  w() - 1);
  fl_yxline(x() + w() - 1, y(), y() +  h() - 1);
}

