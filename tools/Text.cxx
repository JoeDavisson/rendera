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

#include <FL/fl_draw.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Clone.H"
#include "Gui.H"
#include "Inline.H"
#include "Map.H"
#include "Project.H"
#include "Stroke.H"
#include "Text.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"

namespace
{
  int state = 0;
  Bitmap *temp = 0;
}

Text::Text()
{
}

Text::~Text()
{
}

void Text::push(View *view)
{
  Stroke *stroke = Project::stroke.get();

  if(state == 0)
    move(view);

  Undo::push(stroke->x1,
             stroke->y1,
             (stroke->x2 - stroke->x1) + 1,
             (stroke->y2 - stroke->y1) + 1);

  Clone::move(view->imgx, view->imgy);
  Clone::refresh(stroke->x1, stroke->y1, stroke->x2, stroke->y2);

  // render text to image
  Blend::set(Project::brush->blend);

  if(Gui::getTextSmooth() > 0)
  {
    for(int y = 0; y < temp->h; y++)
    {
      for(int x = 0; x < temp->w; x++)
      {
        int c = temp->getpixel(x, y);
        int t = getv(c);

        if(t < 255)
        {
          Project::bmp->setpixel(view->imgx - temp->w / 2 + x,
                                 view->imgy - temp->h / 2 + y,
                                 Project::brush->color,
                                 scaleVal(Project::brush->trans, t));
        }
      }
    }
  }
  else
  {
    for(int y = 0; y < temp->h; y++)
    {
      for(int x = 0; x < temp->w; x++)
      {
        int c = temp->getpixel(x, y);
        int t = getv(c);

        if(t < 192)
        {
          Project::bmp->setpixel(view->imgx - temp->w / 2 + x,
                                 view->imgy - temp->h / 2 + y,
                                 Project::brush->color,
                                 Project::brush->trans);
        }
      }
    }
  }

  if(temp)
  {
    delete temp;
    temp = 0;
  }

  Blend::set(Blend::TRANS);

  state = 0;
  view->drawMain(true);
}

void Text::drag(View*)
{
}

void Text::release(View*)
{
}

void Text::move(View *view)
{
  Stroke *stroke = Project::stroke.get();

  // write text string to FLTK's offscreen image
  if(state == 0)
  {
    int face = Gui::getFontFace();
    int size = Gui::getFontSize();
    const char *s = Gui::getTextInput();
    if(strlen(s) > 250)
      return;

    state = 1;

    // add a space before and after string, or some
    // scripty fonts won't render propery on the sides
    char string[256];
    string[0] = ' ';

    unsigned int i = 0;

    for( ; i <= strlen(s); i++)
      string[i + 1] = s[i];

    string[i++] = ' ';
    string[i] = '\0';

    fl_font(face, size);

    // measure size of image we need
    int tw = 0, th = 0;
    fl_measure(string, tw, th, 1);

    // draw text string to offscreen image
    Fl_Offscreen offscreen = fl_create_offscreen(tw, th);

    if(temp)
    {
      delete temp;
      temp = 0;
    }

    temp = new Bitmap(tw, th);
    temp->clear(makeRgb(255, 255, 255));

    fl_begin_offscreen(offscreen);
    fl_color(FL_WHITE);
    fl_rectf(0, 0, tw, th);
    fl_color(FL_BLACK);
    fl_draw(string, 0, th - 1 - fl_descent());

    fl_read_image((unsigned char *)temp->data, 0, 0, tw, th, 255);

    fl_end_offscreen();
    fl_delete_offscreen(offscreen);
  }

  // create preview map
  int imgx = view->imgx;
  int imgy = view->imgy;

  Map *map = Project::map;
  map->clear(0);

  for(int y = 0; y < temp->h; y++)
  {
    for(int x = 0; x < temp->w; x++)
    {
      int c = temp->getpixel(x, y);
      int t = getv(c);

      if(t < 192)
        map->setpixel(imgx - temp->w / 2 + x, imgy - temp->h / 2 + y, 255);
    }
  }

  stroke->size(imgx - temp->w / 2, imgy - temp->h / 2,
               imgx + temp->w / 2, imgy + temp->h / 2);
  redraw(view);
}

void Text::done(View*)
{
  if(temp)
  {
    delete temp;
    temp = 0;
  }
}

void Text::redraw(View *view)
{
  Stroke *stroke = Project::stroke.get();

  view->drawMain(false);
  stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();
}

bool Text::isActive()
{
  return false;
}

void Text::reset()
{
  state = 0;
}

