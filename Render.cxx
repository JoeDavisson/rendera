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

#include "Render.H"
#include "View.H"
#include "Stroke.H"
#include "Bitmap.H"
#include "Map.H"
#include "Brush.H"
#include "Tool.H"
#include "Gui.H"

namespace
{
  Bitmap *bmp;
  Map *map;
  Brush *brush;
  Stroke *stroke;
  View *view;

  bool is_edge(Map *map, const int &x, const int &y)
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

  int update(int y)
  {
    if(Fl::get_key(FL_Escape))
      return -1;

    if(!(y % 16))
      Gui::getView()->drawMain(1);

    return 0;
  }

  void render_solid()
  {
    int x, y;

    for(y = stroke->y1; y <= stroke->y2; y++)
    {
      unsigned char *p = map->row[y] + stroke->x1;

      for(x = stroke->x1; x <= stroke->x2; x++)
      {
        if(*p++)
          bmp->setpixel(x, y, brush->color, brush->trans);
      }

      if(update(y) < 0)
        break;
    }
  }

  void render_coarse()
  {
    int x, y, i;
    float soft_trans = 255;
    float j = (float)(3 << brush->edge);
    float soft_step = (float)(255 - brush->trans) /
                             (((3 << brush->edge) >> 1) + 1);
    int found = 0;

    for(i = 0; i < j; i++)
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
            bmp->setpixel(x, y, brush->color, soft_trans);
          if(!*s1 && d1)
            bmp->setpixel(x + 1, y, brush->color, soft_trans);
          if(!*s2 && d2)
            bmp->setpixel(x, y + 1, brush->color, soft_trans);
          if(!*s3 && d3)
            bmp->setpixel(x + 1, y + 1, brush->color, soft_trans);
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
              bmp->setpixel(x, y, brush->color, soft_trans);
          }
        }
      }

      if(update(y) < 0)
        break;
    }
  }
}

void Render::begin()
{
  view = Gui::getView();
  bmp = Bitmap::main;
  map = Map::main;
  brush = Brush::main;
  stroke = view->tool->stroke;

  view->tool->undo(0);

  switch(Gui::getPaintMode())
  {
    case SOLID:
      render_solid();
      break;
    case COARSE:
      render_coarse();
      break;
    case FINE:
      break;
  }
}

