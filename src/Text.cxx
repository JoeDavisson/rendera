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
#include "FontPreview.H"
#include "Gui.H"
#include "Inline.H"
#include "InputInt.H"
#include "Map.H"
#include "Progress.H"
#include "Project.H"
#include "Stroke.H"
#include "Text.H"
#include "TextOptions.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

//FIXME should free memory when switched to another tool

Text::Text()
{
  preview_text = 0;
  preview_surf = 0;
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

  // write text string to FLTK's offscreen image
  int index = FontPreview::getFont();

  if (index < 1)
    index = 1;

  int face = index - 1;
  int size = Gui::text->getSize();
  int angle = 360 - Gui::text->getAngle();
  const char *s = Gui::text->getInput();

  if (size < 2)
    size = 2;

  if (size > 256)
    size = 256;

  int smooth = Gui::text->getSmooth();
  int weight = Gui::text->getWeight();

  if (smooth > 0 && weight > 0)
  {
    size *= 2;
  }

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

  Bitmap text_final(tsize, tsize);

  Fl_Image_Surface text_surf(tsize, tsize, 0);
  Fl_Surface_Device::push_current(&text_surf);
  fl_font(face, size);
  fl_color(FL_WHITE);
  fl_rectf(0, 0, tsize, tsize);
  fl_color(FL_BLACK);
  fl_draw(angle, string.data(), center + dx, center + dy);
  fl_read_image((unsigned char *)text_final.data, 0, 0, tsize, tsize, 255);
  Fl_Surface_Device::pop_current();

  int w = text_final.w;
  int h = text_final.h;

  // start progress bar
  int yy = h * 2;

  if (weight > 0)
  {
    yy += h;

    if (smooth > 0)
      yy -= h / 2;
  }

  Progress::show(yy);

  Map map(w, h);
  map.clear(0);

  if (weight > 0)
  {
    for (int y = 0; y < h; y++)
    {
      unsigned char *m = map.row[y];
      int *tb = text_final.row[y];

      for (int x = 0; x < w; x++)
      {
         if (getv(*tb++) < 192)
          *m = 1;

        m++;
      }

      if (Progress::update(yy++) < 0)
        break;
    }

    map.dilate(smooth > 0 ? weight * 2 : weight);
  }

  for (int y = 0; y < h; y++)
  {
    unsigned char *m = map.row[y];
    int *tb = text_final.row[y];

    for (int x = 0; x < w; x++)
    {
      if (*m == 1)
        *tb = makeRgb(0, 0, 0);

      m++;
      tb++;
    }

    if (Progress::update(yy++) < 0)
      break;
  }

  if (smooth > 0)
  {
    if (weight > 0)
    {
      Bitmap scaled_bmp(w / 2, h / 2);
      text_final.scale(&scaled_bmp);

      for (int y = 0; y < scaled_bmp.h; y++)
      {
        int *tb = scaled_bmp.row[y];

        for (int x = 0; x < scaled_bmp.w; x++)
        {
          const int t = getv(*tb);

          if (t < 255)
          {
            Project::bmp->setpixel(view->imgx - w / 4 + x,
                                   view->imgy - h / 4 + y,
                                   Project::brush->color,
                                   scaleVal(Project::brush->trans, t));
          }

          tb++;
        }

        if (Progress::update(yy++) < 0)
          break;
      }
    }
      else
    {
      for (int y = 0; y < h; y++)
      {
        int *tb = text_final.row[y];

        for (int x = 0; x < w; x++)
        {
          const int t = getv(*tb);

          if (t < 255)
          {
            Project::bmp->setpixel(view->imgx - w / 2 + x,
                                   view->imgy - h / 2 + y,
                                   Project::brush->color,
                                   scaleVal(Project::brush->trans, t));
          }

          tb++;
        }

        if (Progress::update(yy++) < 0)
          break;
      }
    }
  }
    else
  {
    for (int y = 0; y < h; y++)
    {
      int *tb = text_final.row[y];

      for (int x = 0; x < w; x++)
      {
        const int t = getv(*tb);

        if (t < 192)
        {
          Project::bmp->setpixel(view->imgx - w / 2 + x,
                                 view->imgy - h / 2 + y,
                                 Project::brush->color,
                                 Project::brush->trans);
        }

        tb++;
      }

      if (Progress::update(yy++) < 0)
        break;
    }
  }

  Progress::hide();
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

  // create preview
  int imgx = view->imgx;
  int imgy = view->imgy;
  int w = preview_text->w;
  int h = preview_text->h;

  Map *map = Project::map;
  map->clear(0);

  int x1 = 0;
  int y1 = 0;
  int x2 = w;
  int y2 = h;

  const int left = imgx - w / 2;
  const int top = imgy - h / 2;

  if (left < 0)
    x1 = -left;

  if (left + w >= map->w)
    x2 = map->w - left;

  if (top < 0)
    y1 = -top;

  if (top + h >= map->h)
    y2 = map->h - top;

  // draw
  for (int y = y1; y < y2; y++)
  {
    unsigned char *m = map->row[top + y] + x1 + left;
    int *tb = preview_text->row[y] + x1;

    for (int x = x1; x < x2; x++)
    {
      *m++ = !((*tb++ & 255) >> 7); 
    }
  }

  int weight = Gui::text->getWeight();

  if (weight > 0)
    map->dilate(weight);

  stroke->size(imgx - w / 2, imgy - h / 2, imgx + w / 2, imgy + h / 2);
  redraw(view);
}

void Text::key(View *)
{
}

void Text::done(View *, int)
{
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

void Text::change()
{
  // write text string to FLTK's offscreen image
  int index = FontPreview::getFont();

  if (index < 1)
    index = 1;

  int face = index - 1;
  int size = Gui::text->getSize();
  int angle = 360 - Gui::text->getAngle();
  const char *s = Gui::text->getInput();

  if (size < 4)
    size = 4;

  if (size > 256)
    size = 256;

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

  delete preview_text;
  preview_text = new Bitmap(tsize, tsize);

  delete preview_surf;
  preview_surf = new Fl_Image_Surface(tsize, tsize, 0);
  Fl_Surface_Device::push_current(preview_surf);
  fl_font(face, size);
  fl_color(FL_WHITE);
  fl_rectf(0, 0, tsize, tsize);
  fl_color(FL_BLACK);
  fl_draw(angle, string.data(), center + dx, center + dy);
  fl_read_image((unsigned char *)preview_text->data, 0, 0, tsize, tsize, 255);
  Fl_Surface_Device::pop_current();
}

