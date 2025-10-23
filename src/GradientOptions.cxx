/*
Copyright (c) 2025 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
state WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "Bitmap.H"
#include "CheckBox.H"
#include "GradientOptions.H"
#include "Gui.H"
#include "Inline.H"
#include "Separator.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

GradientOptions::GradientOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  gradient_style = new Fl_Choice(8, pos, 160, 32, "");
  gradient_style->tooltip("Gradient Style");
  gradient_style->textsize(10);
  gradient_style->resize(this->x() + 8, this->y() + pos, 160, 32);
  gradient_style->add("Linear");
  gradient_style->add("Radial");
  gradient_style->add("Rectangular");
  gradient_style->add("Elliptical");
  gradient_style->value(0);
  gradient_style->textsize(16);
  pos += 40 + 8;

  gradient_use_color = new CheckBox(this, 8, pos, 16, 16,
                                          "Use Paint Color", 0);
  gradient_use_color->center();
  gradient_use_color->value(0);

  pos += 32 + 8;

  gradient_inverse = new CheckBox(this, 8, pos, 16, 16, "Inverse", 0);
  gradient_inverse->center();
  gradient_inverse->value(0);

  resizable(0);
  end();
}

GradientOptions::~GradientOptions()
{
}

int GradientOptions::style()
{
  return gradient_style->value();
}

bool GradientOptions::useColor()
{
  return gradient_use_color->value() > 0 ? true : false;
}

bool GradientOptions::inverse()
{
  return gradient_inverse->value() > 0 ? true : false;
}

