/*
Copyright (c) 2021 Joe Davisson.

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

#include <algorithm>
#include <cstdlib>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Clone.H"
#include "Gui.H"
#include "Inline.H"
#include "Map.H"
#include "ExtraMath.H"
#include "Project.H"
#include "Render.H"
#include "Stroke.H"
#include "View.H"

namespace
{
  // keeps dimensions equal, for drawing circles/squares
  void keepSquare(int x1, int y1, int *x2, int *y2)
  {
    const int px = (*x2 >= x1) ? 1 : 0;
    const int py = (*y2 >= y1) ? 2 : 0;

    const int dx = x1 - *x2;
    const int dy = y1 - *y2;

    if(ExtraMath::abs(dy) > ExtraMath::abs(dx))
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

  inline bool isEdge(Map *map, const int x, const int y)
  {
    if(!map->getpixel(x, y - 1) ||
       !map->getpixel(x - 1, y) ||
       !map->getpixel(x + 1, y) ||
       !map->getpixel(x, y + 1))
    {
      return true;
    }

    return false;
  }

  inline bool isOuterEdge(Map *map, const int x, const int y)
  {
//    if(!map->getpixel(x, y))
//    {
      if(map->getpixel(x, y - 1) > 0 ||
         map->getpixel(x - 1, y) > 0 ||
         map->getpixel(x + 1, y) > 0 ||
         map->getpixel(x, y + 1) > 0)
      {
        return true;
      }
//    }

    return false;
  }

/*
  inline bool isEdge(Map *map, const int x, const int y)
  {
    if((map->getpixel(x, y) &&
       (!map->getpixel(x - 1, y) ||
       !map->getpixel(x + 1, y) ||
       !map->getpixel(x, y - 1) ||
       !map->getpixel(x, y + 1))))
      return true;
    else
      return false;
  }
*/
}

Stroke::Stroke()
{
  poly_x = new int[0x10000];
  poly_y = new int[0x10000];
  edge_x = new int[0x100000];
  edge_y = new int[0x100000];

  poly_count = 0;
  type = 0;
  origin = false;
  origin_always = false;
  constrain = false;
  constrain_always = false;
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
  delete[] poly_x;
  delete[] poly_y;
  delete[] edge_x;
  delete[] edge_y;
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

  for(int i = 0; i < brush->solid_count; i++)
    map->setpixel(x + brush->solidx[i], y + brush->solidy[i], c);
}

void Stroke::drawBrushLine(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  drawBrush(x1, y1, c);
  drawBrush(x2, y2, c);

  for(int i = 0; i < brush->hollow_count; i++)
  {
    map->line(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::drawBrushRect(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  for(int i = 0; i < brush->hollow_count; i++)
  {
    map->rect(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::drawBrushOval(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  for(int i = 0; i < brush->hollow_count; i++)
  {
    map->oval(x1 + brush->hollowx[i], y1 + brush->hollowy[i],
              x2 + brush->hollowx[i], y2 + brush->hollowy[i], c);
  }
}

void Stroke::drawBrushAA(int x, int y, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  for(int i = 0; i < brush->solid_count; i++)
  {
/*
    map->setpixelAA(((x + brush->solidx[i]) << 2) - 1,
                    ((y + brush->solidy[i]) << 2), c);
    map->setpixelAA(((x + brush->solidx[i]) << 2) + 1,
                    ((y + brush->solidy[i]) << 2), c);
    map->setpixelAA(((x + brush->solidx[i]) << 2),
                    ((y + brush->solidy[i]) << 2) - 1, c);
    map->setpixelAA(((x + brush->solidx[i]) << 2),
                    ((y + brush->solidy[i]) << 2) + 1, c);
*/
    map->setpixelAA(((x + brush->solidx[i]) << 2) - 1,
                    ((y + brush->solidy[i]) << 2) - 1, c);
    map->setpixelAA(((x + brush->solidx[i]) << 2) + 1,
                    ((y + brush->solidy[i]) << 2) - 1, c);
    map->setpixelAA(((x + brush->solidx[i]) << 2) - 1,
                    ((y + brush->solidy[i]) << 2) + 1, c);
    map->setpixelAA(((x + brush->solidx[i]) << 2) + 1,
                    ((y + brush->solidy[i]) << 2) + 1, c);
  }
}

void Stroke::drawBrushLineAA(int x1, int y1, int x2, int y2, int c)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  for(int i = 0; i < brush->solid_count; i++)
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

  for(int i = 0; i < brush->solid_count; i++)
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

  for(int i = 0; i < brush->solid_count; i++)
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

  poly_count = 0;

  x1 = x - (r + 1);
  y1 = y - (r + 1);
  x2 = x + (r + 1);
  y2 = y + (r + 1);

  Clone::move(x, y);

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
    case FREEHAND:
    {
      drawBrushLine(x, y, lastx, lasty, 255);
      makeBlitRect(x, y, lastx, lasty, ox, oy, brush->size, zoom);

      // without this slowly-drawn strokes don't look as nice
      if(brush->aa && ((x == lastx) ^ (y == lasty)))
        return;

      poly_x[poly_count] = x;
      poly_y[poly_count] = y;
      poly_count++;
      poly_count &= 0xffff;

      break;
    }

    case REGION:
    {
      map->line(x, y, lastx, lasty, 255);
      makeBlitRect(x, y, lastx, lasty, ox, oy, 1, zoom);

      // without this slowly-drawn strokes don't look as nice
      if(brush->aa && ((x == lastx) ^ (y == lasty)))
        return;

      poly_x[poly_count] = x;
      poly_y[poly_count] = y;
      poly_count++;
      poly_count &= 0xffff;
      oldx = x;
      oldy = y;

      break;
    }

    case LINE:
    {
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
    }

    case POLYGON:
    {
      map->line(oldx, oldy, lastx, lasty, 0);
//      map->rectfill(oldx - 1, oldy - 1, oldx + 1, oldy + 1, 255);
      makeBlitRect(x, y, lastx, lasty, ox, oy, 1, zoom);
      poly_x[poly_count] = x;
      poly_y[poly_count] = y;
      poly_count++;
      poly_count &= 0xffff;
      oldx = x;
      oldy = y;

      break;
    }

    case RECT:
    {
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
    }

    case FILLED_RECT:
    {
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
    }

    case OVAL:
    {
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
    }

    case FILLED_OVAL:
    {
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
    }

    default:
      break;
  }

  lastx = x;
  lasty = y;
}

void Stroke::end(int x, int y)
{
  Brush *brush = Project::brush;
  Map *map = Project::map;

  map->thick_aa = 0;

  int w = 0, h = 0;

  Clone::refresh(x1, y1, x2, y2);

  if(brush->aa)
  {
    // antialiased drawing
    map->clear(0);

    switch(type)
    {
      case FREEHAND:
      {
        poly_x[poly_count] = x;
        poly_y[poly_count] = y;
        poly_count++;
        poly_count &= 0xffff;

        if(poly_count > 2)
        {
          if(brush->size < 5)
            map->thick_aa = 1;

          for(int i = 1; i < poly_count; i++)
          {
            drawBrushLineAA(poly_x[i], poly_y[i],
                            poly_x[i - 1], poly_y[i - 1], 255);
          }
        }
        else
        {
          map->thick_aa = 1;
          drawBrushAA(x, y, 255);
        }

        break;
      }

      case REGION:
      {
        poly_x[poly_count] = beginx;
        poly_y[poly_count] = beginy;
        poly_count++;
        poly_count &= 0xffff;
        map->polyfillAA(poly_x, poly_y, poly_count, y1, y2, 255);

        break;
      }

      case LINE:
      {
        if(brush->size == 1)
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
      }

      case POLYGON:
      {
        map->clear(0);
        poly_x[poly_count] = beginx;
        poly_y[poly_count] = beginy;
        poly_count++;
        poly_count &= 0xffff;
        if(poly_count > 3)
          map->polyfillAA(poly_x, poly_y, poly_count, y1, y2, 255);

        break;
      }

      case RECT:
      {
        if(brush->size == 1)
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
      }

      case FILLED_RECT:
      {
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
      }

      case OVAL:
      {
        if(brush->size == 1)
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
      }

      case FILLED_OVAL:
      {
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
      }

      default:
        break;
    }
  }
  else
  {
    // normal drawing for filled polygons
    // (other types have already been drawn correctly)
    switch(type)
    {
      case REGION:
      case POLYGON:
      {
        map->clear(0);
        poly_x[poly_count] = beginx;
        poly_y[poly_count] = beginy;
        poly_count++;
        poly_count &= 0xffff;
        if(poly_count > 3)
        {
          map->polyfill(poly_x, poly_y, poly_count, y1, y2, 255);

          for(int i = 0; i < poly_count - 1; i++)
          {
            map->line(poly_x[i], poly_y[i],
                poly_x[i + 1], poly_y[i + 1], 255);
          }
        }
        break;
      }

      default:
        break;
    }
  }
}

void Stroke::polyLine(int x, int y, int ox, int oy, float zoom)
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
  map->line(oldx, oldy, x, y, 255);

  for(int i = 0; i < poly_count - 1; i++)
  {
    map->line(poly_x[i], poly_y[i],
              poly_x[i + 1], poly_y[i + 1], 255);
  }

  makeBlitRect(x1, y1, x2, y2, ox, oy, 1, zoom);

  lastx = x;
  lasty = y;
}

// use paint color for preview
void Stroke::previewPaint(View *view)
{
  Bitmap *backbuf = view->backbuf;
  Map *map = Project::map;

  const float zoom = view->zoom;
  const bool bgr_order = view->bgr_order;
  const int zr = (int)((1.0 / zoom) * 65536);
  const int color = convertFormat(Project::brush->color, bgr_order);
  const int trans = Project::brush->trans;
  
  int ox = view->ox;
  int oy = view->oy;

  ox *= zoom;
  oy *= zoom;

  int xx1 = x1 * zoom - ox;
  int yy1 = y1 * zoom - oy;
  int xx2 = x2 * zoom - ox + zoom - 1;
  int yy2 = y2 * zoom - oy + zoom - 1;

  clip();

  // draw brushstroke preview
  if(xx1 < 0)
    xx1 = 0;

  if(yy1 < 0)
    yy1 = 0;

  if(xx1 >= backbuf->w - 1)
    xx1 = backbuf->w - 1;

  if(yy1 >= backbuf->h - 1)
    yy1 = backbuf->h - 1;

  if(xx2 < 0)
    xx2 = 0;

  if(yy2 < 0)
    yy2 = 0;

  if(xx2 >= backbuf->w - 1)
    xx2 = backbuf->w - 1;

  if(yy2 >= backbuf->h - 1)
    yy2 = backbuf->h - 1;

  for(int y = yy1; y <= yy2; y++)
  {
    const int ym = ((y + oy) * zr) >> 16;
    int *p = backbuf->row[y] + xx1;

    for(int x = xx1; x <= xx2; x++)
    {
      const int xm = ((x + ox) * zr) >> 16;

      if(map->getpixel(xm, ym))
      {
        if(Clone::active == false)
        {
          *p = blendFast(*p, color, trans);
        }
        else
        {
          *p = blendFast(*p, 0xffffff, 128);
        }
      }
      else
      {
        if(zoom >= 1)
        {
          // shade edges for contrast
          int xmod = x % (int)zoom;
          int ymod = y % (int)zoom;
          int t = 255 / zoom;

          int tx1 = 128 + xmod * t;
          int tx2 = 128 + ((int)zoom - 1 - xmod) * t;
          int ty1 = 128 + ymod * t;
          int ty2 = 128 + ((int)zoom - 1 - ymod) * t;

          tx1 = clamp(tx1, 255);
          tx2 = clamp(tx2, 255);
          ty1 = clamp(ty1, 255);
          ty2 = clamp(ty2, 255);

          if(map->getpixel(xm - 1, ym))
            *p = blendFast(*p, 0x000000, tx1);

          if(map->getpixel(xm + 1, ym))
            *p = blendFast(*p, 0x000000, tx2);

          if(map->getpixel(xm, ym - 1))
            *p = blendFast(*p, 0x000000, ty1);

          if(map->getpixel(xm, ym + 1))
            *p = blendFast(*p, 0x000000, ty2);

          if(map->getpixel(xm - 1, ym - 1))
            *p = blendFast(*p, 0x000000, std::max(tx1, ty1));

          if(map->getpixel(xm + 1, ym - 1))
            *p = blendFast(*p, 0x000000, std::max(tx2, ty1));

          if(map->getpixel(xm - 1, ym + 1))
            *p = blendFast(*p, 0x000000, std::max(tx1, ty2));

          if(map->getpixel(xm + 1, ym + 1))
            *p = blendFast(*p, 0x000000, std::max(tx2, ty2));
        }
      }

      p++;
    }
  }
}

void Stroke::previewSelection(View *view)
{
  Bitmap *backbuf = view->backbuf;
  Bitmap *select_bmp = Project::select_bmp;
  
  const float zoom = view->zoom;
  const bool bgr_order = view->bgr_order;
  const int zr = (int)((1.0 / zoom) * 65536);
  const int trans = Project::brush->trans;
  const int use_alpha = Gui::getSelectionAlpha();

  int ox = view->ox;
  int oy = view->oy;

  ox *= zoom;
  oy *= zoom;

  int xx1 = x1 * zoom - ox;
  int yy1 = y1 * zoom - oy;
  int xx2 = x2 * zoom - ox;
  int yy2 = y2 * zoom - oy;

  // these preserve sign
  int xx3 = xx1;
  int yy3 = yy1;

  clip();

  if(xx1 < 0)
    xx1 = 0;

  if(yy1 < 0)
    yy1 = 0;

  if(xx1 >= backbuf->w - 1)
    xx1 = backbuf->w - 1;

  if(yy1 >= backbuf->h - 1)
    yy1 = backbuf->h - 1;

  if(xx2 < 0)
    xx2 = 0;

  if(yy2 < 0)
    yy2 = 0;

  if(xx2 >= backbuf->w - 1)
    xx2 = backbuf->w - 1;

  if(yy2 >= backbuf->h - 1)
    yy2 = backbuf->h - 1;

  for(int y = yy1; y <= yy2; y++)
  {
    int ym = ((y - yy3) * zr) >> 16;
    
    int *p = backbuf->row[y] + xx1;

    for(int x = xx1; x <= xx2; x++)
    {
      const int xm = ((x - xx3) * zr) >> 16;
      const int c = convertFormat(select_bmp->getpixel(xm, ym), bgr_order);

      if(use_alpha)
        *p = blendFast(*p, c, scaleVal(255 - geta(c), trans));
      else
        *p = blendFast(*p, c, trans);

      p++;
    }
  }
}

