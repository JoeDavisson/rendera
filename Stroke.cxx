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

#include "Stroke.H"
#include "Bitmap.H"
#include "Blend.H"
#include "Map.H"
#include "Brush.H"
#include "Project.H"

#define XOR_VALUE(x, y) ( ((x & 1) ^ (y & 1)) ? 0x00FFFFFF : 0x00808080)

namespace
{
  void keepSquare(int x1, int y1, int *x2, int *y2)
  {
    const int px = (*x2 >= x1) ? 1 : 0;
    const int py = (*y2 >= y1) ? 2 : 0;

    const int dx = x1 - *x2;
    const int dy = y1 - *y2;

    if(abs(dy) > abs(dx))
    {
      switch(px + py)
      {
        case 0:
          *x2 = x1 - dy;
          break;
        case 1:
          *x2 = x1 + dy;
          break;
        case 2:
          *x2 = x1 + dy;
          break;
        case 3:
          *x2 = x1 - dy;
          break;
      }
    }
    else
    {
      switch (px + py)
      {
        case 0:
          *y2 = y1 - dx;
          break;
        case 1:
          *y2 = y1 + dx;
          break;
        case 2:
          *y2 = y1 + dx;
          break;
        case 3:
          *y2 = y1 - dx;
          break;
      }
    }
  }
}

Stroke::Stroke()
{
  polycachex = new int[65536];
  polycachey = new int[65536];
  edgecachex = new int[0x100000];
  edgecachey = new int[0x100000];
  polycount = 0;
  type = 0;
  origin = 0;
  constrain = 0;
  x1 = 0;
  y1 = 0;
  x2 = 0;
  y2 = 0;
  blitx = 0;
  blity = 0;
  blitw = 0;
  blith = 0;
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
  if(x2 > Project::bmp->w - 1)
    x2 = Project::bmp->w - 1;
  if(y2 > Project::bmp->h - 1)
    y2 = Project::bmp->h - 1;
}

void Stroke::sizeLinear(int bx, int by, int x, int y)
{
  if(bx > x)
  {
    x1 = x - 48;
    x2 = bx + 48;
  }
  else
  {
    x1 = bx - 48;
    x2 = x + 48;
  }

  if(by > y)
  {
    y1 = y - 48;
    y2 = by + 48;
  }
  else
  {
    y1 = by - 48;
    y2 = y + 48;
  }
}

void Stroke::makeBlitRect(int x1, int y1, int x2, int y2,
                           int ox, int oy, int size, float zoom)
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
    std::swap(x1, x2);
  if(y2 < y1)
    std::swap(y1, y2);

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

void Stroke::size(int x1, int y1, int x2, int y2)
{
  if(x1 > x2)
    std::swap(x1, x2);
  if(y1 > y2)
    std::swap(y1, y2);

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
}

void Stroke::drawBrush(int x, int y, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->solid_count; i++)
  {
    map->setpixel(x + brush->solidx[i], y + brush->solidy[i], c);
  }
}

void Stroke::drawBrushLine(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  drawBrush(x1, y1, c);
  drawBrush(x2, y2, c);

  for(i = 0; i < brush->hollow_count; i++)
  {
    map->line(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::drawBrushRect(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->hollow_count; i++)
  {
    map->rect(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::drawBrushOval(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->hollow_count; i++)
  {
    map->oval(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::drawBrushAA(int x, int y, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->solid_count; i++)
  {
    map->setpixelAA(x + (brush->solidx[i] << 2),
                    y + (brush->solidy[i] << 2), c);
  }
}

void Stroke::drawBrushLineAA(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->solid_count; i++)
  {
    map->lineAA(x1 + brush->solidx[i],
                y1 + brush->solidy[i],
                x2 + brush->solidx[i],
                y2 + brush->solidy[i], c);
  }
}

void Stroke::drawBrushRectAA(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->solid_count; i++)
  {
    map->rectAA(x1 + brush->solidx[i],
                y1 + brush->solidy[i],
                x2 + brush->solidx[i],
                y2 + brush->solidy[i], c);
  }
}

void Stroke::drawBrushOvalAA(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int i;

  for(i = 0; i < brush->solid_count; i++)
  {
    map->ovalAA(x1 + brush->solidx[i],
                y1 + brush->solidy[i],
                x2 + brush->solidx[i],
                y2 + brush->solidy[i], c);
  }
}

void Stroke::begin(int x, int y, int ox, int oy, float zoom)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int r = brush->size / 2;

  lastx = x;
  lasty = y;
  beginx = x;
  beginy = y;
  oldx = x;
  oldy = y;

  if(Project::bmp->clone_moved)
  {
    Project::bmp->clone_dx = x - Project::bmp->clone_x;
    Project::bmp->clone_dy = y - Project::bmp->clone_y;
    Project::bmp->clone_moved = 0;
  }

  polycount = 0;

  x1 = x - (r + 1);
  y1 = y - (r + 1);
  x2 = x + (r + 1);
  y2 = y + (r + 1);

  map->clear(0);
  draw(x, y, ox, oy, zoom);
}

void Stroke::draw(int x, int y, int ox, int oy, float zoom)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  int r = brush->size / 2;
  int w = 0, h = 0;

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
      drawBrushLine(x, y, lastx, lasty, 255);
      makeBlitRect(x, y, lastx, lasty, ox, oy, brush->size, zoom);
      polycachex[polycount] = x;
      polycachey[polycount] = y;
      polycount++;
      polycount &= 65535;
      break;
    case 1:
      map->line(x, y, lastx, lasty, 255);
      makeBlitRect(x, y, lastx, lasty, ox, oy, 1, zoom);
      polycachex[polycount] = x;
      polycachey[polycount] = y;
      polycount++;
      polycount &= 65535;
      oldx = x;
      oldy = y;
      break;
    case 3:
      map->line(oldx, oldy, lastx, lasty, 0);
      makeBlitRect(x, y, lastx, lasty, ox, oy, 1, zoom);
      polycachex[polycount] = x;
      polycachey[polycount] = y;
      polycount++;
      polycount &= 65535;
      oldx = x;
      oldy = y;
      break;
    case 2:
      if(origin)
      {
        w = (lastx - beginx);
        h = (lasty - beginy);
        drawBrushLine(beginx - w, beginy - h, beginx + w, beginy + h, 0);
        w = (x - beginx);
        h = (y - beginy);
        drawBrushLine(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        sizeLinear(beginx - w, beginy - h, x + w, y + h);
      }
      else
      {
        drawBrushLine(lastx, lasty, beginx, beginy, 0);
        drawBrushLine(x, y, beginx, beginy, 255);
      }
      makeBlitRect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 4:
      if(constrain)
        keepSquare(beginx, beginy, &x, &y);

      if(origin)
      {
        w = (lastx - beginx);
        h = (lasty - beginy);
        drawBrushRect(beginx - w, beginy - h, beginx + w, beginy + h, 0);
        w = (x - beginx);
        h = (y - beginy);
        drawBrushRect(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        sizeLinear(beginx - w, beginy - h, x + w, y + h);
      }
      else
      {
        drawBrushRect(lastx, lasty, beginx, beginy, 0);
        drawBrushRect(x, y, beginx, beginy, 255);
        sizeLinear(beginx, beginy, x, y);
      }

      makeBlitRect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 5:
      if(constrain)
        keepSquare(beginx, beginy, &x, &y);

      if(origin)
      {
        w = (lastx - beginx);
        h = (lasty - beginy);
        map->rectfill(beginx - w, beginy - h, beginx + w, beginy + h, 0);
        w = (x - beginx);
        h = (y - beginy);
        map->rectfill(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        sizeLinear(beginx - w, beginy - h, x + w, y + h);
      }
      else
      {
        map->rectfill(lastx, lasty, beginx, beginy, 0);
        map->rectfill(x, y, beginx, beginy, 255);
        sizeLinear(beginx, beginy, x, y);
      }
      makeBlitRect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 6:
      if(constrain)
        keepSquare(beginx, beginy, &x, &y);

      if(origin)
      {
        w = (lastx - beginx);
        h = (lasty - beginy);
        drawBrushOval(beginx - w, beginy - h, beginx + w, beginy + h, 0);
        w = (x - beginx);
        h = (y - beginy);
        drawBrushOval(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        sizeLinear(beginx - w, beginy - h, x + w, y + h);
      }
      else
      {
        drawBrushOval(lastx, lasty, beginx, beginy, 0);
        drawBrushOval(x, y, beginx, beginy, 255);
        sizeLinear(beginx, beginy, x, y);
      }
      makeBlitRect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    case 7:
      if(constrain)
        keepSquare(beginx, beginy, &x, &y);

      if(origin)
      {
        w = (lastx - beginx);
        h = (lasty - beginy);
        map->ovalfill(beginx - w, beginy - h, beginx + w, beginy + h, 0);
        w = (x - beginx);
        h = (y - beginy);
        map->ovalfill(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        sizeLinear(beginx - w, beginy - h, x + w, y + h);
      }
      else
      {
        map->ovalfill(lastx, lasty, beginx, beginy, 0);
        map->ovalfill(x, y, beginx, beginy, 255);
        sizeLinear(beginx, beginy, x, y);
      }
      makeBlitRect(x1, y1, x2, y2, ox, oy, brush->size, zoom);
      break;
    default:
      break;
  }

  lastx = x;
  lasty = y;
}

void Stroke::end(int x, int y)
{
  Map *map = Project::map;
  Brush *brush = Project::brush;

  map->thick_aa = 0;

  int w = 0, h = 0;
  int i;

  if(Project::bmp->clone)
  {
    w = (x2 - x1);
    h = (y2 - y1);
    if(Bitmap::clone_buffer)
      delete Bitmap::clone_buffer;
    Bitmap::clone_buffer = new Bitmap(w, h);
    Project::bmp->blit(Bitmap::clone_buffer, x1, y1, 0, 0, w, h);
  }

  if(brush->aa)
  {
    map->clear(0);

    switch(type)
    {
      case 0:
        map->thick_aa = 1;
        for(i = 1; i < polycount; i++)
        {
          drawBrushLineAA(polycachex[i], polycachey[i],
                          polycachex[i - 1], polycachey[i - 1], 255);
        }
        break;
      case 2:
        map->thick_aa = 1;
        if(origin)
        {
          w = (x - beginx);
          h = (y - beginy);
          drawBrushLineAA(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        }
        else
        {
          drawBrushLineAA(x, y, beginx, beginy, 255);
        }
        break;
      case 1:
      case 3:
        polycachex[polycount] = beginx;
        polycachey[polycount] = beginy;
        polycount++;
        polycount &= 65535;
        map->polyfillAA(polycachex, polycachey, polycount, x1, y1, x2, y2, 255);
        break;
      case 4:
        map->thick_aa = 1;
        if(constrain)
          keepSquare(beginx, beginy, &x, &y);

        if(origin)
        {
          w = (x - beginx);
          h = (y - beginy);
          drawBrushRectAA(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        }
        else
        {
          drawBrushRectAA(x, y, beginx, beginy, 255);
        }
        break;
      case 5:
        if(constrain)
          keepSquare(beginx, beginy, &x, &y);

        if(origin)
        {
          w = (x - beginx);
          h = (y - beginy);
          map->rectfillAA(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        }
        else
        {
          map->rectfillAA(x, y, beginx, beginy, 255);
        }
        break;
      case 6:
        map->thick_aa = 1;
        if(constrain)
          keepSquare(beginx, beginy, &x, &y);

        if(origin)
        {
          w = (x - beginx);
          h = (y - beginy);
          drawBrushOvalAA(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        }
        else
        {
          drawBrushOvalAA(x, y, beginx, beginy, 255);
        }
        break;
      case 7:
        if(constrain)
          keepSquare(beginx, beginy, &x, &y);

        if(origin)
        {
          w = (x - beginx);
          h = (y - beginy);
          map->ovalfillAA(beginx - w, beginy - h, beginx + w, beginy + h, 255);
        }
        else
        {
          map->ovalfillAA(x, y, beginx, beginy, 255);
        }
        break;
      default:
        break;
    }
  }
  else
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
}

void Stroke::polyline(int x, int y, int ox, int oy, float zoom)
{
  Map *map = Project::map;

  if(x - 1 < x1)
    x1 = x - 1;
  if(y - 1 < y1)
    y1 = y - 1;
  if(x + 1 > x2)
    x2 = x + 1;
  if(y + 1 > y2)
    y2 = y + 1;

  map->line(oldx, oldy, lastx, lasty, 0);

  int i;
  for(i = 0; i < polycount - 1; i++)
    map->line(polycachex[i], polycachey[i],
              polycachex[i + 1], polycachey[i + 1], 255);

  map->line(oldx, oldy, x, y, 255);

  makeBlitRect(x1, y1, x2, y2, ox, oy, 1, zoom);

  lastx = x;
  lasty = y;
}

void Stroke::preview(Bitmap *backbuf, int ox, int oy, float zoom)
{
  Map *map = Project::map;

  clip();

  ox *= zoom;
  oy *= zoom;

  float yy1 = (float)y1 * zoom;
  float yy2 = yy1 + zoom - 1;
  int x, y;

  // prevent overun when zoomed out
  if(x2 > map->w - 2)
    x2 = map->w - 2;
  if(y2 > map->h - 2)
    y2 = map->h - 2;

  for(y = y1; y <= y2; y++)
  {
    unsigned char *p = map->row[y] + x1;
    float xx1 = (float)x1 * zoom;
    float xx2 = xx1 + zoom - 1;

    for(x = x1; x <= x2; x++)
    {
      if(*p++)
        backbuf->xorRectfill(xx1 - ox, yy1 - oy, xx2 - ox, yy2 - oy);

      xx1 += zoom;
      xx2 += zoom;
    }

    yy1 += zoom;
    yy2 += zoom;
  }
}

