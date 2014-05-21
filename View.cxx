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

static inline void grid_setpixel(const Bitmap *bmp, const int x, const int y,
                                 const int c, const int t)
{
  if(x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)
    return;

  int *p = bmp->row[y] + x;
  *p = blend_fast_ex(*p, c, t);
}

static inline void grid_hline(Bitmap *bmp, int x1, int y, int x2, const int c,
                              const int t)
{
  int x;

  if(x1 < 0)
    x1 = 0;
  if(y < 0)
    y = 0;

  int *p = bmp->row[y] + x1;

  for(x = x1; x <= x2; x++)
  {
    *p = blend_fast_ex(*p, c, t);
    p++;
  }
}

View::View(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Widget(x, y, w, h, label)
{
  group = g;
  ox = 0;
  oy = 0;
  zoom = 1;
  fit = 0;
  smooth = 0;
  moving = 0;
  grid = 0;
  gridx = 8;
  gridy = 8;
  backbuf = new Bitmap(Fl::w(), Fl::h());
  image = new Fl_RGB_Image((unsigned char *)backbuf->data, Fl::w(), Fl::h(), 4, 0);
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
          Bmp::main->rect(imgx, imgy, imgx + 4, imgy + 4, makecol(0, 0, 0), 0);
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
          Bmp::main->rect(imgx, imgy, imgx + 4, imgy + 4, makecol(0, 0, 0), 0);
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
          zoom_out(mousex, mousey);
      }
      else
      {
          zoom_in(mousex, mousey);
      }
      return 1;
  }
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  Fl_Widget::resize(x, y, w, h);
  draw_main();
}

void View::draw_move()
{
  backbuf->clear(makecol(0, 0, 0));
  Bmp::preview->blit(backbuf, 0, 0, px, py, Bmp::preview->w, Bmp::preview->h);
  // Bmp::preview->blit(backbuf, bx - px, by - py, bx, by, bw, bh);
  backbuf->rect(bx, by, bx + bw - 1, by + bh - 1, makecol(0, 0, 0), 0);
  backbuf->rect(bx + 1, by + 1, bx + bw - 2, by + bh - 2, makecol(255, 255, 255), 0);
  redraw();
}

void View::draw_main()
{
  int sw = w() / zoom;
  int sh = h() / zoom;
  sw += 2;
  sh += 2;

  if(sw > Bmp::main->w - ox)
    sw = Bmp::main->w - ox;
  if(sh > Bmp::main->h - oy)
    sh = Bmp::main->h - oy;

  Bitmap *temp = new Bitmap(sw, sh);
  Bmp::main->blit(temp, ox, oy, 0, 0, sw, sh);

  int dw = sw * zoom;
  int dh = sh * zoom;

  int overx = dw - w();
  int overy = dh - h();

  if(zoom < 2)
  {
    overx = 0;
    overy = 0;
  }

  backbuf->clear(makecol(0, 0, 0));
  if(smooth)
    temp->integer_stretch(backbuf, 0, 0, sw, sh, 0, 0, dw, dh, overx, overy);
  else
    temp->point_stretch(backbuf, 0, 0, sw, sh, 0, 0, dw, dh, overx, overy);

  if(grid)
    draw_grid();

  redraw();
}

void View::draw_grid()
{
  int x1, y1, x2, y2, d, i;
  int offx = 0, offy = 0;

  if((zoom < 2) && ((gridx <= zoom) || (gridy <= zoom)))
    return;

  x2 = w() - 1;
  y2 = h() - 1;

  d = 252 - zoom * 16;
  if(d < 192)
    d = 192;

  int zx = zoom * gridx;
  int zy = zoom * gridy;
//  int qx = (overscroll % gridx) * zoom;
//  int qy = (overscroll % gridy) * zoom;
  int qx = 0;
  int qy = 0;

  y1 = 0 - zy + (offy * zoom) + qy - (int)(oy * zoom) % zy;

  do
  {
    x1 = 0 - zx + (offx * zoom) + qx - (int)(ox * zoom) % zx;
    grid_hline(backbuf, x1, y1, x2, makecol(255, 255, 255), d);
    grid_hline(backbuf, x1, y1 + zy - 1, x2, makecol(0, 0, 0), d);
    i = 0;
    do
    {
      x1 = 0 - zx + (offx * zoom) + qx - (int)(ox * zoom) % zx;
      do
      {
        grid_setpixel(backbuf, x1, y1, makecol(255, 255, 255), d);
        grid_setpixel(backbuf, x1 + zx - 1, y1, makecol(0, 0, 0), d);
        x1 += zx;
      }
      while(x1 <= x2);
      y1++;
      i++;
    }
    while(i < zy);
  }
  while(y1 <= y2);
}

void View::begin_move()
{
  int dx = x() - group->x();
  int dy = y() - group->y();
  int ww = w();
  int hh = h();

  winaspect = (float)hh / ww;
  aspect = (float)Bmp::main->h / Bmp::main->w;

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

  delete Bmp::preview;
  Bmp::preview = new Bitmap(pw, ph);
  Bmp::main->fast_stretch(Bmp::preview,
                          0, 0, Bmp::main->w, Bmp::main->h,
                          0, 0, pw, ph);

  px = (ww - pw) >> 1;
  py = (hh - ph) >> 1;
  px += dx;
  py += dy;

  bw = ww * (((float)pw / zoom) / Bmp::main->w);
  bh = bw * winaspect;

  bx = ox * ((float)pw / Bmp::main->w) + px;
  by = oy * ((float)ph / Bmp::main->h) + py;

  // pos.x = bx + bw / 2;
  // pos.y = by + bh / 2;
  // warp mouse here... (unsupported in fltk)

//  redraw();
}

void View::move()
{
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

  ox = (bx - px) / ((float)pw / (Bmp::main->w));
  oy = (by - py) / ((float)ph / (Bmp::main->h));

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

void View::zoom_in(int x, int y)
{
  if(fit)
    return;

  zoom *= 2;
  if(zoom > 32)
  {
    zoom = 32;
  }
  else
  {
    ox += x / zoom;
    oy += y / zoom;
    if(ox > Bmp::main->w - w() / zoom)
      ox = Bmp::main->w - w() / zoom;
    if(oy > Bmp::main->h - h() / zoom)
      oy = Bmp::main->h - h() / zoom;
  }
  draw_main();
}

void View::zoom_out(int x, int y)
{
  if(fit)
    return;

  int oldzoom = zoom;
  zoom /= 2;
  if(zoom < 1)
  {
    zoom = 1;
  }
  else
  {
    ox -= x / oldzoom;
    oy -= y / oldzoom;
    if(ox < 0)
      ox = 0;
    if(oy < 0)
      oy = 0;
  }
  draw_main();
}

void View::zoom_fit(int fitting)
{
  if(!fitting)
  {
    fit = 0;
    zoom = 1;
    ox = 0;
    oy = 0;
    draw_main();
    return;
  }

  winaspect = (float)h() / w();
  aspect = (float)Bmp::main->h / Bmp::main->w;

  if(aspect < winaspect)
    zoom = ((float)w() / Bmp::main->w);
  else
    zoom = ((float)h() / Bmp::main->h);

  ox = 0;
  oy = 0;

  fit = 1;
  draw_main();
}

void View::zoom_one()
{
  fit = 0;
  zoom = 1;
  ox = 0;
  oy = 0;
  draw_main();
}

void View::draw()
{
  image->draw(x(), y(), w(), h(), 0, 0);
}

