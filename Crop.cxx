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

#include "Crop.h"
#include "Tool.h"
#include "Bitmap.h"
#include "Map.h"
#include "View.h"
#include "Stroke.h"
#include "Gui.h"

namespace
{
  bool inbox(int x, int y, int x1, int y1, int x2, int y2)
  {
    if(x1 > x2)
      SWAP(x1, x2);
    if(y1 > y2)
      SWAP(y1, y2);

    if(x >= x1 && x <= x2 && y >= y1 && y <= y2)
      return 1;
    else
      return 0;
}

  void absrect(int *x1, int *y1, int *x2, int *y2)
  {
    if(*x1 > *x2)
      SWAP(*x1, *x2);
    if(*y1 > *y2)
      SWAP(*y1, *y2);

    if(*x1 < Bitmap::main->cl)
      *x1 = Bitmap::main->cl;
    if(*y1 < Bitmap::main->ct)
      *y1 = Bitmap::main->ct;
    if(*x2 > Bitmap::main->cr)
      *x2 = Bitmap::main->cr;
    if(*y2 > Bitmap::main->cb)
      *y2 = Bitmap::main->cb;
  }
}

Crop::Crop()
{
  drag_started = 0;
  resize_started = 0;
  side = 0;
}

Crop::~Crop()
{
}

void Crop::push(View *view)
{
  if(started == 0)
  {
    Map::main->clear(0);
    beginx = view->imgx;
    beginy = view->imgy;
    lastx = view->imgx;
    lasty = view->imgy;
    started = 1;
    active = 1;
  }
  else if(started == 2)
  {
    if(drag_started == 0 && resize_started == 0)
    {
      if(inbox(view->imgx, view->imgy, beginx, beginy, lastx, lasty))
      {
        drag_started = 1;
      }
      else
      {
        if(view->imgx < beginx)
        {
          side = 0;
          offset = ABS(view->imgx - beginx);
          resize_started = 1;
        }
        else if(view->imgx > lastx)
        {
          side = 1;
          offset = ABS(view->imgx - lastx);
          resize_started = 1;
        }
        else if(view->imgy < beginy)
        {
          side = 2;
          offset = ABS(view->imgy - beginy);
          resize_started = 1;
        }
        else if(view->imgy > lasty)
        {
          side = 3;
          offset = ABS(view->imgy - lasty);
          resize_started = 1;
        }

        resize_started = 1;
      }
    }
  }
}

void Crop::drag(View *view)
{
  if(started == 1)
  {
//    absrect(&beginx, &beginy, &lastx, &lasty);
    Map::main->rect(beginx, beginy, lastx, lasty, 0);
//    absrect(&beginx, &beginy, &view->imgx, &view->imgy);
    Map::main->rect(beginx, beginy, view->imgx, view->imgy, 255);
    stroke->size(beginx, beginy, view->imgx, view->imgy);

    lastx = view->imgx;
    lasty = view->imgy;

    view->draw_main(0);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
    Fl::flush();
  }
  else if(started == 2)
  {
    Map::main->rect(beginx, beginy, lastx, lasty, 0);

    if(drag_started == 1)
    {

      int dx = view->imgx - view->oldimgx;
      int dy = view->imgy - view->oldimgy;

      int cl = Bitmap::main->cl;
      int cr = Bitmap::main->cr;
      int ct = Bitmap::main->ct;
      int cb = Bitmap::main->cb;

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
    else if(resize_started == 1)
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
  }

  redraw(view);
  Gui::checkCropValues();
}

void Crop::release(View *view)
{
  if(started == 1)
  {
    started = 2;
  }

  drag_started = 0;
  resize_started = 0;
  absrect(&beginx, &beginy, &lastx, &lasty);
  redraw(view);
}

void Crop::move(View *)
{
}

void Crop::done(View *view)
{
  if(started == 0)
    return;

  stroke->max(); 
  undo(1);
  started = 0;
  active = 0;
  absrect(&beginx, &beginy, &lastx, &lasty);
  int w = lastx - beginx + 1;
  int h = lasty - beginy + 1;
  if(w < 1)
    w = 1;
  if(h < 1)
    h = 1;
  Bitmap *temp = new Bitmap(w, h);
  Bitmap::main->blit(temp, beginx, beginy, 0, 0, w, h);
  delete Bitmap::main;
  Bitmap::main = new Bitmap(w, h, 64,
                            makecol(255, 255, 255), makecol(128, 128, 128));
  temp->blit(Bitmap::main, 0, 0, Bitmap::main->overscroll, Bitmap::main->overscroll, w, h);
  delete temp;
  view->zoom = 1;
  view->ox = 0;
  view->oy = 0;
  view->draw_main(1);
}

void Crop::redraw(View *view)
{
  active = 0;
//  absrect(&beginx, &beginy, &lastx, &lasty);
  Map::main->rect(beginx, beginy, lastx, lasty, 255);
  stroke->size(beginx, beginy, lastx, lasty);
  view->draw_main(0);
  stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();
  Fl::flush();
  active = 1;
}

