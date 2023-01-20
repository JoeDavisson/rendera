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
    const int w = *x2 - *x1;
    const int h = *y2 - *y1;

    if(view->gridsnap)
    {
      *x1 -= *x1 % gridx;

      *y1 -= *y1 % gridy;

      *x2 += 1;
      *x2 -= *x2 % gridx;
      *x2 -= 1;

      *y2 += 1;
      *y2 -= *y2 % gridy;
      *y2 -= 1;
    }

    if(state == 3)
    {
      *x2 = *x1 + w;
      *y2 = *y1 + h;
    }
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
    x2 = x2 * zoom - ox + zoom - 1;
    y2 = y2 * zoom - oy + zoom - 1;

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
  }

  void select(View *view)
  {
    if(state == 3)
      return;

    state = 3;
    Gui::selectCopyEnable(false);
    Gui::selectPasteEnable(true);
    Gui::selectCropEnable(false);

    absrect(view, &beginx, &beginy, &lastx, &lasty);

    if(beginx < Project::bmp->cl)
      beginx = Project::bmp->cl;
    if(beginy < Project::bmp->ct)
      beginy = Project::bmp->ct;
    if(lastx > Project::bmp->cr)
      lastx = Project::bmp->cr;
    if(lasty > Project::bmp->cb)
      lasty = Project::bmp->cb;

    int w = (lastx - beginx) + 1;
    int h = (lasty - beginy) + 1;

    if(w < 1)
      w = 1;
    if(h < 1)
      h = 1;

    delete(Project::select_bmp);
    Project::select_bmp = new Bitmap(w, h);
    Project::bmp->blit(Project::select_bmp, beginx, beginy, 0, 0, w, h);

    Gui::selectValues(0, 0, 0, 0);
  }

  void crop(View *view)
  {
    Project::undo->push();

    absrect(view, &beginx, &beginy, &lastx, &lasty);

   if(beginx < Project::bmp->cl)
      beginx = Project::bmp->cl;
    if(beginy < Project::bmp->ct)
      beginy = Project::bmp->ct;
    if(lastx > Project::bmp->cr)
      lastx = Project::bmp->cr;
    if(lasty > Project::bmp->cb)
      lasty = Project::bmp->cb;

    int w = (lastx - beginx) + 1;
    int h = (lasty - beginy) + 1;

    if(w < 1)
      w = 1;
    if(h < 1)
      h = 1;

    Bitmap *temp = new Bitmap(w, h);
    Project::bmp->blit(temp, beginx, beginy, 0, 0, w, h);

    Project::replaceImage(w, h);
    temp->blit(Project::bmp, 0, 0, 0, 0, w, h);
    delete temp;

    view->zoom = 1;
    view->ox = 0;
    view->oy = 0;
    view->drawMain(true);
    Gui::selectValues(0, 0, 0, 0);
  }

  void paste(View *view)
  {
    Project::undo->push();
    const int w = Project::select_bmp->w;
    const int h = Project::select_bmp->h;

    int x1 = beginx;
    int y1 = beginy;

    if(view->gridsnap)
    {
      x1 -= x1 % view->gridx;
      y1 -= y1 % view->gridy;
    }

    Blend::set(Project::brush->blend);

    const int trans = Project::brush->trans;
    const int alpha = Gui::getSelectAlpha();

    for(int y = 0; y < h; y++)
    {
      for(int x = 0; x < w; x++)
      {
        int c = Project::select_bmp->getpixel(x, y);
        const int t = scaleVal(255 - geta(c), trans);

        c |= 0xff000000;

        if(alpha)
          Project::bmp->setpixel(x1 + x, y1 + y, c, t);
        else
          Project::bmp->setpixel(x1 + x, y1 + y, c, trans);
      }
    }

    Blend::set(Blend::TRANS);
    view->drawMain(true);
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
  }
  else if(state == 2 || state == 3)
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
          offsetx = std::abs(view->imgx - beginx);
          offsety = std::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgx > lastx && view->imgy < beginy)
        {
          side = 5;
          offsetx = std::abs(view->imgx - lastx);
          offsety = std::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgx < beginx && view->imgy > lasty)
        {
          side = 6;
          offsetx = std::abs(view->imgx - beginx);
          offsety = std::abs(view->imgy - lasty);
          resize_started = true;
        }
        else if(view->imgx > lastx && view->imgy > lasty)
        {
          side = 7;
          offsetx = std::abs(view->imgx - lastx);
          offsety = std::abs(view->imgy - lasty);
          resize_started = true;
        }

        // sides
        else if(view->imgx < beginx)
        {
          side = 0;
          offsetx = std::abs(view->imgx - beginx);
          resize_started = true;
        }
        else if(view->imgx > lastx)
        {
          side = 1;
          offsetx = std::abs(view->imgx - lastx);
          resize_started = true;
        }
        else if(view->imgy < beginy)
        {
          side = 2;
          offsety = std::abs(view->imgy - beginy);
          resize_started = true;
        }
        else if(view->imgy > lasty)
        {
          side = 3;
          offsety = std::abs(view->imgy - lasty);
          resize_started = true;
        }

        resize_started = true;
      }
    }
  }
}

void Selection::drag(View *view)
{
  Stroke *stroke = Project::stroke;

  if(state == 1)
  {
    view->drawMain(false);
    drawHandles(view, stroke, beginx, beginy, lastx, lasty);
    drawHandles(view, stroke, beginx, beginy, view->imgx, view->imgy);

    lastx = view->imgx;
    lasty = view->imgy;
  }
  else if(state == 2 || state == 3)
  {
    if(drag_started)
    {
      const int dx = view->imgx - view->oldimgx;
      const int dy = view->imgy - view->oldimgy;

      beginx += dx;
      beginy += dy;
      lastx += dx;
      lasty += dy;
    }
    else if(state == 2 && resize_started)
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
  }

  if(state == 3)
  {
    view->window()->cursor(FL_CURSOR_HAND);

    if(inbox(view->imgx, view->imgy, beginx, beginy, lastx, lasty))
    {
      const int w = Project::select_bmp->w;
      const int h = Project::select_bmp->h;

      int x1 = beginx;
      int y1 = beginy;

      if(view->gridsnap)
      {
        x1 -= x1 % view->gridx;
        y1 -= y1 % view->gridy;
      }

      const int x2 = x1 + w - 1;
      const int y2 = y1 + h - 1;

      stroke->size(x1, y1, x2, y2);
    }
  }

  int temp_beginx = beginx;
  int temp_beginy = beginy;
  int temp_lastx = lastx;
  int temp_lasty = lasty;

  absrect(view, &temp_beginx, &temp_beginy, &temp_lastx, &temp_lasty);

  int x = temp_beginx;
  int y = temp_beginy;
  int w = abs(temp_lastx - temp_beginx) + 1;
  int h = abs(temp_lasty - temp_beginy) + 1;

  Gui::selectValues(x, y, w, h);
  redraw(view);
}

void Selection::release(View *view)
{
  if(state == 1)
  {
    state = 2;
    Gui::selectCopyEnable(true);
    Gui::selectCropEnable(true);
  }

  drag_started = false;
  resize_started = false;
  absrect(view, &beginx, &beginy, &lastx, &lasty);

  redraw(view);

  const int x = beginx;
  const int y = beginy;
  const int w = abs(lastx - beginx) + 1;
  const int h = abs(lasty - beginy) + 1;

  if(state != 3)
    Gui::selectValues(x, y, w, h);
}

void Selection::move(View *view)
{
  Stroke *stroke = Project::stroke;

  if(state == 2)
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
  else if(state == 3)
  {
    if(inbox(view->imgx, view->imgy, beginx, beginy, lastx, lasty))
      view->window()->cursor(FL_CURSOR_HAND);
    else
      view->window()->cursor(FL_CURSOR_DEFAULT);
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

  if(mode == 2 && state == 3)
  {
    paste(view);
  }
  else if(mode == 1)
  {
    crop(view);
    reset();
  }
  else
  {
    select(view);
  }
}

void Selection::redraw(View *view)
{
  Stroke *stroke = Project::stroke;

  view->drawMain(false);

  if(state == 1 || state == 2)
  {
    drawHandles(view, stroke, beginx, beginy, lastx, lasty);
  }
  else if(state == 3)
  {
    stroke->previewSelection(view);
    drawHandles(view, stroke, beginx, beginy, lastx, lasty);
  }

  view->redraw();
}

bool Selection::isActive()
{
  return false;
}

void Selection::reset()
{
  state = 0;
  Gui::selectValues(0, 0, 0, 0);
  Gui::selectCopyEnable(false);
  Gui::selectPasteEnable(false);
  Gui::selectCropEnable(false);
  Gui::getView()->drawMain(true);
}

void Selection::reload()
{
  const int w = Project::select_bmp->w;
  const int h = Project::select_bmp->h;

  beginx = 0;
  beginy = 0;
  lastx = w - 1;
  lasty = h - 1;
  state = 3;

  Project::stroke->size(beginx, beginy, lastx, lasty);
  Gui::selectValues(0, 0, w, h);
}

