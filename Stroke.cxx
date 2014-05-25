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

static inline int is_edge(Map *map, int x, int y)
{
  if((map->getpixel(x - 1, y)) &&
     (map->getpixel(x + 1, y)) &&
     (map->getpixel(x, y - 1)) &&
     (map->getpixel(x, y + 1)))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

static inline float fdist(const int x1, const int y1, const int x2, const int y2)
{
  const int dx = (x1 - x2);
  const int dy = (y1 - y2);
  return dx * dx + dy * dy;
}

static inline int sdist(const int x1, const int y1, const int x2, const int y2, const int t, const int edge, const int trans)
{
  float d = sqrtf(fdist(x1, y1, x2, y2));

  float j = (float)(3 << edge);
  float s = (float)(255 - t) / (j / 2 + 1);

  if(s < 1.0f)
    s = 1.0f;

  float temp = 255.0f;
  temp -= (s * d);
  if(temp < trans)
    temp = trans;

  return (int)temp;
}

Stroke::Stroke()
{
  polycachex = new int[65536];
  polycachey = new int[65536];
  edgecachex = new int[0x100000];
  edgecachey = new int[0x100000];
  polycount = 0;
  type = 0;
  active = 0;
}

Stroke::~Stroke()
{
  delete[] polycachex;
  delete[] polycachey;
  delete[] edgecachex;
  delete[] edgecachey;
}

void Stroke::clip()
{
  if(x1 < 0)
    x1 = 0;
  if(y1 < 0)
    y1 = 0;
  if(x2 < 0)
    x2 = 0;
  if(y2 < 0)
    y2 = 0;
  if(x1 > Bmp::main->w - 1)
    x1 = Bmp::main->w - 1;
  if(y1 > Bmp::main->h - 1)
    y1 = Bmp::main->h - 1;
  if(x2 > Bmp::main->w - 1)
    x2 = Bmp::main->w - 1;
  if(y2 > Bmp::main->h - 1)
    y2 = Bmp::main->h - 1;
}

void Stroke::make_blitrect(int x1, int y1, int x2, int y2, int ox, int oy, int size, float zoom)
{
  int r = (size + 1) / 2 + 1;

  r *= zoom;

  x1 -= ox;
  y1 -= oy;
  x2 -= ox;
  y2 -= oy;

  x1 *= zoom;
  y1 *= zoom;
  x2 *= zoom;
  y2 *= zoom;

  if(x2 < x1)
    SWAP(x1, x2);
  if(y2 < y1)
    SWAP(y1, y2);

  x1 -= r;
  y1 -= r;
  x2 += r;
  y2 += r;

  blitx = x1;
  blity = y1;
  blitw = x2 - x1;
  blith = y2 - y1;

  if(blitw < 1)
    blitw = 1;
  if(blith < 1)
    blith = 1;
}

void Stroke::draw_brush(Brush *brush, Map *map, int x, int y, int c)
{
  int i;

  for(i = 0; i < brush->solid_count; i++)
  {
    map->setpixel(x + brush->solidx[i], y + brush->solidy[i], c);
  }
}

void Stroke::draw_brush_line(Brush *brush, Map *map, int x1, int y1, int x2, int y2, int c)
{
  int i;

  draw_brush(brush, map, x1, y1, c);
  draw_brush(brush, map, x2, y2, c);

  for(i = 0; i < brush->hollow_count; i++)
  {
    map->line(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::draw_brush_rect(Brush *brush, Map *map, int x1, int y1, int x2, int y2, int c)
{
  int i;

  for(i = 0; i < brush->hollow_count; i++)
  {
    map->rect(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::draw_brush_oval(Brush *brush, Map *map, int x1, int y1, int x2, int y2, int c)
{
  int i;

  for(i = 0; i < brush->hollow_count; i++)
  {
    map->oval(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::begin(Brush *brush, Map *map, int x, int y, int ox, int oy, float zoom)
{
  int r = brush->size / 2;
  int inc = brush->size & 1;
  int i;

  lastx = x;
  lasty = y;
  beginx = x;
  beginy = y;
  oldx = x;
  oldy = y;
  polycount = 0;

  x1 = x - (r + 1);
  y1 = y - (r + 1);
  x2 = x + (r + 1);
  y2 = y + (r + 1);

  map->clear(0);
  draw(brush, map, x, y, ox, oy, zoom);
  active = 1;
}

void Stroke::draw(Brush *brush, Map *map, int x, int y, int ox, int oy, float zoom)
{
  int r = brush->size / 2;
  int inc = brush->size & 1;
  int i;

  if(x - r - 1 < x1)
    x1 = x - r - 1;
  if(y - r - 1 < y1)
    y1 = y - r - 1;
  if(x + r + 1 > x2)
    x2 = x + r + 1;
  if(y + r + 1 > y2)
    y2 = y + r + 1;

  switch(type)
  {
    case 0:
      draw_brush_line(brush, map, x, y, lastx, lasty, 255);
      make_blitrect(x, y, lastx, lasty, ox, oy, brush->size, zoom);
      break;
    case 1:
    case 3:
      map->line(x, y, lastx, lasty, 255);
      make_blitrect(x, y, lastx, lasty, ox, oy, 1, zoom);
      polycachex[polycount] = x;
      polycachey[polycount] = y;
      polycount++;
      polycount &= 65535;
      oldx = x;
      oldy = y;
      break;
    case 2:
      draw_brush_line(brush, map, lastx, lasty, beginx, beginy, 0);
      draw_brush_line(brush, map, x, y, beginx, beginy, 255);
      make_blitrect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 4:
      draw_brush_rect(brush, map, lastx, lasty, beginx, beginy, 0);
      draw_brush_rect(brush, map, x, y, beginx, beginy, 255);
      make_blitrect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 5:
      map->rectfill(lastx, lasty, beginx, beginy, 0);
      map->rectfill(x, y, beginx, beginy, 255);
      make_blitrect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 6:
      draw_brush_oval(brush, map, lastx, lasty, beginx, beginy, 0);
      draw_brush_oval(brush, map, x, y, beginx, beginy, 255);
      make_blitrect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 7:
      map->ovalfill(lastx, lasty, beginx, beginy, 0);
      map->ovalfill(x, y, beginx, beginy, 255);
      make_blitrect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    default:
      break;
  }

  lastx = x;
  lasty = y;
}

void Stroke::end(Brush *brush, Map *map, int xx, int yy, int ox, int oy, float zoom)
{
  switch(type)
  {
    case 1:
    case 3:
      polycachex[polycount] = beginx;
      polycachey[polycount] = beginy;
      polycount++;
      polycount &= 65535;
      map->polyfill(polycachex, polycachey, polycount, x1, y1, x2, y2, 255);
      break;
    default:
      break;
  }
}

void Stroke::polyline(Map *map, int x, int y, int ox, int oy, float zoom)
{
  if(x - 1 < x1)
    x1 = x - 1;
  if(y - 1 < y1)
    y1 = y - 1;
  if(x + 1 > x2)
    x2 = x + 1;
  if(y + 1 > y2)
    y2 = y + 1;

  int i;
  for(i = 0; i < polycount - 1; i++)
    map->line(polycachex[i], polycachey[i],
              polycachex[i + 1], polycachey[i + 1], 255);

  map->line(oldx, oldy, lastx, lasty, 0);
  map->line(oldx, oldy, x, y, 255);

  make_blitrect(x1, y1, x2, y2, ox, oy, 1, zoom);

  lastx = x;
  lasty = y;
}

void Stroke::preview(Map *map, Bitmap *backbuf, int ox, int oy, float zoom)
{
  int x, y;

  clip();

  for(y = y1; y <= y2; y++)
  {
    int yy1 = y * zoom;
    int yy2 = yy1 + zoom - 1;
    unsigned char *p = map->row[y] + x1;
    for(x = x1; x <= x2; x++)
    {
      if(*p > 0 && is_edge(map, x, y))
      {
        int xx1 = x * zoom;
        int xx2 = xx1 + zoom - 1;
        backbuf->xor_rectfill(xx1 - ox * zoom, yy1 - oy * zoom, xx2 - ox * zoom, yy2 - oy * zoom);
      }
      p++;
    }
  }
}

void Stroke::render(Map *map, int edge)
{
  int x, y;

  if(edge == 0)
  {
    for(y = y1; y <= y2; y++)
    {
      for(x = x1; x <= x2; x++)
      {
        if(map->getpixel(x, y))
          Bmp::main->setpixel_solid(x, y, makecol(0, 0, 0), 0);
      }
    }
    active = 0;
    return;
  }

  render_count = 0;

  for(y = y1; y <= y2; y++)
  {
    for(x = x1; x <= x2; x++)
    {
      if(map->getpixel(x, y) && is_edge(map, x, y))
      {
        edgecachex[render_count] = x;
        edgecachey[render_count] = y;
        render_count++;
        render_count &= 0xFFFFF;
      }
    }
  }

  rendery = y1;
}

int Stroke::render_callback(Map *map, int edge, int ox, int oy, float zoom)
{
  if(render_count < 2)
  {
    active = 0;
    return 0;
  }

  int x, y;
  int endy = rendery + 32;
  if(endy > y2)
    endy = y2;

  for(y = rendery; y < endy; y++)
  {
    unsigned char *p = map->row[y] + x1;
    for(x = x1; x <= x2; x++)
    {
      if(*p == 0)
      {
        p++;
        continue;
      }

      int *cx = &edgecachex[0];
      int *cy = &edgecachey[0];
      float temp1 = fdist(x, y, *cx++, *cy++);
      int z = 0;
      int i;
      for(i = 1; i < render_count; i++)
      {
        float temp2 = fdist(x, y, *cx++, *cy++);
        if(temp2 < temp1)
        {
          z = i;
          temp1 = temp2;
        }
      }
      Bmp::main->setpixel_solid(x, y, makecol(0, 0 ,0), sdist(x, y, edgecachex[z], edgecachey[z], 0, edge, 0));
      p++;
    }
  }

  make_blitrect(x1, rendery, x2, endy, ox, oy, 1, zoom);

  rendery += 32;
  if(rendery > y2)
  {
    active = 0;
    return 0;
  }

  return 1;
}

