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

#include "Paint.h"
#include "Bitmap.h"
#include "Map.h"
#include "Blend.h"
#include "Brush.h"
#include "Stroke.h"
#include "View.h"

static inline int is_edge(Map *map, const int x, const int y)
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

static inline int fdist(const int x1, const int y1, const int x2, const int y2)
{
  const int dx = (x1 - x2);
  const int dy = (y1 - y2);

  return dx * dx + dy * dy;
}

static inline int sdist(const int x1, const int y1, const int x2, const int y2, const int edge, const int trans)
{
  float d = sqrtf(fdist(x1, y1, x2, y2));
  float s = (255 - trans) / (((3 << edge) >> 1) + 1);

  if(s < 1)
    s = 1;

  int temp = 255;

  temp -= s * d;
  if(temp < trans)
    temp = trans;

  return temp;
}

static inline void shrink_block(unsigned char *s0, unsigned char *s1,
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

// The paintbrush uses two distinct algorithms, marching squares (faster)
// and one that uses distance calculations (slower).
Paint::Paint()
{
}

Paint::~Paint()
{
}

void Paint::render_begin_normal(View * /* view */)
{
  Brush *brush = Brush::main;
  /* Map *map = Map::main; */

  soft_trans = 255;
  float j = (float)(3 << brush->edge);
  soft_step = (255 - brush->trans) / (((3 << brush->edge) >> 1) + 1);

  if(soft_step < 1)
    soft_step = 1;

  render_pos = 0;
  render_end = j;
}

void Paint::render_begin_smooth(View * /* view */)
{
  Brush *brush = Brush::main;

  int x, y;

  if(brush->edge == 0)
    return;

  render_count = 0;

  for(y = stroke->y1; y <= stroke->y2; y++)
  {
    for(x = stroke->x1; x <= stroke->x2; x++)
    {
      if((Map::main)->getpixel(x, y) && is_edge((Map::main), x, y))
      {
        stroke->edgecachex[render_count] = x;
        stroke->edgecachey[render_count] = y;
        render_count++;
        render_count &= 0xFFFFF;
      }
    }
  }

  render_pos = stroke->y1;
}

void Paint::render_begin(View *view)
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

  if(brush->smooth)
    render_begin_smooth(view);
  else
    render_begin_normal(view);
}

int Paint::render_callback_normal(View *view)
{
  Brush *brush = Brush::main;
  Map *map = Map::main;

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

  stroke->make_blitrect(stroke->x1, stroke->y1,
                        stroke->x2, stroke->y2,
                        view->ox, view->oy, 1, view->zoom);
  return 1;
}

int Paint::render_callback_smooth(View *view)
{
  Brush *brush = Brush::main;
  Map *map = Map::main;

  if(render_count < 2)
  {
    active = 0;
    return 0;
  }

  int x, y;

  render_end = render_pos + 64;

  if(render_end > stroke->y2)
    render_end = stroke->y2;

  for(y = render_pos; y < render_end; y++)
  {
    unsigned char *p = map->row[y] + stroke->x1;

    for(x = stroke->x1; x <= stroke->x2; x++)
    {
      if(*p == 0)
      {
        p++;
        continue;
      }

      int *cx = &stroke->edgecachex[0];
      int *cy = &stroke->edgecachey[0];
      int temp1 = fdist(x, y, *cx++, *cy++);
      int z = 0;
      int i;

      for(i = 1; i < render_count; i++)
      {
        const int dx = (x - *cx++);
        const int dy = (y - *cy++);

        const int temp2 = dx * dx + dy * dy;

        if(temp2 < temp1)
        {
          z = i;
          temp1 = temp2;
        }
      }

      Bitmap::main->setpixel(x, y, brush->color,
        sdist(x, y, stroke->edgecachex[z],
              stroke->edgecachey[z], brush->edge, brush->trans));

      p++;
    }
  }

  stroke->make_blitrect(stroke->x1, render_pos,
                        stroke->x2, render_end,
                        view->ox, view->oy, 1, view->zoom);

  render_pos += 64;

  if(render_pos > stroke->y2)
  {
    active = 0;
    return 0;
  }

  return 1;
}

int Paint::render_callback(View *view)
{
  Brush *brush = Brush::main;
  /* Map *map = Map::main; */

  if(brush->edge == 0)
    return 0;

  if(brush->smooth)
    return render_callback_smooth(view);
  else
    return render_callback_normal(view);
}

void Paint::push(View *view)
{
  if(active && stroke->type == 3)
  {
    if(view->dclick)
    {
      stroke->end(view->imgx, view->imgy, view->ox, view->oy, view->zoom);
      Blend::set(Brush::main->blend);
      render_begin(view);
      while(render_callback(view))
      {
        if(Fl::get_key(FL_Escape))
          break;
        view->draw_main(1);
      }
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
    render_begin(view);
    while(render_callback(view))
    {
      if(Fl::get_key(FL_Escape))
        break;
      view->draw_main(1);
    }
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
        stroke->polyline(view->imgx, view->imgy,
                         view->ox, view->oy, view->zoom);
        view->draw_main(0);
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
      stroke->draw_brush(view->imgx, view->imgy, 255);
      stroke->size(view->imgx - 48, view->imgy - 48,
                   view->imgx + 48, view->imgy + 48);
      stroke->make_blitrect(stroke->x1, stroke->y1,
                            stroke->x2, stroke->y2,
                            view->ox, view->oy, 96, view->zoom);
      view->draw_main(0);
      stroke->preview(view->backbuf, view->ox, view->oy, view->zoom);
      view->redraw();
      break;
  }
}

