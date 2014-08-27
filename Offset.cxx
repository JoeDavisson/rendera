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

#include "Offset.h"
#include "Tool.h"
#include "Bitmap.h"
#include "View.h"
#include "Stroke.h"

Offset::Offset()
{
}

Offset::~Offset()
{
}

void Offset::push(View *view)
{
  stroke->max();
  undo(0);

  int w = Bitmap::main->cw;
  int h = Bitmap::main->ch;
  int overscroll = Bitmap::main->overscroll;

  beginx = view->imgx;
  beginy = view->imgy;

  delete Bitmap::offset_buffer;
  Bitmap::offset_buffer = new Bitmap(w, h);
  Bitmap::main->blit(Bitmap::offset_buffer, overscroll, overscroll, 0, 0, w, h);
}

void Offset::drag(View *view)
{
  int w = Bitmap::main->cw;
  int h = Bitmap::main->ch;
  int overscroll = Bitmap::main->overscroll;

  int dx = view->imgx - beginx;
  int dy = view->imgy - beginy;

  int x = dx;
  int y = dy;

  while(x < 0)
    x += w;
  while(y < 0)
    y += h;
  while(x >= w)
    x -= w;
  while(y >= h)
    y -= h;
  
  Bitmap::offset_buffer->blit(Bitmap::main, 0, 0,
                              x + overscroll, y + overscroll, w - x, h - y);
  Bitmap::offset_buffer->blit(Bitmap::main, w - x, 0,
                              overscroll, y + overscroll, x, h - y);
  Bitmap::offset_buffer->blit(Bitmap::main, 0, h - y,
                              x + overscroll, overscroll, w - x, y);
  Bitmap::offset_buffer->blit(Bitmap::main, w - x, h - y,
                              overscroll, overscroll, x, y);

  view->draw_main(1);
}

void Offset::release(View *)
{
}

void Offset::move(View *)
{
}

