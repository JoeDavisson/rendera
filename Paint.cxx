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

Paint::Paint()
{
}

Paint::~Paint()
{
}

void Paint::render()
{
  undo(0);

  Brush *brush = Brush::main;
  Map *map = Map::main;

  int x, y;

  for(y = stroke->y1; y <= stroke->y2; y++)
  {
    for(x = stroke->x1; x <= stroke->x2; x++)
    {
      int c = map->getpixel(x, y);
      if(c)
        Bitmap::main->setpixel(x, y, brush->color, brush->trans);
    }
  }
}

void Paint::push(View *view)
{
  if(active && stroke->type == 3)
  {
    if(view->dclick)
    {
      stroke->end(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
      Blend::set(Brush::main->blend);
      render();
      active = 0;
      Blend::set(0);
      view->moving = 0;
      view->draw_main(1);
      return;
    }
    else
    {
      stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    }
  }
  else
  {
    stroke->begin(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
  }

  stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();

  active = 1;
}

void Paint::drag(View *view)
{
  if(stroke->type != 3)
  {
    stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    view->draw_main(0);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
  }
}

void Paint::release(View *view)
{
  if(active && stroke->type != 3)
  {
    stroke->end(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    Blend::set(Brush::main->blend);
    render();
    active = 0;
    Blend::set(0);
  }

  view->draw_main(1);
}

void Paint::move(View *view)
{
  switch(stroke->type)
  {
    case 3:
      if(active)
      {
        stroke->polyline(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
        view->draw_main(0);
        stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
        view->redraw();
      }
      break;
    case 0:
    case 2:
    case 4:
    case 6:
      Map::main->rectfill(view->oldimgx - 48, view->oldimgy - 48, view->oldimgx + 48, view->oldimgy + 48, 0);
      Map::main->rectfill(view->imgx - 48, view->imgy - 48, view->imgx + 48, view->imgy + 48, 0);
      stroke->draw_brush(view->imgx, view->imgy, 255);
      stroke->size(view->imgx - 48, view->imgy - 48, view->imgx + 48, view->imgy + 48);
      stroke->make_blitrect(stroke->x1, stroke->y1, stroke->x2, stroke->y2, view->ox, view->oy, 96, view->zoom);
      view->draw_main(0);
      stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
      view->redraw();
      break;
  }
}

