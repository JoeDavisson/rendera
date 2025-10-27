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

#include <algorithm>
#include <cmath>

#include "Blend.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Gui.H"
#include "Inline.H"
#include "Gradient.H"
#include "GradientOptions.H"
#include "Map.H"
#include "Progress.H"
#include "Project.H"
#include "Stroke.H"
#include "Undo.H"
#include "View.H"

//FIXME add rectangular gradient

namespace
{
  int beginx;
  int beginy;
  int endx;
  int endy;

  bool started = false;

  void gradientLinear(int x1, int y1, int x2, int y2,
                              int color, int trans,
                              bool use_color, bool inverse)
  {
    Bitmap *bmp = Project::bmp;

    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float length = dx * dx + dy * dy;

    if (length <= 0)
      return;

    Blend::set(Gui::gradient->blendingMode());
    Progress::show(bmp->h);
    int yy = 0;

    for (int y = 0; y < bmp->h; y++)
    {
      for (int x = 0; x < bmp->w; x++)
      {
        float t = (dx * (x - x1) + dy * (y - y1)) / length;

        if (t < 0)
          t = 0;

        if (t > 1.0)
          t = 1.0;

        if (inverse == true)
          t = 1.0 - t;

        const int c = bmp->getpixel(x, y);

        if (use_color == true)
        {
          bmp->setpixel(x, y,
                        Blend::current(c, color, scaleVal(trans, t * 255)));
        }
          else
        {
          const int r = getr(c);
          const int g = getg(c);
          const int b = getb(c);
          const int a = geta(c) - geta(c) * t;

          bmp->setpixel(x, y, makeRgba(r, g, b, a));
        }
      }

      if (Progress::update(yy++) < 0)
        break;
    }

    Progress::hide();
    Blend::set(Blend::TRANS);
  }

  void gradientRadial(int x1, int y1, int x2, int y2,
                              int color, int trans,
                              bool use_color, bool inverse)
  {
    Bitmap *bmp = Project::bmp;

    const float dx = std::abs(x2 - x1);
    const float dy = std::abs(y2 - y1);
    const float length = (dx * dx + dy * dy);

    if (length <= 0)
      return;

    Blend::set(Gui::gradient->blendingMode());
    Progress::show(bmp->h);
    int yy = 0;

    for (int y = 0; y < bmp->h; y++)
    {
      for (int x = 0; x < bmp->w; x++)
      {
        float t = ((x - x1) * (x - x1) + (y - y1) * (y - y1)) / length;

        if (t < 0)
          t = 0;

        if (t > 1)
          t = 1;

        if (inverse == true)
          t = 1.0 - t;

        const int c = bmp->getpixel(x, y);

        if (use_color == true)
        {
          bmp->setpixel(x, y,
                        Blend::current(c, color, scaleVal(trans, t * 255)));
        }
          else
        {
          const int r = getr(c);
          const int g = getg(c);
          const int b = getb(c);
          const int a = geta(c) - geta(c) * t;

          bmp->setpixel(x, y, makeRgba(r, g, b, a));
        }
      }

      if (Progress::update(yy++) < 0)
        break;
    }

    Progress::hide();
    Blend::set(Blend::TRANS);
  }

  void gradientElliptical(int x1, int y1, int x2, int y2,
                                  int color, int trans,
                                  bool use_color, bool inverse)
  {
    Bitmap *bmp = Project::bmp;

    if (x1 > x2)
      std::swap(x1, x2);

    if (y1 > y2)
      std::swap(y1, y2);

    const float dx = x2 - x1;
    const float dy = y2 - y1;

    if (dx == 0 || dy == 0)
      return;

    const float rx = dx / 2;
    const float ry = dy / 2;
    const float cx = x1 + rx;
    const float cy = y1 + ry;

    Blend::set(Gui::gradient->blendingMode());
    Progress::show(bmp->h);
    int yy = 0;

    for (int y = 0; y < bmp->h; y++)
    {
      for (int x = 0; x < bmp->w; x++)
      {
        float t = ((x - cx) * (x - cx) / (rx * rx)) +
                  ((y - cy) * (y - cy) / (ry * ry));

        if (t < 0)
          t = 0;

        if (t > 1)
          t = 1;

        if (inverse == true)
          t = 1.0 - t;

        const int c = bmp->getpixel(x, y);

        if (use_color == true)
        {
          bmp->setpixel(x, y,
                        Blend::current(c, color, scaleVal(trans, t * 255)));
        }
          else
        {
          const int r = getr(c);
          const int g = getg(c);
          const int b = getb(c);
          const int a = geta(c) - geta(c) * t;

          bmp->setpixel(x, y, makeRgba(r, g, b, a));
        }
      }

      if (Progress::update(yy++) < 0)
        break;
    }

    Progress::hide();
    Blend::set(Blend::TRANS);
  }
}

Gradient::Gradient()
{
}

Gradient::~Gradient()
{
}

void Gradient::push(View *view)
{
  if (!view->button1)
    return;

  beginx = view->imgx;
  beginy = view->imgy;
  endx = view->imgx;
  endy = view->imgy;

  started = true;
}

void Gradient::drag(View *view)
{
  Bitmap *backbuf = view->backbuf;

  float zoom = view->zoom;
  int ox = view->ox * zoom;
  int oy = view->oy * zoom;

  view->drawMain(false);

  endx = view->imgx;
  endy = view->imgy;

  int x1 = beginx * zoom - ox;
  int y1 = beginy * zoom - oy;
  int x2 = endx * zoom - ox;
  int y2 = endy * zoom - oy;

  const int style = Gui::gradient->style();

  if (style < 2)
  {
    backbuf->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 0, 3);
    backbuf->rectfill(x2 - 8, y2 - 8, x2 + 8, y2 + 8, makeRgb(0, 0, 0), 0);

    backbuf->line(x1, y1, x2, y2, makeRgb(255, 255, 255), 0, 1);
    backbuf->rectfill(x2 - 5, y2 - 5, x2 + 5, y2 + 5, makeRgb(255, 255, 255), 0);
  }
    else
  {
    backbuf->rect(x1, y1, x2, y2, makeRgb(0, 0, 0), 0);
    backbuf->rect(x1 + 1, y1 + 1, x2 - 1, y2 - 1, makeRgb(255, 255, 255), 0);
    backbuf->rect(x1 + 2, y1 + 2, x2 - 2, y2 - 2, makeRgb(0, 0, 0), 0);
  }

  view->redraw();
}

void Gradient::release(View *view)
{
  if (!started)
    return;

  started = false;

  Project::undo->push();

  const int style = Gui::gradient->style();
  const bool use_color = Gui::gradient->useColor();
  const bool inverse = Gui::gradient->inverse();
  const int color = Project::brush->color;
  const int trans = Project::brush->trans;

  switch (style)
  {
    case 0:
      gradientLinear(beginx, beginy, endx, endy,
                     color, trans, use_color, inverse);
      break;
    case 1:
      gradientRadial(beginx, beginy, endx, endy,
                     color, trans, use_color, inverse);
      break;
    case 2:
//    gradientRectangular(beginx, beginy, endx, endy,
//                        color, trans, use_color, inverse);
      break;
    case 3:
      gradientElliptical(beginx, beginy, endx, endy,
                         color, trans, use_color, inverse);
      break;
  }

  view->drawMain(true);
}

void Gradient::move(View *)
{
}

void Gradient::key(View *)
{
}

void Gradient::done(View *, int)
{
}

void Gradient::redraw(View *view)
{
  view->drawMain(true);
}

bool Gradient::isActive()
{
  return false;
}

void Gradient::reset()
{
}

