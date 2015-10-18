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

#include <algorithm>
#include <cstdlib>

#include "Bitmap.H"
#include "Selection.H"
#include "Gui.H"
#include "Map.H"
#include "ExtraMath.H"
#include "Project.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"


namespace
{
  int beginx = 0, beginy = 0, oldx = 0, oldy = 0, lastx = 0, lasty = 0;
  int state = 0;
  bool active = false;
  bool drag_started = false;
  bool resize_started = false;
  int side = 0;
  int offset = 0;

  bool inbox(int x, int y, int x1, int y1, int x2, int y2)
  {
    if(x1 > x2)
      std::swap(x1, x2);
    if(y1 > y2)
      std::swap(y1, y2);

    if(x >= x1 && x <= x2 && y >= y1 && y <= y2)
      return 1;
    else
      return 0;
  }

  void absrect(int *x1, int *y1, int *x2, int *y2)
  {
    if(*x1 > *x2)
      std::swap(*x1, *x2);
    if(*y1 > *y2)
      std::swap(*y1, *y2);

    if(*x1 < Project::bmp->cl)
      *x1 = Project::bmp->cl;
    if(*y1 < Project::bmp->ct)
      *y1 = Project::bmp->ct;
    if(*x2 > Project::bmp->cr)
      *x2 = Project::bmp->cr;
    if(*y2 > Project::bmp->cb)
      *y2 = Project::bmp->cb;
  }

  void drawHandles(Stroke *stroke, int x1, int y1, int x2, int y2, int color)
  {
    absrect(&x1, &y1, &x2, &y2);
    Project::map->rect(x1, y1, x2, y2, color);
    stroke->size(x1, y1, x2, y2);
  }

  void crop(View *view)
  {
    if(state == 3)
      return;

    Undo::push();
    state = 0;
    active = false;
    absrect(&beginx, &beginy, &lastx, &lasty);

    int w = (lastx - beginx) + 1;
    int h = (lasty - beginy) + 1;

    if(w < 1)
      w = 1;
    if(h < 1)
      h = 1;

    Bitmap *temp = new Bitmap(w, h);
    Project::bmp->blit(temp, beginx, beginy, 0, 0, w, h);

    Project::newImage(w, h);
    temp->blit(Project::bmp, 0, 0,
               Project::overscroll, Project::overscroll, w, h);
    delete temp;

    view->zoom = 1;
    view->ox = 0;
    view->oy = 0;
    view->drawMain(true);
    Gui::checkSelectionValues(0, 0, 0, 0);
  }

  void duplicate(View *)
  {
    if(state == 3)
      return;

    state = 3;

    int w = (lastx - beginx) + 1;
    int h = (lasty - beginy) + 1;

    if(w < 1)
      w = 1;
    if(h < 1)
      h = 1;

    delete(Project::brush_bmp);
    Project::brush_bmp = new Bitmap(w, h);
    Project::bmp->blit(Project::brush_bmp, beginx, beginy, 0, 0, w, h);

    Gui::checkSelectionValues(0, 0, 0, 0);
  }
}

Selection::Selection()
{
}

Selection::~Selection()
{
}

void Selection::push(View *view)
{
  if(state == 0)
  {
    Project::map->clear(0);
    beginx = view->imgx;
    beginy = view->imgy;
    lastx = view->imgx;
    lasty = view->imgy;
    state = 1;
    active = true;
  }
  else if(state == 2)
  {
    if(!drag_started && !resize_started)
    {
      if(inbox(view->imgx, view->imgy, beginx, beginy, lastx, lasty))
      {
        drag_started = true;
      }
      else
      {
        if(view->imgx < beginx)
        {
          side = 0;
          offset = ExtraMath::abs(view->imgx - beginx);
          resize_started = true;
        }
        else if(view->imgx > lastx)
        {
          side = 1;
          offset = ExtraMath::abs(view->imgx - lastx);
          resize_started = true;
        }
        else if(view->imgy < beginy)
        {
          side = 2;
          offset = ExtraMath::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgy > lasty)
        {
          side = 3;
          offset = ExtraMath::abs(view->imgy - lasty);
          resize_started = true;
        }

        resize_started = true;
      }
    }
  }
  else if(state == 3)
  {
    Undo::push();
    const int w = Project::brush_bmp->w;
    const int h = Project::brush_bmp->h;

    Project::brush_bmp->drawBrush(Project::bmp,
               0, 0, view->imgx - w / 2, view->imgy - h / 2, w, h);

    view->ignore_tool = true;
    view->drawMain(true);
  }
}

void Selection::drag(View *view)
{
  Stroke *stroke = Project::stroke.get();

  if(state == 1)
  {
    drawHandles(stroke, beginx, beginy, lastx, lasty, 0);
    drawHandles(stroke, beginx, beginy, view->imgx, view->imgy, 255);

    lastx = view->imgx;
    lasty = view->imgy;

    view->drawMain(false);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();

    redraw(view);
  }
  else if(state == 2)
  {
    drawHandles(stroke, beginx, beginy, lastx, lasty, 0);

    if(drag_started)
    {
      const int dx = view->imgx - view->oldimgx;
      const int dy = view->imgy - view->oldimgy;

      const int cl = Project::bmp->cl;
      const int cr = Project::bmp->cr;
      const int ct = Project::bmp->ct;
      const int cb = Project::bmp->cb;

      if( (beginx + dx >= cl) && (beginx + dx <= cr) &&
          (beginy + dy >= ct) && (beginy + dy <= cb) &&
          (lastx + dx >= cl) && (lastx + dx <= cr) &&
          (lasty + dy >= ct) && (lasty + dy <= cb) )
      {
        beginx += dx;
        beginy += dy;
        lastx += dx;
        lasty += dy;
      }
    }
    else if(resize_started)
    {
      switch(side)
      {
        case 0:
          beginx = view->imgx + offset;
          break;
        case 1:
          lastx = view->imgx - offset;
          break;
        case 2:
          beginy = view->imgy + offset;
          break;
        case 3:
          lasty = view->imgy - offset;
          break;
      }
    }

    redraw(view);
  }
  else if(state == 3)
  {
    Undo::push();
    const int w = Project::brush_bmp->w;
    const int h = Project::brush_bmp->h;

    Project::brush_bmp->drawBrush(Project::bmp,
               0, 0, view->imgx - w / 2, view->imgy - h / 2, w, h);
    view->ignore_tool = true;
    view->drawMain(true);
  }

  const int overscroll = Project::overscroll;
  const int x = beginx - overscroll;
  const int y = beginy - overscroll;
  const int w = abs(lastx - beginx) + 1;
  const int h = abs(lasty - beginy) + 1;

  Gui::checkSelectionValues(x, y, w, h);
}

void Selection::release(View *view)
{
  if(state == 1)
    state = 2;

  drag_started = false;
  resize_started = false;
  absrect(&beginx, &beginy, &lastx, &lasty);

  redraw(view);

  const int overscroll = Project::overscroll;
  const int x = beginx - overscroll;
  const int y = beginy - overscroll;
  const int w = abs(lastx - beginx) + 1;
  const int h = abs(lasty - beginy) + 1;

  Gui::checkSelectionValues(x, y, w, h);
}

void Selection::move(View *view)
{
  Stroke *stroke = Project::stroke.get();

  if(state == 3)
  {
    const int w = Project::brush_bmp->w;
    const int h = Project::brush_bmp->h;

    const int x1 = view->imgx - w / 2;
    const int y1 = view->imgy - h / 2;
    const int x2 = x1 + w - 1;
    const int y2 = y1 + h - 1;

    stroke->size(x1, y1, x2, y2);

    view->drawMain(false);
    stroke->previewBrush(view->backbuf, view->ox, view->oy, view->zoom,
                         view->bgr_order);
    view->ignore_tool = true;
    view->redraw();
  }
}

void Selection::done(View *view, int mode)
{
  if(state == 0)
    return;

  switch(mode)
  {
    case 0:
      crop(view);
      break;
    case 1:
      duplicate(view);
      break;
    case 2:
      reset();
      break;
  }
}

void Selection::redraw(View *view)
{
  if(state == 3)
  {
    move(view);
    return;
  }

  Stroke *stroke = Project::stroke.get();

  if(active)
  {
    active = false;
    drawHandles(stroke, beginx, beginy, lastx, lasty, 255);
    view->drawMain(false);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
    active = true;
  }
}

bool Selection::isActive()
{
  return active;
}

void Selection::reset()
{
  active = false;
  state = 0;
  Gui::checkSelectionValues(0, 0, 0, 0);
  Gui::getView()->drawMain(true);
}

