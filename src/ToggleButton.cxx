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

#include "Bitmap.H"
#include "File.H"
#include "Project.H"
#include "ToggleButton.H"

ToggleButton::ToggleButton(Fl_Group *g, int x, int y, int w, int h,
                           const char *label, const char *filename,
                           Fl_Callback *cb)
: Fl_Button(x, y, w, h, label)
{
  var = 0;

  if(cb)
    callback(cb, &var);

  group = g;

  if(!(bitmap = File::loadPng(filename, 0)))
  {
    fl_message_title("Error");
    fl_message("Could not load %s, exiting.", filename);
    exit(1);
  }

  image = new Fl_RGB_Image((unsigned char *)bitmap->data, bitmap->w, bitmap->h, 4, 0);

  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
}

ToggleButton::~ToggleButton()
{
}

int ToggleButton::handle(int event)
{
  switch(event)
  {
    case FL_ENTER:
      return 1;
    case FL_PUSH:
      switch(Fl::event_button())
      {
        case 1:
          var = 1 - var;
          do_callback();
          redraw();
          return 1;
      }
  }

  return 0;
}

void ToggleButton::draw()
{
  fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(),
              var ? Project::fltk_theme_color : FL_BACKGROUND_COLOR);

  fl_push_clip(x(), y(), w(), h());

  if(var)
    image->draw(x() + 1, y() + 1);
  else
    image->draw(x(), y());

  image->uncache();
  fl_pop_clip();

  fl_draw_box(var ? FL_DOWN_FRAME : FL_UP_FRAME, x(), y(), w(), h(), FL_BLACK);
}

