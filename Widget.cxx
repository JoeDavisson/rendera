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

#include "Widget.H"
#include "Bitmap.H"
#include "Blend.H"
#include "File.H"

// load a PNG image from a file
Widget::Widget(Fl_Group *g, int x, int y, int w, int h,
               const char *label, const char *filename,
               int sx, int sy, Fl_Callback *cb)
: Fl_Widget(x, y, w, h, label)
{
  var = 0;

  if(cb)
    callback(cb, &var);

  stepx = sx;
  stepy = sy;
  group = g;

  if(!(bitmap = File::loadPNG(filename, 0)))
  {
    fl_message_title("Error");
    fl_message("Could not load %s, exiting.", filename);
    exit(1);
  }

  image = new Fl_RGB_Image((unsigned char *)bitmap->data, bitmap->w, bitmap->h, 4, 0);

  bitmap2 = new Bitmap(bitmap->w, bitmap->h);
  image2 = new Fl_RGB_Image((unsigned char *)bitmap2->data, bitmap2->w, bitmap2->h, 4, 0);
  bitmap->blit(bitmap2, 0, 0, 0, 0, bitmap->w, bitmap->h);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
  use_highlight = 1;

  // shade
  bitmap->rectfill(0, 0, bitmap->w - 1, bitmap->h - 1, makeRgb(128, 128, 128), 192);
}

// use a blank bitmap
Widget::Widget(Fl_Group *g, int x, int y, int w, int h,
               const char *label, int sx, int sy, Fl_Callback *cb)
: Fl_Widget(x, y, w, h, label)
{
  var = 0;

  if(cb)
    callback(cb, &var);

  stepx = sx;
  stepy = sy;
  group = g;
  bitmap = new Bitmap(w, h);
  bitmap->clear(makeRgb(255, 255, 255));
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
  use_highlight = 0;
}

Widget::~Widget()
{
}

int Widget::handle(int event)
{
  int x1, y1;

  switch(event)
  {
    case FL_ENTER:
      return 1;
    case FL_PUSH:
    case FL_DRAG:
      if(stepx == 0 || stepy == 0)
        return 0;

      x1 = (Fl::event_x() - x()) / stepx;
      if(x1 > w() / stepx - 1)
        x1 = w() / stepx - 1;
      if(x1 < 0)
        x1 = 0;

      y1 = (Fl::event_y() - y()) / stepy;
      if(y1 > h() / stepy - 1)
        y1 = h() / stepy - 1;
      if(y1 < 0)
        y1 = 0;

      var = x1 + (w() / stepx) * y1;
      do_callback();
      redraw();
      return 1;
  }

  return 0;
}

void Widget::draw()
{
  if(use_highlight && (stepx <= 1 || stepy <= 1))
    image2->draw(x(), y());
  else
    image->draw(x(), y());
    
  fl_draw_box(FL_DOWN_FRAME, x(), y(), w(), h(), FL_BLACK);

  if(stepx <= 1 && stepy <= 1)
    return;

  int offsety = (var / (w() / stepx)) * stepy;
  int offsetx = var;

  while(offsetx >= (w() / stepx))
    offsetx -= (w() / stepx);

  offsetx *= stepx;

  fl_push_clip(x() + offsetx, y() + offsety, stepx, stepy);

  if(use_highlight)
    image2->draw(x() + offsetx, y() + offsety, stepx, stepy,
                offsetx + 1, offsety + 1);
  else
    image->draw(x() + offsetx, y() + offsety, stepx, stepy,
                offsetx + 1, offsety + 1);

  fl_pop_clip();

  int tempx = stepx > 1 ? stepx : 2;
  int tempy = stepy > 1 ? stepy : 2;

  fl_draw_box(FL_UP_FRAME, x() + offsetx, y() + offsety, tempx, tempy, FL_BLACK);
}

