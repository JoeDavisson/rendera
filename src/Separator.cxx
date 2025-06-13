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
#include "Inline.H"
#include "Separator.H"

namespace
{
  const int gap = 1;
}

Separator::Separator(Fl_Group *g,
                     int x, int y, int size, int direction,
                     const char *label)
: Fl_Widget(x, y, 8, 8, label)
{
  group = g;

  if (direction == HORIZONTAL) 
  {
    dir = HORIZONTAL;
    resize(group->x() + x, group->y() + y, size, 4);
  }
    else
  {
    dir = VERTICAL;
    resize(group->x() + x, group->y() + y, 4, size);
  }
}

Separator::~Separator()
{
}

void Separator::draw()
{
//  fl_draw_box(FL_THIN_DOWN_FRAME, x(), y(), w(), h(), FL_BACKGROUND2_COLOR); 
//  int up = Blend::trans(getFltkColor(FL_BACKGROUND2_COLOR), makeRgb(0, 0, 0), 160);
//  int down = Blend::trans(getFltkColor(FL_BACKGROUND2_COLOR), makeRgb(255, 255, 255), 160);
//  up = fl_rgb_color(getr(up), getg(up), getb(up));
//  down = fl_rgb_color(getr(down), getg(down), getb(down));

/*
  fl_color(up);
  fl_xyline(x(), y() - 1, x() + w() - 1);
  fl_yxline(x(), y() - 1, y() - 1 + h() - 1);
  fl_color(down);
  fl_xyline(x(), y() - 1 + h() - 1, x() +  w() - 1);
  fl_yxline(x() + w() - 1, y() - 1, y() - 1 +  h() - 1);
*/

  if (dir == HORIZONTAL)
  {
    fl_color(40);
    fl_xyline(x() + gap + 1, y() + 0, x() + w() - gap * 2 - 1);
    fl_color(36);
    fl_xyline(x() + gap + 0, y() + 1, x() + w() - gap * 2 - 0);
    fl_color(46);
    fl_xyline(x() + gap + 0, y() + 2, x() + w() - gap * 2 - 0);
    fl_color(48);
    fl_xyline(x() + gap + 1, y() + 3, x() + w() - gap * 2 - 1);
  }
    else
  {
    fl_color(40);
    fl_yxline(x() + 0, y() + gap + 1, y() + h() - gap * 2 - 1);
    fl_color(36);
    fl_yxline(x() + 1, y() + gap + 0, y() + h() - gap * 2 - 0);
    fl_color(46);
    fl_yxline(x() + 2, y() + gap + 0, y() + h() - gap * 2 - 0);
    fl_color(48);
    fl_yxline(x() + 3, y() + gap + 1, y() + h() - gap * 2 - 1);
  }
}

