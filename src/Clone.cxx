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
#include "Clone.H"
#include "Inline.H"
#include "Project.H"
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

  Fl_Double_Window *window;
  Widget *preview;
}

void Clone::init()
{
  window = new Fl_Double_Window(400, 400, "Clone Preview");
  preview = new Widget(window, 8, 8, 384, 384, "", 0, 0, 0);
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

void Clone::show(int enabled)
{
  if(enabled == true)
    window->show();
  else
    window->hide();
}

void Clone::update(View *view)
{
  Bitmap *bmp = Project::bmp;

  int sw = preview->w();
  int sh = preview->h();

  int sx = 0;
  int sy = 0;

  for(int yy = 0; yy < sh; yy++)
  {
    for(int xx = 0; xx < sw; xx++)
    {
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

      const int c = bmp->getpixel(xx + sx - sw / 2, yy + sy - sh / 2);

      preview->bitmap->setpixel(xx, yy, c);
    }
  }

  const int x1 = preview->w() / 2;
  const int y1 = preview->h() / 2;

  preview->bitmap->rect(x1 - 8, y1 - 1, x1 + 8, y1 + 1, makeRgb(0, 0, 0), 0);
  preview->bitmap->rect(x1 - 1, y1 - 8, x1 + 1, y1 + 8, makeRgb(0, 0, 0), 0);
  preview->bitmap->xorRectfill(x1 - 7, y1, x1 + 7, y1);
  preview->bitmap->xorRectfill(x1, y1 - 7, x1, y1 + 7);
  preview->bitmap->rectfill(x1 - 7, y1, x1 + 7, y1, makeRgb(255, 255, 255), 128);
  preview->bitmap->rectfill(x1, y1 - 7, x1, y1 + 7, makeRgb(255, 255, 255), 128);

  preview->redraw();
}

void Clone::hide()
{
  window->hide();
}

