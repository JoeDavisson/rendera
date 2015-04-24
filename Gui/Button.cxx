/*
Copyright (c) 2015 Joe Davisson.

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

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>

#include "Bitmap.H"
#include "Button.H"
#include "File.H"
#include "Project.H"

Button::Button(Fl_Group *g, int x, int y, int w, int h,
               const char *label, const unsigned char *array, Fl_Callback *cb)
: Fl_Button(x, y, w, h, label)
{
  var = 0;

  if(cb)
    callback(cb, &var);

  group = g;

  if(!(bitmap = File::loadPngFromArray(array, 0)))
  {
    fl_message_title("Error");
//    fl_message("Could not load %s, exiting.", filename);
    fl_message("Could not load image.");
    exit(1);
  }

  image = new Fl_RGB_Image((unsigned char *)bitmap->data, bitmap->w, bitmap->h, 4, 0);

  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
}

Button::~Button()
{
}

void Button::draw()
{
  fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(),
              value() ? Project::fltk_theme_highlight_color : FL_BACKGROUND_COLOR);

  fl_push_clip(x(), y(), w(), h());

  if(value())
    image->draw(x() + 1, y() + 1);
  else
    image->draw(x(), y());

  image->uncache();
  fl_pop_clip();

  fl_draw_box(value() ? FL_DOWN_FRAME : FL_UP_FRAME,
              x(), y(), w(), h(),
              FL_BLACK);
}

