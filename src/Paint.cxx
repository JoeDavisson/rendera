/*
Copyright (c) 2024 Joe Davisson.

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

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Clone.H"
#include "Gui.H"
#include "Map.H"
#include "Paint.H"
#include "Project.H"
#include "Render.H"
#include "Stroke.H"
#include "View.H"

Paint::Paint()
{
  active = 0;
}

Paint::~Paint()
{
}

void Paint::push(View *view)
{
  Stroke *stroke = Project::stroke;

  if (active && stroke->type == 3)
  {
    if (view->dclick || view->button3)
    {
      stroke->end(view->imgx, view->imgy);
      Blend::set(Project::brush->blend);
      Render::begin();
      active = false;
      Blend::set(Blend::TRANS);
      view->drawMain(true);
      return;
    }
    else if (view->button1)
    {
      stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    }
  }
  else if (view->button1)
  {
    stroke->begin(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    Clone::state = Clone::STARTED;
    active = true;
  }
}

void Paint::drag(View *view)
{
  Stroke *stroke = Project::stroke;

  if (stroke->type != 3)
  {
    stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    view->drawMain(false);
    stroke->previewPaint(view);

    view->redraw();
  }
}

void Paint::release(View *view)
{
  Stroke *stroke = Project::stroke;

  if (active && stroke->type != 3)
  {
    stroke->end(view->imgx, view->imgy);
    Blend::set(Project::brush->blend);
    Render::begin();
    active = false;
    Blend::set(Blend::TRANS);
  }

  Project::map->clear(0);
  view->drawMain(true);
}

void Paint::move(View *view)
{
  Stroke *stroke = Project::stroke;

  view->drawMain(false);

  switch (stroke->type)
  {
    case 3:
      if (active)
      {
        stroke->polyLine(view->imgx, view->imgy,
                         view->ox, view->oy, view->zoom);
      }
        
      stroke->previewPaint(view);
      break;
    case 0:
    case 2:
    case 4:
    case 6:
      Project::map->rectfill(view->oldimgx - 48, view->oldimgy - 48,
                          view->oldimgx + 48, view->oldimgy + 48, 0);
      Project::map->rectfill(view->imgx - 48, view->imgy - 48,
                          view->imgx + 48, view->imgy + 48, 0);
      stroke->drawBrush(view->imgx, view->imgy, 255);
      stroke->size(view->imgx - 48, view->imgy - 48,
                   view->imgx + 48, view->imgy + 48);
      stroke->makeBlitRect(stroke->x1, stroke->y1,
                           stroke->x2, stroke->y2,
                           view->ox, view->oy, 96, view->zoom);
      stroke->previewPaint(view);
      break;
  }

  view->redraw();
}

void Paint::key(View *)
{
}

void Paint::done(View *, int)
{
}

void Paint::redraw(View *view)
{
  Stroke *stroke = Project::stroke;

  if (active)
  {
    active = false;
    view->drawMain(false);
    stroke->previewPaint(view);
    view->redraw();
    active = true;
  }
    else
  {
    view->drawMain(false);
    view->redraw();
  }

}

bool Paint::isActive()
{
  return active;
}

void Paint::reset()
{
  active = false;
}

