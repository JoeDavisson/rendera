/*
Copyright (c) 2021 Joe Davisson.

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
#include <FL/Fl_Widget.H>

#include "Bitmap.H"
#include "Blend.H"
#include "File.H"
#include "Inline.H"
#include "Project.H"
#include "Widget.H"

// load a PNG image from a file
Widget::Widget(Fl_Group *g, int x, int y, int w, int h,
               const char *label, const unsigned char *array,
               int sx, int sy, Fl_Callback *cb)
: Fl_Widget(x, y, w, h, label)
{
  var = 0;

  if(cb)
    callback(cb, &var);

  stepx = sx;
  stepy = sy;
  group = g;

  if(!(bitmap = File::loadPngFromArray(array)))
  {
    fl_message_title("Error");
    fl_message("Could not load image.");
    exit(1);
  }

  bitmap2 = new Bitmap(bitmap->w, bitmap->h);
  bitmap->blit(bitmap2, 0, 0, 0, 0, bitmap->w, bitmap->h);
  Blend::set(Blend::LIGHTEN);
  bitmap2->rectfill(0, 0, bitmap2->w - 1, bitmap2->h - 1,
                    Project::theme_highlight_color, 192);
  Blend::set(Blend::TRANS);

  if((stepx > 1) && (stepy > 1))
  {
    for(int y1 = 0; y1 < bitmap2->h; y1 += stepy)
    {
      for(int x1 = 0; x1 < bitmap2->w; x1 += stepx)
      {
        bitmap2->xorRect(x1 + 1, y1 + 1, x1 + stepx - 2, y1 + stepy - 2);
        bitmap2->rect(x1, y1, x1 + stepx - 1, y1 + stepy - 1, makeRgb(0, 0, 0), 0);
      }
    }
  }

  image = new Fl_RGB_Image((unsigned char *)bitmap->data, bitmap->w, bitmap->h, 4, 0);

  image2 = new Fl_RGB_Image((unsigned char *)bitmap2->data, bitmap2->w, bitmap2->h, 4, 0);

  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
  use_highlight = true;
}

// load static PNG image from file
Widget::Widget(Fl_Group *g, int x, int y, int w, int h,
               const char *label, const unsigned char *array)
: Fl_Widget(x, y, w, h, label)
{
  stepx = 0;
  stepy = 0;
  group = g;

  if(!(bitmap = File::loadPngFromArray(array)))
  {
    fl_message_title("Error");
    fl_message("Could not load image.");
    exit(1);
  }

  bitmap2 = 0;
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, bitmap->w, bitmap->h, 4, 0);

  resize(group->x() + x, group->y() + y, w, h);
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
  bitmap->clear(makeRgb(0, 0, 0));
  bitmap2 = 0;
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
  use_highlight = false;
}

Widget::~Widget()
{
  delete image;
  delete bitmap;
  delete bitmap2;
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
      if(stepx <= 0 || stepy <= 0)
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

      x1 *= stepx;
      y1 *= stepy;

      do_callback();

      redraw();
      return 1;
  }

  return 0;
}

void Widget::draw()
{
//  if(stepx > 0 && stepy > 0)
//    fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), FL_BACKGROUND_COLOR);
  if(stepx > 0 && stepy > 0)
    fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), 42);

  image->draw(x(), y());
  image->uncache();

  if(stepx >= 0 && stepx <= 1 && stepy >= 0 && stepy <= 1)
    return;

  int offsety = (var / (w() / stepx)) * stepy;
  int offsetx = var;

  while(offsetx >= (w() / stepx))
    offsetx -= (w() / stepx);

  offsetx *= stepx;

  fl_push_clip(x() + offsetx, y() + offsety, stepx, stepy);

  if(stepx >= 0 && stepy >= 0)
    fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), FL_BACKGROUND2_COLOR);

  if(use_highlight)
  {
    image2->draw(x() + offsetx, y() + offsety, stepx, stepy, offsetx, offsety);
    image2->uncache();
  }
  else
  {
    image->draw(x() + offsetx, y() + offsety, stepx, stepy, offsetx, offsety);
    image->uncache();
  }

  fl_pop_clip();
}

