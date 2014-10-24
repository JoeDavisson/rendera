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
#include "Project.H"

namespace
{
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
  if(state == 0)
    move(view);

  Undo::push(stroke->x1,
             stroke->y1,
             (stroke->x2 - stroke->x1) + 1,
             (stroke->y2 - stroke->y1) + 1, 0);

  // render text to image
  Bitmap *dest = Project::bmp;
  Blend::set(Project::brush->blend);

  int x, y;

  for(y = 0; y < temp->h; y++)
  {
    for(x = 0; x < temp->w; x++)
    {
      int c = temp->getpixel(x, y);
      int t = getv(c);

      if(t < 255)
      {
        dest->setpixel(view->imgx - temp->w / 2 + x,
                       view->imgy - temp->h / 2 + y,
                       Project::brush->color,
                       scaleVal(Project::brush->trans, t));
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

void Text::drag(View *view)
{
}

void Text::release(View *)
{
}

void Text::move(View *view)
{
  // write text string to FLTK's offscreen image
  if(state == 0)
  {
    int i;

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
    for(i = 0; i <= strlen(s); i++)
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

  // create preview map
  int x, y;
  int imgx = view->imgx;
  int imgy = view->imgy;

  Map *map = Project::map;
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

  stroke->size(imgx - temp->w / 2, imgy - temp->h / 2,
               imgx + temp->w / 2, imgy + temp->h / 2);
  redraw(view);
}

void Text::done(View *view)
{
}

void Text::redraw(View *view)
{
  view->drawMain(false);
  stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();
}

