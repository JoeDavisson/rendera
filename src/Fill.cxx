/*
Copyright (c) 2015 Joe Davisson.

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
#include "Map.H"
#include "Project.H"
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

  // determines if flood-fill is within range and sets transparency level
/*
  bool inRange(const int c1, const int c2,
                      const int range, int *trans)
  {
    int diff = std::abs(getl(c1) - getl(c2));

    *trans = diff * (256.0f / (range + 1));

    if(diff <= range)
      return true;
    else
      return false;
  }
*/

  // "grows" a 2x2 block based on a marching-squares type algorithm
  // used for feathering edges quickly
  inline void growBlock(unsigned char *s0, unsigned char *s1,
                        unsigned char *s2, unsigned char *s3)
  {
    int z = (*s0 << 0) + (*s1 << 1) + (*s2 << 2) + (*s3 << 3);

    switch (z)
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

  void fill(Bitmap *bmp, int x, int y, int new_color, int old_color, int range, int blur)
  {
    if(old_color == new_color)
      return;

    emptyStack();
    
    if(!push(x, y))
      return;

    // make copy
    Bitmap temp(bmp->w, bmp->h);
    bmp->blit(&temp, 0, 0, 0, 0, bmp->w, bmp->h);

    // blending map
    Map map_blend(bmp->w, bmp->h);
    map_blend.clear(0);

    // main map
    Map *map_main = Project::map; 
    map_main->clear(0);

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
        map_blend.setpixel(x1, y, 1);
        map_main->setpixel(x1, y, 255);

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
    float soft_trans = Project::brush->trans;
    float soft_step = (float)(255.0 - soft_trans) / (range + 1);
    bool found = false;
    int inc = 0;

    for(int y = ct; y <= cb; y++)
    {
      for(int x = cl; x <= cr; x++)
      {
        if(map_blend.getpixel(x, y))
        {
          bmp->setpixelSolid(x, y, new_color, soft_trans);
          map_main->setpixel(x, y, 255 - soft_trans);
        }
      }
    }

    for(int i = 0; i < range; i++)
    {
      inc++;

      for(int y = ct + (inc & 1); y < cb  - 1; y += 2)
      {
        for(int x = cl + (inc & 1); x < cr - 1; x += 2)
        {
          unsigned char *s0 = map_blend.row[y] + x;
          unsigned char *s1 = map_blend.row[y] + x + 1;
          unsigned char *s2 = map_blend.row[y + 1] + x;
          unsigned char *s3 = map_blend.row[y + 1] + x + 1;

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

          if(*s0 && !d0)
            map_main->setpixel(x, y, 255 - soft_trans);
          if(*s1 && !d1)
            map_main->setpixel(x + 1, y, 255 - soft_trans);
          if(*s2 && !d2)
            map_main->setpixel(x, y + 1, 255 - soft_trans);
          if(*s3 && !d3)
            map_main->setpixel(x + 1, y + 1, 255 - soft_trans);
        }
      }

      if(!found)
        break;

      soft_trans += soft_step;

      if(soft_trans > 255)
        break;
    }

    if(blur > 0)
      map_main->blur(blur);

    for(int y = 0; y < bmp->h; y++)
    {
      unsigned char *p = map_main->row[y];

      for(int x = 0; x < bmp->w; x++)
      {
        if(*p > 0)
          bmp->setpixel(x, y, new_color, scaleVal(255 - *p, Project::brush->trans));

        p++;
      }
    }
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
    int range = Gui::getFillRange();
    int blur = Gui::getFillBlur();
    Blend::set(Project::brush->blend);
    fill(Project::bmp, view->imgx, view->imgy, color, target, range, blur);
    //Project::bmp->fill(view->imgx, view->imgy, color, target, range);
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

