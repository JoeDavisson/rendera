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

#include <cmath>

#include "Paint.H"
#include "Bitmap.H"
#include "Map.H"
#include "Blend.H"
#include "Brush.H"
#include "Stroke.H"
#include "View.H"

namespace
{
  inline bool is_edge(Map *map, const int &x, const int &y)
  {
    if(x < 1 || x >= map->w - 1 || y < 1 || y >= map->h - 1)
      return 0;

    if( *(map->row[y - 1] + x) &&
        *(map->row[y] + x - 1) &&
        *(map->row[y] + x + 1) &&
        *(map->row[y + 1] + x) )
      return 0;
    else
      return 1;
  }

  inline int fdist(const int &x1, const int &y1, const int &x2, const int &y2)
  {
    const int dx = (x1 - x2);
    const int dy = (y1 - y2);

    return dx * dx + dy * dy;
  }

  inline int sdist(const int &x1, const int &y1,
                   const int &x2, const int &y2,
                   const int &edge, const int &trans)
  {
    float d = std::sqrt(fdist(x1, y1, x2, y2));
    float s = (255 - trans) / (((3 << edge) >> 1) + 1);

    if(s < 1)
      s = 1;

    int temp = 255;

    temp -= s * d;
    if(temp < trans)
      temp = trans;

    return temp;
  }

  inline void shrink_block(unsigned char *s0, unsigned char *s1,
                           unsigned char *s2, unsigned char *s3)
  {
    const int z = (*s0 << 0) + (*s1 << 1) + (*s2 << 2) + (*s3 << 3);

    switch(z)
    {
      case 0:
      case 15:
        return;
      case 7:
        *s1 = 0;
        *s2 = 0;
        return;
      case 11:
        *s0 = 0;
        *s3 = 0;
        return;
      case 13:
        *s0 = 0;
        *s3 = 0;
        return;
      case 14:
        *s1 = 0;
        *s2 = 0;
        return;
    }

    *s0 = 0;
    *s1 = 0;
    *s2 = 0;
    *s3 = 0;
  }

 inline void grow_block(unsigned char *s0, unsigned char *s1,
                        unsigned char *s2, unsigned char *s3)
  {
    int z = (*s0 << 0) + (*s1 << 1) + (*s2 << 2) + (*s3 << 3);

    switch (z)
    {
      case 0:
      case 15:
        return;
      case 1:
        *s1 = 1;
        *s2 = 1;
        return;
      case 2:
        *s0 = 1;
        *s3 = 1;
        return;
      case 4:
        *s0 = 1;
        *s3 = 1;
        return;
      case 8:
        *s1 = 1;
        *s2 = 1;
        return;
    }
    *s0 = 1;
    *s1 = 1;
    *s2 = 1;
    *s3 = 1;
  }
}

// The paintbrush uses two distinct algorithms, marching squares (faster)
// and one that uses distance calculations (slower).
Paint::Paint()
{
}

Paint::~Paint()
{
}

void Paint::renderBegin(View *)
{
  undo(0);

  Brush *brush = Brush::main;
  Map *map = Map::main;

  if(brush->edge == 0)
  {
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
    return;
  }

  soft_trans = 255;
  float j = (float)(3 << brush->edge);
  soft_step = (float)(255 - brush->trans) / (((3 << brush->edge) >> 1) + 1);

  render_pos = 0;
  render_end = j;
}

int Paint::renderCallback(View *view)
{
  Brush *brush = Brush::main;
  Map *map = Map::main;

  if(brush->edge == 0)
    return 0;

  int x, y;

  int found = 0;
  int i;
  int end = render_pos + 64;
  if(end > render_end)
    end = render_end;

  for(i = render_pos; i < end; i++)
  {
    for(y = stroke->y1 + (i & 1); y < stroke->y2; y += 2)
    {
      for(x = stroke->x1 + (i & 1); x < stroke->x2; x += 2)
      {
        unsigned char *s0 = map->row[y] + x;
        unsigned char *s1 = map->row[y] + x + 1;
        unsigned char *s2 = map->row[y + 1] + x;
        unsigned char *s3 = map->row[y + 1] + x + 1;

        *s0 &= 1;
        *s1 &= 1;
        *s2 &= 1;
        *s3 &= 1;

        if(*s0 | *s1 | *s2 | *s3)
          found = 1;

        unsigned char d0 = *s0;
        unsigned char d1 = *s1;
        unsigned char d2 = *s2;
        unsigned char d3 = *s3;

        shrink_block(s0, s1, s2, s3);

        if(!*s0 && d0)
          Bitmap::main->setpixel(x, y, brush->color, soft_trans);
        if(!*s1 && d1)
          Bitmap::main->setpixel(x + 1, y, brush->color, soft_trans);
        if(!*s2 && d2)
          Bitmap::main->setpixel(x, y + 1, brush->color, soft_trans);
        if(!*s3 && d3)
          Bitmap::main->setpixel(x + 1, y + 1, brush->color, soft_trans);
      }
    }

    if(found == 0)
      break;

    soft_trans -= soft_step;
    if(soft_trans < brush->trans)
    {
      soft_trans = brush->trans;
      for(y = stroke->y1; y <= stroke->y2; y++)
      {
        for(x = stroke->x1; x <= stroke->x2; x++)
        {
          int c = map->getpixel(x, y);
          if(c)
            Bitmap::main->setpixel(x, y, brush->color, soft_trans);
        }
      }
      active = 0;
      return 0;
    }
  }

  render_pos += 64;

  if(found == 0 || render_pos >= render_end)
  {
    active = 0;
    return 0;
  }

  stroke->makeBlitrect(stroke->x1, stroke->y1,
                        stroke->x2, stroke->y2,
                        view->ox, view->oy, 1, view->zoom);
  return 1;
}

void Paint::push(View *view)
{
  if(active && stroke->type == 3)
  {
    if(view->dclick)
    {
      stroke->end();
      Blend::set(Brush::main->blend);
      renderBegin(view);
      while(renderCallback(view))
      {
        if(Fl::get_key(FL_Escape))
          break;
        view->drawMain(1);
      }
      active = 0;
      Blend::set(Blend::TRANS);
      view->moving = 0;
      view->drawMain(1);
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
    view->drawMain(0);
    stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
    view->redraw();
  }
}

void Paint::release(View *view)
{
  if(active && stroke->type != 3)
  {
    stroke->end();
    Blend::set(Brush::main->blend);
    renderBegin(view);
    while(renderCallback(view))
    {
      if(Fl::get_key(FL_Escape))
        break;
      view->drawMain(1);
    }
    active = 0;
    Blend::set(Blend::TRANS);
  }

  view->drawMain(1);
}

void Paint::move(View *view)
{
  switch(stroke->type)
  {
    case 3:
      if(active)
      {
        stroke->polyline(view->imgx, view->imgy,
                         view->ox, view->oy, view->zoom);
        view->drawMain(0);
        stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
        view->redraw();
      }
      break;
    case 0:
    case 2:
    case 4:
    case 6:
      Map::main->rectfill(view->oldimgx - 48, view->oldimgy - 48,
                          view->oldimgx + 48, view->oldimgy + 48, 0);
      Map::main->rectfill(view->imgx - 48, view->imgy - 48,
                          view->imgx + 48, view->imgy + 48, 0);
      stroke->drawBrush(view->imgx, view->imgy, 255);
      stroke->size(view->imgx - 48, view->imgy - 48,
                   view->imgx + 48, view->imgy + 48);
      stroke->makeBlitrect(stroke->x1, stroke->y1,
                            stroke->x2, stroke->y2,
                            view->ox, view->oy, 96, view->zoom);
      view->drawMain(0);
      stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
      view->redraw();
      break;
  }
}

void Paint::done(View *)
{
}

void Paint::redraw(View *view)
{
  active = 0;
  view->drawMain(0);
  stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
  view->redraw();
  active = 1;
}

