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
#include <cmath>
#include <vector>

#include "Blend.H"
#include "Bitmap.H"
#include "Brush.H"
#include "FilterMatrix.H"
#include "Fractal.H"
#include "Gamma.H"
#include "Gui.H"
#include "Inline.H"
#include "KDtree.H"
#include "Map.H"
#include "ExtraMath.H"
#include "Project.H"
#include "Render.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"

namespace
{
  Bitmap *bmp;
  Map *map;
  Brush *brush;
  Stroke *stroke;
  View *view;
  int color;
  int trans;

  // returns true if pixel is on a boundary
  inline bool isEdge(Map *map, const int x, const int y)
  {
    if(x < 1 || x >= map->w - 2 || y < 1 || y >= map->h - 2)
      return 0;

    if( *(map->row[y - 1] + x) &&
        *(map->row[y] + x - 1) &&
        *(map->row[y] + x + 1) &&
        *(map->row[y + 1] + x) )
      return 0;
    else
      return 1;
  }

  // used by fine airbrush
  inline int fineEdge(int x1, int y1, const int x2, const int y2,
               const int edge, const int trans)
  {
    x1 -= x2;
    y1 -= y2;

    const float d = __builtin_sqrtf(x1 * x1 + y1 * y1);
    const int s = (255 - trans) / (((3 << edge) >> 1) + 1);
    const int temp = 255 - s * d;

    return temp < trans ? trans : temp;
  }

  // "shrinks" a 2x2 block based on a marching-squares type algorithm
  // used for feathering edges
  inline void shrinkBlock(unsigned char *s0, unsigned char *s1,
                          unsigned char *s2, unsigned char *s3)
  {
    const int z = (*s0 << 0) | (*s1 << 1) | (*s2 << 2) | (*s3 << 3);

    switch(z)
    {
      case 0:
      case 15:
        return;
      case 7:
      case 14:
        *s1 = 0;
        *s2 = 0;
        return;
      case 11:
      case 13:
        *s0 = 0;
        *s3 = 0;
        return;
    }

    *s0 = *s1 = *s2 = *s3 = 0;
  }

  // "grows" a 2x2 block based on a marching-squares type algorithm
  // used for feathering edges
  inline void growBlock(unsigned char *s0, unsigned char *s1,
                        unsigned char *s2, unsigned char *s3)
  {
    const int z = (*s0 << 0) | (*s1 << 1) | (*s2 << 2) | (*s3 << 3);

    switch(z)
    {
      case 0:
      case 15:
        return;
      case 1:
      case 8:
        *s1 = 1;
        *s2 = 1;
        return;
      case 2:
      case 4:
        *s0 = 1;
        *s3 = 1;
        return;
    }

    *s0 = *s1 = *s2 = *s3 = 1;
  }

  // updates the viewport during rendering
  int update(int pos)
  {
    // user cancelled operation
    if(Fl::get_key(FL_Escape))
    {
      Gui::getView()->drawMain(true);
      return -1;
    }

    if((pos & 63) == 63)
    {
      view->drawMain(true);
      Fl::check();
    }

    return 0;
  }

  // solid
  void renderSolid()
  {
    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      unsigned char *p = map->row[y] + stroke->x1;

      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(*p++)
          bmp->setpixel(x, y, color, trans);
      }
    }
  }

  // antialiased
  void renderAntialiased()
  {
    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      unsigned char *p = map->row[y] + stroke->x1;

      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(*p > 0)
          bmp->setpixel(x, y, color, scaleVal((255 - *p), trans));

        p++;
      }
    }
  }

  // coarse airbrush
  void renderCoarse()
  {
    float soft_trans = 255;
    const int j = (3 << brush->coarse_edge);
    float soft_step = (float)(255 - trans) / ((j >> 1) + 1);
    bool found = false;

    for(int i = 0; i < j; i++)
    {
      for(int y = stroke->y1 + (i & 1); y < stroke->y2; y += 2)
      {
        for(int x = stroke->x1 + (i & 1); x < stroke->x2; x += 2)
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
            found = true;

          const unsigned char d0 = *s0;
          const unsigned char d1 = *s1;
          const unsigned char d2 = *s2;
          const unsigned char d3 = *s3;

          shrinkBlock(s0, s1, s2, s3);

          if(!*s0 && d0)
            bmp->setpixel(x, y, color, soft_trans);
          if(!*s1 && d1)
            bmp->setpixel(x + 1, y, color, soft_trans);
          if(!*s2 && d2)
            bmp->setpixel(x, y + 1, color, soft_trans);
          if(!*s3 && d3)
            bmp->setpixel(x + 1, y + 1, color, soft_trans);
        }
      }

      if(!found)
        break;

      soft_trans -= soft_step;

      if(soft_trans < trans)
      {
        soft_trans = trans;

        for(int y = stroke->y1; y <= stroke->y2; y++)
        {
          for(int x = stroke->x1; x <= stroke->x2; x++)
          {
            if(map->getpixel(x, y))
              bmp->setpixel(x, y, color, soft_trans);
          }
        }

        return;
      }

      if(update(i) < 0)
        break;
    }
  }

  // fine airbrush
  void renderFine()
  {
    int count = 0;

    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(map->getpixel(x, y) && isEdge(map, x, y))
        {
          stroke->edge_x[count] = x;
          stroke->edge_y[count] = y;
          count++;
          if(count > 0xfffff)
            break;
        }
      }
    }

    if(count == 0)
      return;

    KDtree::node_type test_node;
    KDtree::node_type *root, *found;
    KDtree::node_type *points = new KDtree::node_type[count];

    int best_dist;

    for(int i = 0; i < count; i++)
    {
      points[i].x[0] = stroke->edge_x[i];
      points[i].x[1] = stroke->edge_y[i];
    }

    root = make_tree(points, count, 0, 2);

    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      unsigned char *p = map->row[y] + stroke->x1;

      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(*p++ == 0)
          continue;

        test_node.x[0] = x;
        test_node.x[1] = y;
        found = 0;
        nearest(root, &test_node, 0, 2, &found, &best_dist);

        const int zx = found->x[0];
        const int zy = found->x[1];
        const int t = fineEdge(x, y, zx, zy, brush->fine_edge, trans);

        bmp->setpixel(x, y, color, t);
      }

      if(update(y) < 0)
        break;
    }

    delete[] points;
  }

  // gaussian blur
  void renderBlur()
  {
    const int w = (stroke->x2 - stroke->x1) + 1;
    const int h = (stroke->y2 - stroke->y1) + 1;

    Map temp(w, h);
    temp.clear(0);

    const int amount = (brush->blurry_edge + 2) * (brush->blurry_edge + 2) + 1;

    std::vector<int> kernel(amount);
    const int b = amount / 2;

    for(int x = 0; x < amount; x++)
    {
      const int xb = x - b;

      kernel[x] = 255 * std::exp(-((double)((xb) * (xb)) / ((b * b) / 2)));
    }

    // x direction
    for(int y = 0; y < h; y++)
    {
      const int y1 = y + stroke->y1;

      for(int x = -b; x < w + b; x++)
      {
        int xx = stroke->x1 + x - b;
        int val = 0;
        int div = 0;

        for(int i = 0; i < amount; i++)
        {
          if(xx >= 0 && xx < map->w)
          {
            val += *(map->row[y1] + xx) * kernel[i];
            div += kernel[i];
          }

          xx++;
        }

        if(div > 0)
          val /= div;

        temp.setpixel(x, y, val);
      }
    }

    // y direction
    for(int y = -b; y < h + b; y++)
    {
      for(int x = 0; x < w; x++)
      {
        int yy = y - b;
        int val = 0;
        int div = 0;

        for(int i = 0; i < amount; i++)
        {
          if(yy >= 0 && yy < h)
          {
            val += *(temp.row[yy] + x) * kernel[i];
            div += kernel[i];
          }

          yy++;
        }

        if(div > 0)
          val /= div;

        const int x1 = x + stroke->x1;
        const int y1 = y + stroke->y1;

        map->setpixel(x1, y1, val);
      }
    }

    // render
    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      unsigned char *p = map->row[y] + stroke->x1;

      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(*p > 0)
          bmp->setpixel(x, y, color, scaleVal((255 - *p), trans));

        p++;
      }

      if(update(y) < 0)
        break;
    }
  }

  // watercolor
  void renderWatercolor()
  {
    float soft_trans = trans;
    const int j = (2 << brush->watercolor_edge);
    float soft_step = (float)(255 - trans) / ((j >> 1) + 1);
    bool found = false;
    int inc = 0;

    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(map->getpixel(x, y))
          bmp->setpixel(x, y, color, trans);
      }
    }

    for(int i = 0; i < j; i++)
    {
      inc++;

      for(int y = stroke->y1 + (inc & 1); y < stroke->y2 - 1; y += 2)
      {
        for(int x = stroke->x1 + (inc & 1); x < stroke->x2 - 1; x += 2)
        {
          int yy = y + !(ExtraMath::rnd() & 3);

          unsigned char *s0 = map->row[yy] + x;
          unsigned char *s1 = map->row[yy] + x + 1;
          unsigned char *s2 = map->row[yy + 1] + x;
          unsigned char *s3 = map->row[yy + 1] + x + 1;

          *s0 &= 1;
          *s1 &= 1;
          *s2 &= 1;
          *s3 &= 1;

          if(*s0 | *s1 | *s2 | *s3)
            found = true;

          const unsigned char d0 = *s0;
          const unsigned char d1 = *s1;
          const unsigned char d2 = *s2;
          const unsigned char d3 = *s3;

          growBlock(s0, s1, s2, s3);

          if(*s0 & !(ExtraMath::rnd() & 15))
          {
            *s0 = 1;
            *s1 = 1;
            *s2 = 1;
            *s3 = 1;
            inc--;
            inc--;
          }

          if(*s0 && !d0)
            bmp->setpixel(x, yy, color, soft_trans);
          if(*s1 && !d1)
            bmp->setpixel(x + 1, yy, color, soft_trans);
          if(*s2 && !d2)
            bmp->setpixel(x, yy + 1, color, soft_trans);
          if(*s3 && !d3)
            bmp->setpixel(x + 1, yy + 1, color, soft_trans);
        }
      }

      if(!found)
        break;

      soft_trans += soft_step;

      if(soft_trans > 255)
        break;

      if(update(i) < 0)
        break;
    }
  }

  // chalk
  void renderChalk()
  {
    float soft_trans = 255;
    int j = (3 << brush->chalk_edge);
    float soft_step = (float)(255 - trans) / ((j >> 1) + 1);
    bool found = false;

    if(brush->chalk_edge == 0)
    {
      j = 1;
      soft_trans = trans;
    }

    for(int i = 0; i < j; i++)
    {
      for(int y = stroke->y1 + (i & 1); y < stroke->y2; y += 2)
      {
        for(int x = stroke->x1 + (i & 1); x < stroke->x2; x += 2)
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
            found = true;

          unsigned char d0 = *s0;
          unsigned char d1 = *s1;
          unsigned char d2 = *s2;
          unsigned char d3 = *s3;

          shrinkBlock(s0, s1, s2, s3);

          int t = 0;

          if(!*s0 && d0)
          {
            t = (int)soft_trans + (ExtraMath::rnd() & 63) - 32;
            if(t < 0)
              t = 0;
            if(t > 255)
              t = 255;
            bmp->setpixel(x, y, color, t);
          }

          if(!*s1 && d1)
          {
            t = (int)soft_trans + (ExtraMath::rnd() & 63) - 32;
            if(t < 0)
              t = 0;
            if(t > 255)
              t = 255;
            bmp->setpixel(x + 1, y, color, t);
          }

          if(!*s2 && d2)
          {
            t = (int)soft_trans + (ExtraMath::rnd() & 63) - 32;
            if(t < 0)
              t = 0;
            if(t > 255)
              t = 255;
            bmp->setpixel(x, y + 1, color, t);
          }

          if(!*s3 && d3)
          {
            t = (int)soft_trans + (ExtraMath::rnd() & 63) - 32;
            if(t < 0)
              t = 0;
            if(t > 255)
              t = 255;
            bmp->setpixel(x + 1, y + 1, color, t);
          }
        }
      }

      if(!found)
        break;

      soft_trans -= soft_step;

      if(soft_trans < trans)
      {
        soft_trans = trans;
        for(int y = stroke->y1; y <= stroke->y2; y++)
        {
          for(int x = stroke->x1; x <= stroke->x2; x++)
          {
            if(map->getpixel(x, y))
            {
              int t = (int)soft_trans + (ExtraMath::rnd() & 63) - 32;
              if(t < 0)
                t = 0;
              if(t > 255)
                t = 255;
              bmp->setpixel(x, y, color, t);
            }
          }
        }

        return;
      }

      if(update(i) < 0)
        break;
    }
  }

  // texture
  void renderTexture()
  {
    float soft_trans = 255;
    int j = (3 << brush->texture_edge);
    float soft_step = (float)(255 - trans) / ((j >> 1) + 1);
    bool found = false;

    int w = 256;
    int h = 256;

    Map plasma(w, h);
    Map marble(w, h);
    Map marbx(w, h);
    Map marby(w, h);

    Fractal::plasma(&plasma, (brush->texture_turb + 1) << 10);
    Fractal::plasma(&marbx, (brush->texture_turb + 1) << 10);
    Fractal::plasma(&marby, (brush->texture_turb + 1) << 10);
    Fractal::marble(&plasma, &marble, &marbx, &marby, brush->texture_marb << 2, 100, 0);

    Map *src = &marble;

    if(brush->texture_edge == 0)
    {
      j = 1;
      soft_trans = trans;
    }

    for(int i = 0; i < j; i++)
    {
      soft_trans -= soft_step;

      if(soft_trans < trans)
      {
        soft_trans = trans;
        break;
      }

      found = false;

      for(int y = stroke->y1 + (i & 1); y < stroke->y2; y += 2)
      {
        for(int x = stroke->x1 + (i & 1); x < stroke->x2; x += 2)
        {
          unsigned char *s0 = map->row[y] + x;
          unsigned char *s1 = map->row[y] + x + 1;
          unsigned char *s2 = map->row[y + 1] + x;
          unsigned char *s3 = map->row[y + 1] + x + 1;

          *s0 &= 1;
          *s1 &= 1;
          *s2 &= 1;
          *s3 &= 1;

          s0 = (map->row[y] + x);
          s1 = (map->row[y] + x + 1);
          s2 = (map->row[y + 1] + x);
          s3 = (map->row[y + 1] + x + 1);

          if(*s0 | *s1 | *s2 | *s3)
            found = true;

          unsigned char d0 = *s0;
          unsigned char d1 = *s1;
          unsigned char d2 = *s2;
          unsigned char d3 = *s3;

          shrinkBlock(s0, s1, s2, s3);

          if(!*s0 && d0)
            bmp->setpixel(x, y, color,
                     scaleVal(src->getpixel(x % w, y % h), soft_trans));

          if(!*s1 && d1)
            bmp->setpixel(x + 1, y, color,
                     scaleVal(src->getpixel((x + 1) % w, y % h), soft_trans));

          if(!*s2 && d2)
            bmp->setpixel(x, y + 1, color,
                     scaleVal(src->getpixel(x % w, (y + 1) % h), soft_trans));

          if(!*s3 && d3)
            bmp->setpixel(x + 1, y + 1, color,
                     scaleVal(src->getpixel((x + 1) % w, (y + 1) % h), soft_trans));
        }
      }

      if(!found)
        return;

      if(update(i) < 0)
        break;
    }

    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(map->getpixel(x, y))
        {
          bmp->setpixel(x, y, color,
                   scaleVal(src->getpixel(x % w, y % h), trans));
        }
      }
    }
  }

  // averaged
  void renderAverage()
  {
    int r = 0;
    int g = 0;
    int b = 0;
    int count = 0;

    for(int y = stroke->y1; y <= stroke->y2; y++)
    {
      unsigned char *p = map->row[y] + stroke->x1;

      for(int x = stroke->x1; x <= stroke->x2; x++)
      {
        if(*p++)
        {
          const int c = bmp->getpixel(x, y);
          rgba_type rgba = getRgba(c);
          r += rgba.r;
          g += rgba.g;
          b += rgba.b;
          count++; 
        }
      }
    }

    if(count == 0)
      return;

    r /= count;
    g /= count;
    b /= count;

    const int average = makeRgb(r, g, b);

    float soft_trans = 255;
    int j = (3 << brush->average_edge);
    float soft_step = (float)(255 - trans) / ((j >> 1) + 1);
    bool found = false;

    if(brush->average_edge == 0)
    {
      j = 1;
      soft_trans = trans;
    }

    for(int i = 0; i < j; i++)
    {
      for(int y = stroke->y1 + (i & 1); y < stroke->y2; y += 2)
      {
        for(int x = stroke->x1 + (i & 1); x < stroke->x2; x += 2)
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
            found = true;

          const unsigned char d0 = *s0;
          const unsigned char d1 = *s1;
          const unsigned char d2 = *s2;
          const unsigned char d3 = *s3;

          shrinkBlock(s0, s1, s2, s3);

          if(!*s0 && d0)
            bmp->setpixel(x, y, average, soft_trans);
          if(!*s1 && d1)
            bmp->setpixel(x + 1, y, average, soft_trans);
          if(!*s2 && d2)
            bmp->setpixel(x, y + 1, average, soft_trans);
          if(!*s3 && d3)
            bmp->setpixel(x + 1, y + 1, average, soft_trans);
        }
      }

      if(!found)
        break;

      soft_trans -= soft_step;

      if(soft_trans < trans)
      {
        soft_trans = trans;

        for(int y = stroke->y1; y <= stroke->y2; y++)
        {
          for(int x = stroke->x1; x <= stroke->x2; x++)
          {
            if(map->getpixel(x, y))
              bmp->setpixel(x, y, average, soft_trans);
          }
        }

        return;
      }

      if(update(i) < 0)
        break;
    }
  }
}

// start the rendering process
void Render::begin()
{
  view = Gui::getView();
  bmp = Project::bmp;
  map = Project::map;
  brush = Project::brush;
  stroke = Project::stroke;
  color = brush->color;
  trans = brush->trans;

  int size = 1;

  // kludge for tools that grow outward
  switch(Gui::getPaintMode())
  {
    case BLURRY:
      size = (3 << brush->blurry_edge);
      break;
    case WATERCOLOR:
      size = (3 << brush->watercolor_edge);
      break;
  }

  stroke->x1 -= size;
  stroke->y1 -= size;
  stroke->x2 += size;
  stroke->y2 += size;
  stroke->clip();

  stroke->makeBlitRect(stroke->x1, stroke->y1,
                       stroke->x2, stroke->y2,
                       view->ox, view->oy, 1, view->zoom);

  Project::undo->push();

  view->rendering = true;

  switch(Gui::getPaintMode())
  {
    case SOLID:
      renderSolid();
      break;
    case ANTIALIASED:
      renderAntialiased();
      break;
    case COARSE:
      renderCoarse();
      break;
    case FINE:
      renderFine();
      break;
    case BLURRY:
      renderBlur();
      break;
    case WATERCOLOR:
      renderWatercolor();
      break;
    case CHALK:
      renderChalk();
      break;
    case TEXTURE:
      renderTexture();
      break;
    case AVERAGE:
      renderAverage();
      break;
    default:
      break;
  }

  view->drawMain(true);
  view->rendering = false;
}

