/*
Copyright (c) 2025 Joe Davisson.

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
#include "CheckBox.H"
#include "Fill.H"
#include "FillOptions.H"
#include "Group.H"
#include "Gui.H"
#include "Inline.H"
#include "InputInt.H"
#include "KDtree.H"
#include "Map.H"
#include "Progress.H"
#include "Project.H"
#include "Stroke.H"
#include "Undo.H"
#include "View.H"

Fill::Fill()
{
  stack_size = 16384;
  stack_x = new int [stack_size];
  stack_y = new int [stack_size];
  sp = 0;
}

Fill::~Fill()
{
  delete[] stack_x;
  delete[] stack_y;
}

bool Fill::inbox(int x, int y, int x1, int y1, int x2, int y2)
{
  if (x1 > x2)
    std::swap(x1, x2);
  if (y1 > y2)
    std::swap(y1, y2);

  if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
    return true;
  else
    return false;
}

// finds edges
bool Fill::isEdge(Map *map, const int x, const int y)
{
  // special case
  if (x < 1 || x >= map->w - 1 || y < 1 || y >= map->h - 1)
  {
//    if (*(map->row[y] + x))
//      return 0;
//    else
      return 1;
  }

  if ( *(map->row[y - 1] + x) &&
      *(map->row[y] + x - 1) &&
      *(map->row[y] + x + 1) &&
      *(map->row[y + 1] + x) )
  {
    return 0;
  }
    else
  {
    return 1;
  }
}

// edge feathering
int Fill::fineEdge(int x1, int y1, const int x2, const int y2,
             const int feather, const int trans)
{
  x1 -= x2;
  y1 -= y2;

  const float d = std::sqrt(x1 * x1 + y1 * y1);
  const int s = (255 - trans) / (feather + 1);
  int temp = s * d;

  temp = clamp(temp, 255);

  return temp < trans ? trans : temp;
}

// flood-fill related stack routines
bool Fill::pop(int *x, int *y)
{
  if (sp > 0)
  {
    *x = stack_x[sp];
    *y = stack_y[sp];
    sp--;
    return true;
  }

  return false;
}

bool Fill::push(const int x, const int y)
{
  if (sp < stack_size - 1)
  {
    sp++;
    stack_x[sp] = x;
    stack_y[sp] = y;
    return true;
  }

  return false;
}

void Fill::clear()
{
  int x, y;

  while (pop(&x, &y))
  {
    // loop until pop returns false
  }
}

bool Fill::inRange(const int c1, const int c2, const int range)
{
  if ((std::sqrt(diff32(c1, c2)) / 2) <= range)
    return true;
  else
    return false;
}

void Fill::fill(int x, int y, int new_color, int old_color, int range, int feather, int color_only)
{
  if (old_color == new_color)
    return;

  clear();
  
  if (!push(x, y))
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

  while (pop(&x, &y))
  {    
    int x1 = x;

    while (x1 >= cl && inRange(temp.getpixel(x1, y), old_color, range))
      x1--;

    x1++;

    bool span_t = false;
    bool span_b = false;

    while (x1 <= cr && inRange(temp.getpixel(x1, y), old_color, range))
    {
      temp.setpixel(x1, y, new_color);
      map->setpixel(x1 - bmp->cl, y - bmp->ct, 255);

      if ((!span_t && y > ct) && inRange(temp.getpixel(x1, y - 1),
                                        old_color, range))
      {
        if (!push(x1, y - 1))
          return;

        span_t = true;
      }
      else if ((span_t && y > ct) && !inRange(temp.getpixel(x1, y - 1),
                                             old_color, range))
      {
        span_t = false;
      }

      if ((!span_b && y < cb) && inRange(temp.getpixel(x1, y + 1),
                                        old_color, range))
      {
        if (!push(x1, y + 1))
          return;

        span_b = true;
      }
      else if ((span_b && y < cb) && !inRange(temp.getpixel(x1, y + 1),
                                             old_color, range))
      {
        span_b = false;
      } 

      x1++;
    }
  }

  if (feather == 0)
  {
    if (color_only)
    {
      for (int y = 0; y < temp.h; y++)
      {
        for (int x = 0; x < temp.w; x++)
        {
          if (temp.getpixel(x, y) == new_color)
          {
            const int c1 = bmp->getpixel(x, y);
            const int c = Blend::trans(c1, temp.getpixel(x, y), 255 - geta(c1));
            bmp->setpixel(x, y, Blend::keepLum(c, getl(c1)));
          }
        }
      }
    }
      else
    {
      temp.blit(bmp, 0, 0, 0, 0, temp.w, temp.h);
    }

    return;
  }

  Stroke *stroke = Project::stroke;
  int count = 0;

  Progress::show((cb - ct) + 1);

  for (y = ct; y <= cb; y++)
  {
    for (x = cl; x <= cr; x++)
    {
      if (map->getpixel(x - cl, y - ct) && isEdge(map, x - cl, y - ct))
      {
        stroke->edge_x[count] = x;
        stroke->edge_y[count] = y;
        count++;

        if (count > 0xfffff)
          break;
      }
    }

    if (Progress::update(y) < 0)
      return;
  }

  if (count == 0)
    return;

  KDtree::node_type test_node;
  KDtree::node_type *root, *found;
  std::vector<KDtree::node_type> points(count);

  int best_dist;

  int tl = 0xfffff;
  int tr = 0;
  int tt = 0xfffff;
  int tb = 0;

  for (int i = 0; i < count; i++)
  {
    const int ex = stroke->edge_x[i];
    const int ey = stroke->edge_y[i];

    if (ex < tl)
      tl = ex;

    if (ex > tr)
      tr = ex;

    if (ey < tt)
      tt = ey;

    if (ey > tb)
      tb = ey;

    points[i].x[0] = ex;
    points[i].x[1] = ey;
    points[i].x[2] = 0;
  }

  tl -= feather;
  tr += feather;
  tt -= feather;
  tb += feather;

  if (tl < cl)
    tl = cl; 

  if (tr > cr)
    tr = cr; 

  if (tt < ct)
    tt = ct; 

  if (tb > cb)
    tb = cb; 

  if (tl > tr)
    std::swap(tl, tr);

  if (tt > tb)
    std::swap(tt, tb);

  root = KDtree::build(&points[0], count, 0);
  Progress::show((cb - ct) + 1);

  for (y = ct; y <= cb; y++)
  {
    for (x = cl; x <= cr; x++)
    {
      if (x > tl || x < tr || y < tt || y > tb)
        if (map->getpixel(x - cl, y - ct) == 255)
          bmp->setpixel(x, y, new_color);
    }
  }

  for (y = tt; y <= tb; y++)
  {
    for (x = tl; x <= tr; x++)
    {
      if (map->getpixel(x - cl, y - ct) == 255)
        continue;

      test_node.x[0] = x;
      test_node.x[1] = y;
      test_node.x[2] = 0;
      found = 0;
      KDtree::nearest(root, &test_node, &found, &best_dist, 0);

      const int zx = found->x[0];
      const int zy = found->x[1];
      const int c1 = bmp->getpixel(x, y);
      const int t = fineEdge(x, y, zx, zy, feather, 0);

      if (color_only)
      {
        const int c = Blend::trans(c1, new_color, t);
        bmp->setpixel(x, y, Blend::keepLum(c, getl(c1)));
      }
        else
      {
        bmp->setpixel(x, y, Blend::trans(c1, new_color, t));
      }
    }

    if (Progress::update(y) < 0)
      break;
  }

  Progress::hide();
}

void Fill::push(View *view)
{
  if (inbox(view->imgx, view->imgy, Project::bmp->cl, Project::bmp->ct,
                                   Project::bmp->cr, Project::bmp->cb))
  {
    Project::undo->push();
    Blend::set(Blend::TRANS);

    rgba_type rgba = getRgba(Project::brush->color);
    int color = makeRgba(rgba.r, rgba.g, rgba.b, 255 - Project::brush->trans);
    int target = Project::bmp->getpixel(view->imgx, view->imgy);

    fill(view->imgx,
         view->imgy,
         color,
         target,
         Gui::fill->getRange(),
         Gui::fill->getFeather(),
         Gui::fill->getColorOnly());

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

void Fill::redraw(View *view)
{
  view->drawMain(true);
}

bool Fill::isActive()
{
  return false;
}

void Fill::reset()
{
}

