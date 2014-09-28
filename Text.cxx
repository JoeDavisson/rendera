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

#include "Text.H"
#include "Tool.H"
#include "Bitmap.H"
#include "Blend.H"
#include "Map.H"
#include "Brush.H"
#include "View.H"
#include "Stroke.H"
#include "Gui.H"
#include "Undo.H"

namespace
{
  Bitmap *temp;
  int temp_allocated = 0;
}

Text::Text()
{
}

Text::~Text()
{
}

void Text::push(View *view)
{
  if(!started)
    move(view);

  Undo::push(stroke->x1,
             stroke->y1,
             (stroke->x2 - stroke->x1) + 1,
             (stroke->y2 - stroke->y1) + 1, 0);

  int x, y;
  Bitmap *dest = Bitmap::main;
  Blend::set(Brush::main->blend);
  int imgx = view->imgx;
  int imgy = view->imgy;

  for(y = 0; y < temp->h; y++)
  {
    for(x = 0; x < temp->w; x++)
    {
      int c = temp->getpixel(x, y);
      int t = getv(c);
      if(t < 255)
        dest->setpixel(imgx - temp->w / 2 + x, imgy - temp->h / 2 + y,
                       Brush::main->color, scaleVal(Brush::main->trans, t));
    }
  }

  if(temp_allocated)
  {
    delete temp;
    temp_allocated = 0;
  }

  Blend::set(Blend::TRANS);

  active = 0;
  started = 0;
  view->drawMain(1);
}

void Text::drag(View *view)
{
}

void Text::release(View *)
{
}

void Text::move(View *view)
{
  if(!started)
  {
    started = 1;
    active = 1;
    int tw = 0;
    int th = 0;
    int face = Gui::getFontFace();
    int size = Gui::getFontSize();
    fl_font(face, size);
    const char *string = Gui::getTextInput();
    fl_measure(string, tw, th, 1);
    Fl_Offscreen offscreen = fl_create_offscreen(tw, th);

    if(temp_allocated)
    {
      delete temp;
      temp_allocated = 0;
    }

    temp = new Bitmap(tw, th);
    temp_allocated = 1;
    temp->clear(makeRgb(255, 255, 255));
    Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)temp->data, temp->w, temp->h, 4, 0);

    fl_begin_offscreen(offscreen);
    fl_color(FL_WHITE);
    fl_rectf(0, 0, tw, th);
    fl_color(FL_BLACK);
    fl_draw(string, 0, th - 1 - fl_descent());
    fl_read_image((unsigned char *)temp->data, 0, 0, tw, th, 255);

    fl_end_offscreen();
    fl_delete_offscreen(offscreen);
  }

  int x, y;
  Map *map = Map::main;
  int imgx = view->imgx;
  int imgy = view->imgy;

  map->clear(0);

  for(y = 0; y < temp->h; y++)
  {
    for(x = 0; x < temp->w; x++)
    {
      int c = temp->getpixel(x, y);
      int t = getv(c);
      if(t < 128)
        map->setpixel(imgx - temp->w / 2 + x, imgy - temp->h / 2 + y, 255);
    }
  }

  stroke->size(imgx - temp->w / 2, imgy - temp->h / 2, imgx + temp->w / 2, imgy + temp->h / 2);
  redraw(view);
}

void Text::done(View *view)
{
}

void Text::redraw(View *view)
{
  if(active)
  {
    active = 0;
    view->drawMain(0);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
    active = 1;
  }
}

