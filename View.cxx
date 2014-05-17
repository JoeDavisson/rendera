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

View::View(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Widget(x, y, w, h, label)
{
  group = g;
  image = 0;
  backbuf = 0;
  ox = 0;
  oy = 0;
  zoom = 1;
  moving = 0;
  resize(group->x() + x, group->y() + y, w, h);
}

View::~View()
{
}

int View::handle(int event)
{
  mousex = Fl::event_x() - x();
  mousey = Fl::event_y() - y();
  if(mousex < 0)
    mousex = 0;
  if(mousex >  w() - 1)
    mousex = w() - 1;
  if(mousey < 0)
    mousey = 0;
  if(mousey >  h() - 1)
    mousey = h() - 1;
  imgx = mousex / zoom + ox;
  imgy = mousey / zoom + oy;
  int button = Fl::event_button();

  switch(event)
  {
    case FL_PUSH:
      switch(button)
      {
        case 1:
          //bmp->main->setpixel_solid(imgx, imgy, makecol(0, 0, 0), 0);
          bmp->main->rect(imgx, imgy, imgx + 4, imgy + 4, makecol(0, 0, 0), 0);
          draw_main();
          return 1;
        case 2:
          if(moving == 0)
          {
            moving = 1;
            begin_move();
            draw_move();
            return 1;
          }
          return 0;
      } 
    case FL_DRAG:
      switch(button)
      {
        case 1:
          //bmp->main->setpixel_solid(imgx, imgy, makecol(0, 0, 0), 0);
          bmp->main->rect(imgx, imgy, imgx + 4, imgy + 4, makecol(0, 0, 0), 0);
          draw_main();
          return 1;
        case 2:
          if(moving == 1)
          {
            move();
            draw_move();
            return 1;
          }
      } 
    case FL_RELEASE:
      moving = 0;
      draw_main();
      return 1;
    case FL_MOUSEWHEEL:
      moving = 0;
      if(Fl::event_dy() >= 0)
      {
        int oldzoom = zoom;
        zoom /= 2;
        if(zoom < 1)
        {
          zoom = 1;
        }
        else
        {
          ox -= mousex / oldzoom;
          oy -= mousey / oldzoom;
          if(ox < 0)
            ox = 0;
          if(oy < 0)
            oy = 0;
        }
      }
      else
      {
        zoom *= 2;
        if(zoom > 16)
        {
          zoom = 16;
        }
        else
        {
          ox += mousex / zoom;
          oy += mousey / zoom;
          if(ox > bmp->main->w - w() / zoom - 1)
            ox = bmp->main->w - w() / zoom - 1;
          if(oy > bmp->main->h - h() / zoom - 1)
            oy = bmp->main->h - h() / zoom - 1;
        }
      }
      draw_main();
      return 1;
  }
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  delete image;
  delete backbuf;
  backbuf = new Bitmap(w, h);
  image = new Fl_RGB_Image((unsigned char *)backbuf->data, w, h, 4, 0);
  draw_main();
  Fl_Widget::resize(x, y, w, h);
}

void View::draw_move()
{
  backbuf->clear(makecol(0, 0, 0));
  bmp->preview->blit(backbuf, 0, 0, px, py, bmp->preview->w, bmp->preview->h);
//  bmp->preview->blit(backbuf, bx - px, by - py, bx, by, bw, bh);
  backbuf->rect(bx, by, bx + bw - 1, by + bh - 1, makecol(0, 0, 0), 0);
  redraw();
}

void View::draw_main()
{
  int ww = w();
  int hh = h();

  int sw = ww / zoom;
  int sh = hh / zoom;
  sw += 2;
  sh += 2;

  if(sw > bmp->main->w)
    sw = bmp->main->w;
  if(sh > bmp->main->h)
    sh = bmp->main->h;

  Bitmap *temp = new Bitmap(sw, sh);
  bmp->main->blit(temp, ox, oy, 0, 0, sw, sh);

  int dw = sw * zoom;
  int dh = sh * zoom;

  int overx = dw - w();
  int overy = dh - h();

  if(zoom < 2)
  {
    overx = 0;
    overy = 0;
  }

  backbuf->clear(makecol(0, 255, 0));
  temp->point_stretch(backbuf, 0, 0, sw, sh, 0, 0, dw, dh, overx, overy);

  redraw();
}

void View::begin_move()
{
  int dx = x() - group->x();
  int dy = y() - group->y();
  int ww = w();
  int hh = h();

  winaspect = (double)hh / ww;
  aspect = (double)bmp->main->h / bmp->main->w;

  pw = ww;
  ph = hh;

  if(aspect < winaspect)
    ph = ww * aspect;
  else
    pw = hh / aspect;
  if(pw > ww)
    pw = ww;
  if(ph > hh)
    ph = hh;

  delete bmp->preview;
  bmp->preview = new Bitmap(pw, ph);

  bmp->main->point_stretch(bmp->preview,
                  0, 0,
                  bmp->main->w, bmp->main->h,
                  0, 0, bmp->preview->w, bmp->preview->h, 0, 0);

  px = (ww - pw) >> 1;
  py = (hh - ph) >> 1;
  px += dx;
  py += dy;

  bw = ww * (((double)pw / zoom) / bmp->main->w);
  bh = bw * winaspect;

  bx = ox * ((double)pw / bmp->main->w) + px;
  by = oy * ((double)ph / bmp->main->h) + py;

//  pos.x = bx + bw / 2;
//  pos.y = by + bh / 2;

//  ClientToScreen(mainHwnd, &pos);
//  SetCursorPos(pos.x, pos.y);
  redraw();
}

void View::move()
{
//  bw = w() * ((pw / zoom) / bmp->main->w);
//  bh = bw * winaspect;

  bx = mousex - (bw >> 1);
  by = mousey - (bh >> 1);

  if(bx < px)
    bx = px;
  if(bx > px + pw - bw - 1)
    bx = px + pw - bw - 1;
  if(by < py)
    by = py;
  if(by > py + ph - bh - 1)
    by = py + ph - bh - 1;

  ox = (bx - px) / ((double)pw / (bmp->main->w));
  oy = (by - py) / ((double)ph / (bmp->main->h));

  if(bw > pw)
  {
    bx = px;
    bw = pw;
    ox = 0;
  }
  if(bh > ph)
  {
    by = py;
    bh = ph;
    oy = 0;
  }
  if(bw < 1)
    bw = 1;
  if(bh < 1)
    bh = 1;
}

void View::draw()
{
  image->draw(x(), y());
}

