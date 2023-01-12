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

#include <FL/Fl_Double_Window.H>

#include "Bitmap.H"
#include "Brush.H"
#include "Clone.H"
#include "Inline.H"
#include "Map.H"
#include "Project.H"
#include "Stroke.H"
#include "View.H"
#include "Widget.H"

namespace Clone
{
  int x = 0;
  int y = 0;
  int dx = 0;
  int dy = 0;
  int state = 0;
  bool active = false;
  bool moved = false;
  Bitmap *buffer_bmp = 0;

  Fl_Double_Window *window = 0;
  Widget *preview = 0;
}

void Clone::init()
{
  window = new Fl_Double_Window(400, 400, "Clone Preview");
  preview = new Widget(window, 8, 8, 384, 384, "", 0, 0, 0);
  window->set_non_modal();
  window->end();
}

// change the clone target
void Clone::move(int xx, int yy)
{
  if(moved)
  {
    dx = xx - x; 
    dy = yy - y; 
    moved = false;
  }
}

// set clone buffer bitmap to the correct size
void Clone::refresh(int x1, int y1, int x2, int y2)
{
  const int w = x2 - x1;
  const int h = y2 - y1;

  delete buffer_bmp;
  buffer_bmp = new Bitmap(w, h);
  Project::bmp->blit(buffer_bmp, x1, y1, 0, 0, w, h);
}

void Clone::show(bool enabled)
{
  if(enabled == true)
    window->show();
  else
    window->hide();
}

void Clone::update(View *view)
{
  Bitmap *bmp = Project::bmp;
  Map *map = Project::map;

  const float zoom = 1;

  int sw = preview->bitmap->w;
  int sh = preview->bitmap->h;

  int sx = 0;
  int sy = 0;

  if(state == RESET || state == PLACED)
  {
    sx = x;
    sy = y;
  }
  else
  {
    sx = view->imgx - dx;
    sy = view->imgy - dy;
  }

  for(int yy = 0; yy < sh; yy++)
  {
    for(int xx = 0; xx < sw; xx++)
    {
      int tempx = xx + sx - sw / 2;
      int tempy = yy + sy - sh / 2;
 
      if(tempx >= 0 && tempx < bmp->w && tempy >= 0 && tempy < bmp->h)
      {
        preview->bitmap->setpixel(xx, yy, bmp->getpixel(tempx, tempy));
      }
      else
      {
        preview->bitmap->setpixel(xx, yy, getFltkColor(FL_BACKGROUND2_COLOR));
      }
    }
  }

  int xx1 = 0;
  int yy1 = 0;
  int xx2 = sw - 1;
  int yy2 = sh - 1;

  for(int yy = yy1; yy <= yy2; yy++)
  {
    int *p = preview->bitmap->row[yy];
    int ym = yy + sy - sh / 2 + dy;

    for(int xx = xx1; xx <= xx2; xx++)
    {
      int xm = xx + sx - sw / 2 + dx;

      if(map->getpixel(xm, ym))
      {
        *p = blendFast(*p, makeRgb(255, 0, 192), 128);
      }
      else
      {
        if(zoom >= 1)
        {
          // shade edges for contrast
          int xmod = xx % (int)zoom;
          int ymod = yy % (int)zoom;

          int tx1 = xmod;
          int tx2 = ((int)zoom - 1 - xmod);
          int ty1 = ymod;
          int ty2 = ((int)zoom - 1 - ymod);

          tx1 = tx1 == 0 ? 0 : 255;
          tx2 = tx2 == 0 ? 0 : 255;
          ty1 = ty1 == 0 ? 0 : 255;
          ty2 = ty2 == 0 ? 0 : 255;

          const int checker = visibleColor(xx, yy);

          if(map->getpixel(xm - 1, ym - 1))
            *p = blendFast(*p, checker, tx1 | ty1);

          if(map->getpixel(xm, ym - 1))
            *p = blendFast(*p, checker, ty1);

          if(map->getpixel(xm + 1, ym - 1))
            *p = blendFast(*p, checker, tx2 | ty1);

          if(map->getpixel(xm - 1, ym))
            *p = blendFast(*p, checker, tx1);

          if(map->getpixel(xm + 1, ym))
            *p = blendFast(*p, checker, tx2);

          if(map->getpixel(xm - 1, ym + 1))
            *p = blendFast(*p, checker, tx1 | ty2);

          if(map->getpixel(xm, ym + 1))
            *p = blendFast(*p, checker, ty2);

          if(map->getpixel(xm + 1, ym + 1))
            *p = blendFast(*p, checker, tx2 | ty2);
        }
      }

      p++;
    }
  }

  int x1 = sw / 2;
  int y1 = sh / 2;

  // draw crosshair
  preview->bitmap->rect(x1 - 8, y1 - 1, x1 + 8, y1 + 1, makeRgb(0, 0, 0), 0);
  preview->bitmap->rect(x1 - 1, y1 - 8, x1 + 1, y1 + 8, makeRgb(0, 0, 0), 0);
  preview->bitmap->xorRectfill(x1 - 7, y1, x1 + 7, y1);
  preview->bitmap->xorRectfill(x1, y1 - 7, x1, y1 + 7);
  preview->bitmap->rectfill(x1 - 7, y1, x1 + 7, y1, makeRgb(255, 0, 192), 128);
  preview->bitmap->rectfill(x1, y1 - 7, x1, y1 + 7, makeRgb(255, 0, 192), 128);

  preview->redraw();
}

