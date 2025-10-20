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

#include "Bitmap.H"
#include "Brush.H"
#include "Gui.H"
#include "Inline.H"
#include "Gradient.H"
#include "GradientOptions.H"
#include "Map.H"
#include "Project.H"
#include "Stroke.H"
#include "Undo.H"
#include "View.H"

namespace
{
  int beginx;
  int beginy;
  int endx;
  int endy;

  bool started = false;
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
  Bitmap *bmp = Project::bmp;

  const int style = Gui::gradient->style();
  const bool use_color = Gui::gradient->useColor();
  const bool inverse = Gui::gradient->inverse();
  const int color = Project::brush->color;
  const int trans = Project::brush->trans;

  switch (style)
  {
    case 0:
      bmp->gradientLinear(beginx, beginy, endx, endy,
                          color, trans, use_color, inverse);
      break;
    case 1:
      bmp->gradientRadial(beginx, beginy, endx, endy,
                          color, trans, use_color, inverse);
      break;
    case 2:
//      bmp->gradientRectangular(beginx, beginy, endx, endy,
//                          color, trans, use_color, inverse);
      break;
    case 3:
      bmp->gradientElliptical(beginx, beginy, endx, endy,
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

