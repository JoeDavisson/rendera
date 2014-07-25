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

#include "rendera.h"

static int inbox(int x, int y, int x1, int y1, int x2, int y2)
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

static void absrect(int *x1, int *y1, int *x2, int *y2)
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
  }
  else if(started == 2)
  {
    if(view->dclick && inbox(view->imgx, view->imgy,
                             beginx, beginy, lastx, lasty))
    {
      started = 0;
      absrect(&beginx, &beginy, &lastx, &lasty);
      int w = lastx - beginx;
      int h = lasty - beginy;
      if(w < 1)
        w = 1;
      if(h < 1)
        h = 1;
      Bitmap *temp = new Bitmap(w, h);
      Bitmap::main->blit(temp, beginx, beginy, 0, 0, w, h);
      delete Bitmap::main;
      int overscroll = Bitmap::main->overscroll;
      int aw = w + overscroll * 2;
      int ah = h + overscroll * 2;
      Bitmap::main = new Bitmap(aw, ah);
      Bitmap::main->clear(makecol(128, 128, 128));
      Bitmap::main->set_clip(overscroll, overscroll, aw - overscroll - 1, ah - overscroll - 1);
      temp->blit(Bitmap::main, 0, 0, overscroll, overscroll, w, h);
      delete temp;
      view->zoom = 1;
      view->ox = 0;
      view->oy = 0;
      view->draw_main(1);
    }
  }
}

void Crop::drag(View *view)
{
  if(started == 1)
  {
    absrect(&beginx, &beginy, &lastx, &lasty);
    Map::main->rect(beginx, beginy, lastx, lasty, 0);
    absrect(&beginx, &beginy, &view->imgx, &view->imgy);
    Map::main->rect(beginx, beginy, view->imgx, view->imgy, 255);
    stroke->size(beginx, beginy, view->imgx, view->imgy);

    lastx = view->imgx;
    lasty = view->imgy;

    view->draw_main(1);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
  }
  else if(started == 2)
  {
    Map::main->rect(beginx, beginy, lastx, lasty, 0);

    if(inbox(view->imgx, view->imgy, beginx, beginy, lastx, lasty))
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

        drag_started = 1;
        resize_started = 0;
      }
    }
    else if(!drag_started && resize_started)
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
    }
  }

  absrect(&beginx, &beginy, &lastx, &lasty);
  Map::main->rect(beginx, beginy, lastx, lasty, 255);
  stroke->size(beginx, beginy, lastx, lasty);
  view->draw_main(1);
  stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();
  check_crop();
}

void Crop::release(View *view)
{
  if(started == 1)
  {
    started = 2;
  }

  drag_started = 0;
  resize_started = 0;
}

void Crop::move(View *view)
{
}

