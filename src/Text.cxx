/*
Copyright (c) 2025 Joe Davisson.

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

#include <cmath>
#include <vector>

#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Image_Surface.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "CheckBox.H"
#include "Clone.H"
#include "Gui.H"
#include "Inline.H"
#include "InputInt.H"
#include "Map.H"
#include "Project.H"
#include "Stroke.H"
#include "Text.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

Text::Text()
{
  textbmp = 0;
}

Text::~Text()
{
}

void Text::push(View *view)
{
  Stroke *stroke = Project::stroke;

  move(view);

  Project::undo->push();

  Clone::move(view->imgx, view->imgy);
  Clone::refresh(stroke->x1, stroke->y1, stroke->x2, stroke->y2);

  // render text to image
  Blend::set(Project::brush->blend);

  int w = textbmp->w;
  int h = textbmp->h;

  if (Gui::text_smooth->value() > 0)
  {
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        int c = textbmp->getpixel(x, y);
        int t = getv(c);

        if (t < 255)
        {
          Project::bmp->setpixel(view->imgx - w / 2 + x,
                                 view->imgy - h / 2 + y,
                                 Project::brush->color,
                                 scaleVal(Project::brush->trans, t));
        }
      }
    }
  }
    else
  {
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        int c = textbmp->getpixel(x, y);
        int t = getv(c);

        if (t < 192)
        {
          Project::bmp->setpixel(view->imgx - w / 2 + x,
                                 view->imgy - h / 2 + y,
                                 Project::brush->color,
                                 Project::brush->trans);
        }
      }
    }
  }

  delete textbmp;
  textbmp = 0;

  Blend::set(Blend::TRANS);

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
  Stroke *stroke = Project::stroke;

  // write text string to FLTK's offscreen image
  int index = Gui::font_browse->value();

  if (index < 1)
    index = 1;

  int face =  index - 1;

  int size = atoi(Gui::font_size->value());
  int angle = 360 - atoi(Gui::font_angle->value());
  const char *s = Gui::text_input->value();

  if (size < 4)
    size = 4;

  // add a space before and after string, or some
  // scripty fonts won't render properly on the sides
  std::vector<char> string((int)strlen(s) + 8, 0);

  string[0] = ' ';

  int i = 0;

  for (i = 0; i < (int)strlen(s); i++)
    string[i + 1] = s[i];

  i++;

  string[i] = ' ';

  // compensate for odd string length
  if ((strlen(s) & 1) == 1)
    string[i++] = ' ';
  else
    i++;

  string[i] = '\0';

  // measurement/rotation
  fl_font(face, size);

  int tw = 0, th = 0;
  fl_measure(string.data(), tw, th);

  int tsize = tw > th ? tw : th;
  tsize *= std::sqrt(2);

  const float center = (float)tsize / 2;
  const float d = (M_PI * (float)angle) / 180;

  float dx = -std::cos(d) * ((float)tw / 2);
  float dy = std::sin(d) * ((float)tw / 2);

  delete textbmp;
  textbmp = new Bitmap(tsize, tsize);

  Fl_RGB_Image textbuf((unsigned char *)&textbmp->data, tsize, tsize, 4, 0);
  Fl_Image_Surface surf(tsize, tsize, 1);
  Fl_Surface_Device::push_current(&surf);
  fl_font(face, size);
  fl_color(FL_WHITE);
  fl_rectf(0, 0, tsize, tsize);
  fl_color(FL_BLACK);
  fl_draw(angle, string.data(), center + dx, center + dy);
  fl_read_image((unsigned char *)textbmp->data, 0, 0, tsize, tsize, 255);
  Fl_Surface_Device::pop_current();

  // create preview
  int imgx = view->imgx;
  int imgy = view->imgy;

  int w = textbmp->w;
  int h = textbmp->h;

  Map *map = Project::map;
  map->clear(0);

  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      int c = textbmp->getpixel(x, y);
      int t = getv(c);

      if (t < 192)
        map->setpixel(imgx - w / 2 + x,
                      imgy - h / 2 + y, 255);
    }
  }

  stroke->size(imgx - w / 2, imgy - h / 2,
               imgx + w / 2, imgy + h / 2);
  redraw(view);
}

void Text::key(View *)
{
}

void Text::done(View *, int)
{
  delete textbmp;
  textbmp = 0;
}

void Text::redraw(View *view)
{
  Stroke *stroke = Project::stroke;

  view->drawMain(false);
  stroke->previewPaint(view);
  view->redraw();
}

bool Text::isActive()
{
  return false;
}

void Text::reset()
{
}

