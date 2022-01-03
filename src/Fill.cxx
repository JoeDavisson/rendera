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
#include <vector>

#include "Blend.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Fill.H"
#include "Gui.H"
#include "Inline.H"
#include "KDtree.H"
#include "Map.H"
#include "Project.H"
#include "Stroke.H"
#include "Undo.H"
#include "View.H"

namespace
{
  bool inbox(int x, int y, int x1, int y1, int x2, int y2)
  {
    if(x1 > x2)
      std::swap(x1, x2);
    if(y1 > y2)
      std::swap(y1, y2);

    if(x >= x1 && x <= x2 && y >= y1 && y <= y2)
      return 1;
    else
      return 0;
  }

  // returns true if pixel is on a boundary
  bool isEdge(Map *map, const int x, const int y)
  {
    if(x < 1 || x > map->w - 1 || y < 1 || y > map->h - 1)
      return 1;

    if( *(map->row[y - 1] + x) &&
        *(map->row[y] + x - 1) &&
        *(map->row[y] + x + 1) &&
        *(map->row[y + 1] + x) )
      return 0;
    else
      return 1;
  }

  // edge feathering
  int fineEdge(int x1, int y1, const int x2, const int y2,
               const int feather, const int trans)
  {
    x1 -= x2;
    y1 -= y2;

    const float d = __builtin_sqrtf(x1 * x1 + y1 * y1);
    const int s = (255 - trans) / (feather + 1);
    int temp = s * d;

    temp = clamp(temp, 255);

    return temp < trans ? trans : temp;
  }

  // flood-fill related stack routines
  const int stack_size = 4096;
  std::vector<int> stack_x(stack_size);
  std::vector<int> stack_y(stack_size);
  int sp = 0;

  inline bool pop(int *x, int *y)
  {
    if(sp > 0)
    {
      *x = stack_x[sp];
      *y = stack_y[sp];
      sp--;
      return true;
    }

    return false;
  }

  inline bool push(const int x, const int y)
  {
    if(sp < stack_size - 1)
    {
      sp++;
      stack_x[sp] = x;
      stack_y[sp] = y;
      return true;
    }

    return false;
  }

  inline void emptyStack()
  {
    int x, y;

    while(pop(&x, &y))
    {
      // loop until pop returns false
    }
  }

  void fill(int x, int y, int new_color, int old_color, int feather)
  {
    if(old_color == new_color)
      return;

    emptyStack();
    
    if(!push(x, y))
      return;

    Bitmap *bmp = Project::bmp;
    Bitmap temp(bmp->w, bmp->h);
    bmp->blit(&temp, 0, 0, 0, 0, bmp->w, bmp->h);
    Map *map = Project::map;
    map->clear(0);

    int cl = bmp->cl;
    int cr = bmp->cr;
    int ct = bmp->ct;
    int cb = bmp->cb;

    while(pop(&x, &y))
    {    
      int x1 = x;

      while(x1 >= cl && (temp.getpixel(x1, y) == old_color))
        x1--;

      x1++;

      bool span_t = false;
      bool span_b = false;

      while(x1 <= cr && (temp.getpixel(x1, y) == old_color))
      {
        temp.setpixel(x1, y, new_color);
        map->setpixel(x1 - bmp->cl, y - bmp->ct, 255);

        if((!span_t && y > ct) && (temp.getpixel(x1, y - 1) == old_color))
        {
          if(!push(x1, y - 1))
            return;

          span_t = true;
        }
        else if((span_t && y > ct) && (temp.getpixel(x1, y - 1) != old_color))
        {
          span_t = false;
        }

        if((!span_b && y < cb) && (temp.getpixel(x1, y + 1) == old_color))
        {
          if(!push(x1, y + 1))
            return;

          span_b = true;
        }
        else if((span_b && y < cb) && (temp.getpixel(x1, y + 1) != old_color))
        {
          span_b = false;
        } 

        x1++;
      }
    }

    if(feather == 0)
    {
      temp.blit(bmp, 0, 0, 0, 0, temp.w, temp.h);
      return;
    }

    Stroke *stroke = Project::stroke.get();
    int count = 0;

    Gui::showProgress((cb - ct) + 1);

    for(y = ct; y <= cb; y++)
    {
      for(x = cl; x <= cr; x++)
      {
        if(map->getpixel(x - cl, y - ct) && isEdge(map, x - cl, y - ct))
        {
          stroke->edgecachex[count] = x;
          stroke->edgecachey[count] = y;
          count++;

          if(count > 0xFFFFF)
            break;
        }
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    if(count == 0)
      return;

    KDtree::node_type test_node;
    KDtree::node_type *root, *found;
    KDtree::node_type *points = new KDtree::node_type[count];

    int best_dist;

    for(int i = 0; i < count; i++)
    {
      points[i].x[0] = stroke->edgecachex[i];
      points[i].x[1] = stroke->edgecachey[i];
    }

    root = make_tree(points, count, 0, 2);
    Gui::showProgress((cb - ct) + 1);

    for(y = ct; y <= cb; y++)
    {
      for(x = cl; x <= cr; x++)
      {
        if(map->getpixel(x - cl, y - ct) == 255)
        {
          bmp->setpixel(x, y, new_color);
          continue;
        }

        test_node.x[0] = x;
        test_node.x[1] = y;
        found = 0;
        nearest(root, &test_node, 0, 2, &found, &best_dist);

        const int zx = found->x[0];
        const int zy = found->x[1];
        const int c1 = bmp->getpixel(x, y);
        const int t = fineEdge(x, y, zx, zy, feather, 0);

        bmp->setpixel(x, y, Blend::trans(c1, new_color, t));
      }

      if(Gui::updateProgress(y) < 0)
        break;
    }

    Gui::hideProgress();
    delete points;
  }
}

Fill::Fill()
{
}

Fill::~Fill()
{
}

void Fill::push(View *view)
{
  if(inbox(view->imgx, view->imgy, Project::bmp->cl, Project::bmp->ct,
                                   Project::bmp->cr, Project::bmp->cb))
  {
    Undo::push();
    int target = Project::bmp->getpixel(view->imgx, view->imgy);
    rgba_type rgba = getRgba(Project::brush->color);
    int color = makeRgba(rgba.r, rgba.g, rgba.b, 255 - Project::brush->trans);
    Blend::set(Project::brush->blend);
    fill(view->imgx, view->imgy, color, target, Gui::getFillFeather());
    Blend::set(Blend::TRANS);
    view->drawMain(true);
  }
}

void Fill::drag(View *)
{
}

void Fill::release(View *)
{
}

void Fill::move(View *)
{
}

void Fill::key(View *)
{
}

void Fill::done(View *, int)
{
}

void Fill::redraw(View *)
{
}

bool Fill::isActive()
{
  return false;
}

void Fill::reset()
{
}

