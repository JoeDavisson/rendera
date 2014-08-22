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

#include "rendera.h"

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

  Fl_PNG_Image *png_image = new Fl_PNG_Image(filename);
  bitmap = new Bitmap(png_image->w(), png_image->h());
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);

  int i;
  int index = 0;

  int xx, yy;

  const unsigned char *p = png_image->array;

  for(yy = 0; yy < png_image->h(); yy++)
  {
    for(xx = 0; xx < png_image->w(); xx++)
    {
      int r = *p++;
      int g = *p++;
      int b = *p++;

      bitmap->data[index++] = makecol(r, g, b);
    }
  }

  delete png_image;

  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
  redraw();
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
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
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

  image->draw(x() + offsetx, y() + offsety, stepx, stepy,
              offsetx + 1, offsety + 1);

  fl_pop_clip();

  int tempx = stepx > 1 ? stepx : 2;
  int tempy = stepy > 1 ? stepy : 2;

  fl_draw_box(FL_UP_FRAME, x() + offsetx, y() + offsety, tempx, tempy, FL_BLACK);
}

