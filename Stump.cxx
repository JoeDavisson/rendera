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

Stump::Stump()
{
}

Stump::~Stump()
{
}

void Stump::render()
{
  Brush *brush = Brush::main;
  Map *map = Map::main;

  int x, y;
  int i;
  int c[9];

  for(y = stroke->y1; y <= stroke->y2; y++)
  {
    for(x = stroke->x1; x <= stroke->x2; x++)
    {
      int p = map->getpixel(x, y);
      if(p)
      {
        c[0] = Bitmap::main->getpixel(x - 1, y - 1); 
        c[1] = Bitmap::main->getpixel(x, y - 1); 
        c[2] = Bitmap::main->getpixel(x + 1, y - 1); 
        c[3] = Bitmap::main->getpixel(x - 1, y); 
        c[4] = Bitmap::main->getpixel(x, y); 
        c[5] = Bitmap::main->getpixel(x + 1, y); 
        c[6] = Bitmap::main->getpixel(x - 1, y + 1); 
        c[7] = Bitmap::main->getpixel(x, y + 1); 
        c[8] = Bitmap::main->getpixel(x + 1, y + 1); 

        int r = 0;
        int g = 0;
        int b = 0;

        r += getr(c[0]) * 1;
        r += getr(c[1]) * 2;
        r += getr(c[2]) * 1;
        r += getr(c[3]) * 2;
        r += getr(c[4]) * 3;
        r += getr(c[5]) * 2;
        r += getr(c[6]) * 1;
        r += getr(c[7]) * 2;
        r += getr(c[8]) * 1;
        r /= 15;

        g += getg(c[0]) * 1;
        g += getg(c[1]) * 2;
        g += getg(c[2]) * 1;
        g += getg(c[3]) * 2;
        g += getg(c[4]) * 3;
        g += getg(c[5]) * 2;
        g += getg(c[6]) * 1;
        g += getg(c[7]) * 2;
        g += getg(c[8]) * 1;
        g /= 15;

        b += getb(c[0]) * 1;
        b += getb(c[1]) * 2;
        b += getb(c[2]) * 1;
        b += getb(c[3]) * 2;
        b += getb(c[4]) * 3;
        b += getb(c[5]) * 2;
        b += getb(c[6]) * 1;
        b += getb(c[7]) * 2;
        b += getb(c[8]) * 1;
        b /= 15;

        Bitmap::main->setpixel(x, y, makecol(r, g, b), amount);
      }
    }
  }
}

void Stump::push(View *view)
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

void Stump::drag(View *view)
{
  if(stroke->type != 3)
  {
    stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    view->draw_main(0);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
  }
}

void Stump::release(View *view)
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

void Stump::move(View *view)
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

