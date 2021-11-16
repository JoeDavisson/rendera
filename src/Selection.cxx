/*
Copyright (c) 2021 Joe Davisson.

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

#include <FL/Fl_Window.H>

#include "Blend.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Gui.H"
#include "Map.H"
#include "ExtraMath.H"
#include "Inline.H"
#include "Project.H"
#include "Selection.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"


namespace
{
  int beginx = 0, beginy = 0, lastx = 0, lasty = 0;
  int state = 0;
  bool active = false;
  bool drag_started = false;
  bool resize_started = false;
  int side = 0;
  int offsetx = 0;
  int offsety = 0;

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

  void absrect(View *view, int *x1, int *y1, int *x2, int *y2)
  {
    if(*x1 > *x2)
      std::swap(*x1, *x2);
    if(*y1 > *y2)
      std::swap(*y1, *y2);

    const int gridx = view->gridx;
    const int gridy = view->gridy;
    const int overscroll = Project::overscroll;

    if(view->gridsnap)
    {
      *x1 -= overscroll;
      *x1 -= *x1 % gridx;
      *x1 += overscroll;

      *y1 -= overscroll;
      *y1 -= *y1 % gridy;
      *y1 += overscroll;

      *x2 -= overscroll;
      *x2 += 1;
      *x2 -= *x2 % gridx;
      *x2 += overscroll - 1;

      *y2 -= overscroll;
      *y2 += 1;
      *y2 -= *y2 % gridy;
      *y2 += overscroll - 1;
    }

    if(*x1 < Project::bmp->cl)
      *x1 = Project::bmp->cl;
    if(*y1 < Project::bmp->ct)
      *y1 = Project::bmp->ct;
    if(*x2 > Project::bmp->cr)
      *x2 = Project::bmp->cr;
    if(*y2 > Project::bmp->cb)
      *y2 = Project::bmp->cb;
  }

  void drawHandles(View *view, Stroke *stroke, int x1, int y1, int x2, int y2)
  {
    int d = 8;
    int s = 16;

    Bitmap *backbuf = view->backbuf;
    float zoom = view->zoom;
    int ox = view->ox * zoom;
    int oy = view->oy * zoom;

    absrect(view, &x1, &y1, &x2, &y2);
    stroke->size(x1, y1, x2, y2);

    x1 = x1 * zoom - ox;
    y1 = y1 * zoom - oy;
    x2 = x2 * zoom - ox + zoom;
    y2 = y2 * zoom - oy + zoom;

    int c1 = makeRgb(0, 0, 0);

    backbuf->xorHline(x1 - s - 1, y1 - 1, x1 - d - 1);
    backbuf->hline(x1 - s - 1, y1 - 2, x1 - d - 1, c1, 128);
    backbuf->hline(x1 - s - 1, y1 - 3, x1 - d - 1, c1, 192);
    backbuf->xorVline(y1 - s - 1, x1 - 1, y1 - d - 1);
    backbuf->vline(y1 - s - 1, x1 - 2, y1 - d - 1, c1, 128);
    backbuf->vline(y1 - s - 1, x1 - 3, y1 - d - 1, c1, 192);

    backbuf->xorHline(x2 + d, y1 - 1, x2 + s);
    backbuf->hline(x2 + d, y1 - 2, x2 + s, c1, 128);
    backbuf->hline(x2 + d, y1 - 3, x2 + s, c1, 192);
    backbuf->xorVline(y1 - s - 1, x2 + 1, y1 - d - 1);
    backbuf->vline(y1 - s - 1, x2 + 2, y1 - d - 1, c1, 128);
    backbuf->vline(y1 - s - 1, x2 + 3, y1 - d - 1, c1, 192);

    backbuf->xorHline(x1 - s - 1, y2 + 1, x1 - d - 1);
    backbuf->hline(x1 - s - 1, y2 + 2, x1 - d - 1, c1, 128);
    backbuf->hline(x1 - s - 1, y2 + 3, x1 - d - 1, c1, 192);
    backbuf->xorVline(y2 + d + 1, x1 - 1, y2 + s + 1);
    backbuf->vline(y2 + d + 1, x1 - 2, y2 + s + 1, c1, 128);
    backbuf->vline(y2 + d + 1, x1 - 3, y2 + s + 1, c1, 192);

    backbuf->xorHline(x2 + d + 1, y2 + 1, x2 + s + 1);
    backbuf->hline(x2 + d + 1, y2 + 2, x2 + s + 1, c1, 128);
    backbuf->hline(x2 + d + 1, y2 + 3, x2 + s + 1, c1, 192);
    backbuf->xorVline(y2 + d + 1, x2 + 1, y2 + s + 1);
    backbuf->vline(y2 + d + 1, x2 + 2, y2 + s + 1, c1, 128);
    backbuf->vline(y2 + d + 1, x2 + 3, y2 + s + 1, c1, 192);

    backbuf->xorRect(x1 - 1, y1 - 1, x2 + 1, y2 + 1);
    backbuf->rect(x1 - 2, y1 - 2, x2 + 2, y2 + 2, c1, 128);
    backbuf->rect(x1 - 3, y1 - 3, x2 + 3, y2 + 3, c1, 192);
//    backbuf->xorRect(x1 - 2, y1 - 2, x2 + 2, y2 + 2);
  }

  void crop(View *view)
  {
    if(state == 3)
      return;

    Undo::push();
    state = 0;
    active = false;
    absrect(view, &beginx, &beginy, &lastx, &lasty);

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

  void select(View *)
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

    delete(Project::select_bmp);
    Project::select_bmp = new Bitmap(w, h);
    Project::bmp->blit(Project::select_bmp, beginx, beginy, 0, 0, w, h);

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
  if(!view->button1)
    return;

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
        // corners
        if(view->imgx < beginx && view->imgy < beginy)
        {
          side = 4;
          offsetx = ExtraMath::abs(view->imgx - beginx);
          offsety = ExtraMath::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgx > lastx && view->imgy < beginy)
        {
          side = 5;
          offsetx = ExtraMath::abs(view->imgx - lastx);
          offsety = ExtraMath::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgx < beginx && view->imgy > lasty)
        {
          side = 6;
          offsetx = ExtraMath::abs(view->imgx - beginx);
          offsety = ExtraMath::abs(view->imgy - lasty);
          resize_started = true;
        }
        else if(view->imgx > lastx && view->imgy > lasty)
        {
          side = 7;
          offsetx = ExtraMath::abs(view->imgx - lastx);
          offsety = ExtraMath::abs(view->imgy - lasty);
          resize_started = true;
        }

        // sides
        else if(view->imgx < beginx)
        {
          side = 0;
          offsetx = ExtraMath::abs(view->imgx - beginx);
          resize_started = true;
        }
        else if(view->imgx > lastx)
        {
          side = 1;
          offsetx = ExtraMath::abs(view->imgx - lastx);
          resize_started = true;
        }
        else if(view->imgy < beginy)
        {
          side = 2;
          offsety = ExtraMath::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgy > lasty)
        {
          side = 3;
          offsety = ExtraMath::abs(view->imgy - lasty);
          resize_started = true;
        }

        resize_started = true;
      }
    }
  }
  else if(state == 3)
  {
    Undo::push();
    const int w = Project::select_bmp->w;
    const int h = Project::select_bmp->h;

    int x1 = view->imgx - w / 2;
    int y1 = view->imgy - h / 2;

    if(view->gridsnap)
    {
      x1 -= Project::overscroll;
      x1 -= x1 % view->gridx;
      x1 += Project::overscroll;

      y1 -= Project::overscroll;
      y1 -= y1 % view->gridy;
      y1 += Project::overscroll;
    }

    Project::select_bmp->blit(Project::bmp,
               0, 0, x1, y1, w, h);

    Blend::set(Blend::TRANS);
    view->ignore_tool = true;
    view->drawMain(true);
  }
}

void Selection::drag(View *view)
{
  Stroke *stroke = Project::stroke.get();

  if(state == 1)
  {
    view->drawMain(false);
    drawHandles(view, stroke, beginx, beginy, lastx, lasty);
    drawHandles(view, stroke, beginx, beginy, view->imgx, view->imgy);

    lastx = view->imgx;
    lasty = view->imgy;

    redraw(view);
  }
  else if(state == 2)
  {
    view->drawMain(false);
    drawHandles(view, stroke, beginx, beginy, lastx, lasty);

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
        // sides
        case 0:
          beginx = view->imgx + offsetx;
          break;
        case 1:
          lastx = view->imgx - offsetx;
          break;
        case 2:
          beginy = view->imgy + offsety;
          break;
        case 3:
          lasty = view->imgy - offsety;
          break;

        // corners
        case 4:
          beginx = view->imgx + offsetx;
          beginy = view->imgy + offsety;
          break;
        case 5:
          lastx = view->imgx - offsetx;
          beginy = view->imgy + offsety;
          break;
        case 6:
          beginx = view->imgx + offsetx;
          lasty = view->imgy - offsety;
          break;
        case 7:
          lastx = view->imgx - offsetx;
          lasty = view->imgy - offsety;
          break;
      }
    }

    redraw(view);
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
  absrect(view, &beginx, &beginy, &lastx, &lasty);

  redraw(view);

  const int overscroll = Project::overscroll;
  const int x = beginx - overscroll;
  const int y = beginy - overscroll;
  const int w = abs(lastx - beginx) + 1;
  const int h = abs(lasty - beginy) + 1;

  if(state != 3)
    Gui::checkSelectionValues(x, y, w, h);
}

void Selection::move(View *view)
{
  Stroke *stroke = Project::stroke.get();

  if(state == 3)
  {
    const int w = Project::select_bmp->w;
    const int h = Project::select_bmp->h;

    int x1 = view->imgx - w / 2;
    int y1 = view->imgy - h / 2;

    if(view->gridsnap)
    {
      x1 -= Project::overscroll;
      x1 -= x1 % view->gridx;
      x1 += Project::overscroll;

      y1 -= Project::overscroll;
      y1 -= y1 % view->gridy;
      y1 += Project::overscroll;
    }

    const int x2 = x1 + w - 1;
    const int y2 = y1 + h - 1;

    view->window()->cursor(FL_CURSOR_HAND);
    stroke->size(x1, y1, x2, y2);

    view->drawMain(false);
    stroke->previewBrush(view->backbuf, view->ox, view->oy, view->zoom,
                         view->bgr_order);
    view->ignore_tool = true;
    view->redraw();
  }
  else if(state == 2)
  {
    if(view->imgx < stroke->x1 && view->imgy < stroke->y1)
      view->window()->cursor(FL_CURSOR_NW);
    else if(view->imgx > stroke->x2 && view->imgy < stroke->y1)
      view->window()->cursor(FL_CURSOR_NE);
    else if(view->imgx < stroke->x1 && view->imgy > stroke->y2)
      view->window()->cursor(FL_CURSOR_SW);
    else if(view->imgx > stroke->x2 && view->imgy > stroke->y2)
      view->window()->cursor(FL_CURSOR_SE);

    else if(view->imgx < stroke->x1)
      view->window()->cursor(FL_CURSOR_W);
    else if(view->imgx > stroke->x2)
      view->window()->cursor(FL_CURSOR_E);
    else if(view->imgy < stroke->y1)
      view->window()->cursor(FL_CURSOR_N);
    else if(view->imgy > stroke->y2)
      view->window()->cursor(FL_CURSOR_S);

    else
      view->window()->cursor(FL_CURSOR_HAND);
  }
  else
  {
    view->window()->cursor(FL_CURSOR_CROSS);
  }
}

void Selection::key(View *view)
{
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
      select(view);
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
    view->drawMain(false);
    drawHandles(view, stroke, beginx, beginy, lastx, lasty);
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

