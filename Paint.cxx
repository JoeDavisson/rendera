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

void Paint::push(View *view)
{
  if(view->stroke->active && view->stroke->type == 3)
  {
    if(view->dclick)
    {
      view->stroke->end(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
      Blend::set(Brush::main->blend);
      view->stroke->render();
      while(view->stroke->render_callback(view->ox, view->oy, view->zoom))
      {
        if(Fl::get_key(FL_Escape))
          break;
        view->draw_main(1);
        Fl::flush();
      }
      view->stroke->active = 0;
      Blend::set(0);
      view->moving = 0;
      view->draw_main(1);
    }
    else
    {
      view->stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    }
  }
  else
  {
    view->stroke->begin(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
  }

  view->stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();
}

void Paint::drag(View *view)
{
  if(view->stroke->type != 3)
  {
    view->stroke->draw(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    view->draw_main(0);
    view->stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
  }
}

void Paint::release(View *view)
{
  if(view->stroke->active && view->stroke->type != 3)
  {
    view->stroke->end(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
    Blend::set(Brush::main->blend);
    view->stroke->render();
    while(view->stroke->render_callback(view->ox, view->oy, view->zoom))
    {
      if(Fl::get_key(FL_Escape))
        break;
      view->draw_main(1);
      Fl::flush();
    }
    view->stroke->active = 0;
    Blend::set(0);
  }

  view->draw_main(1);
}

void Paint::move(View *view)
{
  switch(view->stroke->type)
  {
    case 3:
      if(view->stroke->active)
      {
        view->stroke->polyline(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
        view->draw_main(0);
        view->stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
        view->redraw();
      }
      break;
    case 0:
    case 2:
    case 4:
    case 6:
      Map::main->rectfill(view->oldimgx - 48, view->oldimgy - 48, view->oldimgx + 48, view->oldimgy + 48, 0);
      Map::main->rectfill(view->imgx - 48, view->imgy - 48, view->imgx + 48, view->imgy + 48, 0);
      view->stroke->draw_brush(view->imgx, view->imgy, 255);
      view->stroke->size(view->imgx - 48, view->imgy - 48, view->imgx + 48, view->imgy + 48);
      view->stroke->make_blitrect(view->stroke->x1, view->stroke->y1, view->stroke->x2, view->stroke->y2, view->ox, view->oy, 96, view->zoom);
      view->draw_main(0);
      view->stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
      view->redraw();
      break;
  }
}

