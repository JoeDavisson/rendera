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
  *p = blend_fast_solid(*p, c, t);
}

static inline void grid_hline(Bitmap *bmp, int x1, int y, int x2,
                              const int c, const int t)
{
  int x;

  if(x1 < 0)
    x1 = 0;
  if(y < 0)
    y = 0;

  int *p = bmp->row[y] + x1;

  for(x = x1; x <= x2; x++)
  {
    *p = blend_fast_solid(*p, c, t);
    p++;
  }
}

static int inbox(int x, int y, int x1, int y1, int x2, int y2)
{
  if(x1 > x2)
    SWAP(x1, x2);
  if(y1 > y2)
    SWAP(y1, y2);

  if(x >= x1 && x <= x2 && y >= y1 && y <= y2)
    return 1;
  else
    return 0;
}

static void absrect(int *x1, int *y1, int *x2, int *y2)
{
  if(*x1 > *x2)
    SWAP(*x1, *x2);
  if(*y1 > *y2)
    SWAP(*y1, *y2);

  if(*x1 < Bitmap::main->cl)
    *x1 = Bitmap::main->cl;
  if(*y1 < Bitmap::main->ct)
    *y1 = Bitmap::main->ct;
  if(*x2 > Bitmap::main->cr)
    *x2 = Bitmap::main->cr;
  if(*y2 > Bitmap::main->cb)
    *y2 = Bitmap::main->cb;
}

View::View(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Widget(x, y, w, h, label)
{
  group = g;
  ox = 0;
  oy = 0;
  zoom = 1;
  fit = 0;
  moving = 0;
  grid = 0;
  gridx = 8;
  gridy = 8;
  oldimgx = 0;
  oldimgy = 0;
  tool = 0;
  tool_started = 0;
  crop_resize_started = 0;
  stroke = new Stroke();
  backbuf = new Bitmap(Fl::w(), Fl::h());
  alphadata = new unsigned char[Fl::w() * Fl::h() * 3];
  image_part = new Fl_RGB_Image((unsigned char *)backbuf->data, Fl::w(), Fl::h(), 4, 0);
  image_full = new Fl_RGB_Image((unsigned char *)alphadata, Fl::w(), Fl::h(), 3, 0);
  take_focus();
  resize(group->x() + x, group->y() + y, w, h);
}

View::~View()
{
}

int View::handle(int event)
{
  mousex = Fl::event_x() - x();
  mousey = Fl::event_y() - y();
  imgx = mousex / zoom + ox;
  imgy = mousey / zoom + oy;

  //int button = Fl::event_button();
  // do is this way to prevent multiple button presses
  button1 = Fl::event_button1() ? 1 : 0;
  button2 = Fl::event_button2() ? 2 : 0;
  button3 = Fl::event_button3() ? 4 : 0;
  button = button1 | button2 | button3;
  dclick = Fl::event_clicks() ? 1 : 0;
  shift = Fl::event_shift() ? 1 : 0;

  switch(event)
  {
    case FL_FOCUS:
      return 1;
    case FL_UNFOCUS:
      return 1;
    case FL_ENTER:
      return 1;
    case FL_PUSH:
      take_focus();

      switch(button)
      {
        window()->make_current();
        fl_overlay_clear();

        case 1:
          if(shift)
          {
            Bitmap::clone_x = imgx;
            Bitmap::clone_y = imgy;
            Bitmap::clone_moved = 1;
            break;
          }

          switch(tool)
          {
            case 0:
              brush_push();
              break;
            case 1:
              crop_push();
              break;
            case 2:
              getcolor_push();
              break;
            case 3:
              offset_push();
              break;
          }

          break;
        case 2:
          if(tool_started == 0 && moving == 0)
          {
            begin_move();
            moving = 1;
            break;
          }
      } 
      return 1;
    case FL_DRAG:
      take_focus();
      window()->make_current();
      fl_overlay_clear();

      switch(button)
      {
        case 1:
          switch(tool)
          {
            case 0:
              brush_drag();
              break;
            case 1:
              crop_drag();
              break;
            case 2:
              getcolor_push();
              break;
            case 3:
              offset_drag();
              break;
          }

          break;
        case 2:
          if(moving == 1)
            move();
          break;
      } 
      oldimgx = imgx;
      oldimgy = imgy;
      return 1;
    case FL_RELEASE:
      window()->make_current();
      fl_overlay_clear();

      switch(tool)
      {
        case 0:
          brush_release();
          break;
        case 1:
          crop_release();
          break;
        case 3:
          offset_release();
          break;
      }

      if(moving)
      {
        moving = 0;
        draw_main(1);
      }

      return 1;
    case FL_MOVE:
      switch(tool)
      {
        case 0:
          brush_move();
          break;
      }

      oldimgx = imgx;
      oldimgy = imgy;
      return 1;
    case FL_MOUSEWHEEL:
      if(tool_started)
        break;

      if(Fl::event_dy() >= 0)
      {
        zoom_out(mousex, mousey);
      }
      else
      {
        zoom_in(mousex, mousey);
      }
      return 1;
    case FL_KEYDOWN:
      if(tool_started)
      {
        if(Fl::event_key() == FL_Escape)
        {
          tool_started = 0;
          draw_main(1);
        }
        break;
      }

      switch(Fl::event_key())
      {
        case 32:
          Bitmap::clone_x = imgx;
          Bitmap::clone_y = imgy;
          Bitmap::clone_moved = 1;
          break;
        case FL_Right:
          scroll(0, 64);
          break;
        case FL_Left:
          scroll(1, 64);
          break;
        case FL_Down:
          scroll(2, 64);
          break;
        case FL_Up:
          scroll(3, 64);
          break;
      }
      return 1;
  }
  return 0;
}

void View::brush_push()
{
  if(stroke->active && stroke->type == 3)
  {
    if(dclick)
    {
      stroke->end(imgx, imgy, ox, oy, zoom);
      Blend::set(Brush::main->blend);
      stroke->render();
      while(stroke->render_callback(ox, oy, zoom))
      {
        draw_main(1);
        Fl::flush();
      }
      Blend::set(0);
      moving = 0;
      draw_main(1);
    }
    else
    {
      stroke->draw(imgx, imgy, ox, oy, zoom);
    }
  }
  else
  {
    stroke->begin(imgx, imgy, ox, oy, zoom);
  }

  stroke->preview(backbuf, ox, oy, zoom);
  redraw();
}

void View::brush_drag()
{
  if(stroke->type != 3)
  {
    stroke->draw(imgx, imgy, ox, oy, zoom);
    draw_main(0);
    stroke->preview(backbuf, ox, oy, zoom);
    redraw();
  }
}

void View::brush_release()
{
  if(stroke->active && stroke->type != 3)
  {
    stroke->end(imgx, imgy, ox, oy, zoom);
    Blend::set(Brush::main->blend);
    stroke->render();
    while(stroke->render_callback(ox, oy, zoom))
    {
      draw_main(1);
      Fl::flush();
    }
    Blend::set(0);
  }

  draw_main(1);
}

void View::brush_move()
{
  if(stroke->active && stroke->type == 3)
  {
    stroke->polyline(imgx, imgy, ox, oy, zoom);
    draw_main(0);
    stroke->preview(backbuf, ox, oy, zoom);
    redraw();
  }
}

void View::offset_push()
{
  int w = Bitmap::main->cw;
  int h = Bitmap::main->ch;
  int overscroll = Bitmap::main->overscroll;

  beginx = imgx;
  beginy = imgy;

  delete Bitmap::offset_buffer;
  Bitmap::offset_buffer = new Bitmap(w, h);
  Bitmap::main->blit(Bitmap::offset_buffer, overscroll, overscroll, 0, 0, w, h);
}

void View::offset_drag()
{
  int w = Bitmap::main->cw;
  int h = Bitmap::main->ch;
  int overscroll = Bitmap::main->overscroll;

  int dx = imgx - beginx;
  int dy = imgy - beginy;

  int x = dx;
  int y = dy;

  while(x < 0)
    x += w;
  while(y < 0)
    y += h;
  while(x >= w)
    x -= w;
  while(y >= h)
    y -= h;

  Bitmap::offset_buffer->blit(Bitmap::main, 0, 0, x + overscroll, y + overscroll, w - x, h - y);
  Bitmap::offset_buffer->blit(Bitmap::main, w - x, 0, overscroll, y + overscroll, x, h - y);
  Bitmap::offset_buffer->blit(Bitmap::main, 0, h - y, x + overscroll, overscroll, w - x, y);
  Bitmap::offset_buffer->blit(Bitmap::main, w - x, h - y, overscroll, overscroll, x, y);

  draw_main(1);
}

void View::offset_release()
{
}

void View::crop_push()
{
  if(tool_started == 0)
  {
    Map::main->clear(0);
    beginx = imgx;
    beginy = imgy;
    lastx = imgx;
    lasty = imgy;
    tool_started = 1;
  }
  else if(tool_started == 2)
  {
    if(dclick)
    {
      tool_started = 0;
      absrect(&beginx, &beginy, &lastx, &lasty);
      int w = lastx - beginx;
      int h = lasty - beginy;
      if(w < 1)
        w = 1;
      if(h < 1)
        h = 1;
      Bitmap *temp = new Bitmap(w, h);
      Bitmap::main->blit(temp, beginx, beginy, 0, 0, w, h);
      delete Bitmap::main;
      int overscroll = Bitmap::overscroll;
      int aw = w + overscroll * 2;
      int ah = h + overscroll * 2;
      Bitmap::main = new Bitmap(aw, ah);
      Bitmap::main->clear(makecol(0, 0, 0));
      Bitmap::main->set_clip(overscroll, overscroll, aw - overscroll - 1, ah - overscroll - 1);
      temp->blit(Bitmap::main, 0, 0, overscroll, overscroll, w, h);
      delete temp;
      zoom = 1;
      ox = 0;
      oy = 0;
      draw_main(1);
    }
  }
}

void View::crop_drag()
{
  if(tool_started == 1)
  {
    absrect(&beginx, &beginy, &lastx, &lasty);
    Map::main->rect(beginx, beginy, lastx, lasty, 0);
    absrect(&beginx, &beginy, &imgx, &imgy);
    Map::main->rect(beginx, beginy, imgx, imgy, 255);
    stroke->size(beginx, beginy, imgx, imgy);

    lastx = imgx;
    lasty = imgy;

    draw_main(1);
    stroke->preview(backbuf, ox, oy, zoom);
    redraw();
  }
  else if(tool_started == 2)
  {
    Map::main->rect(beginx, beginy, lastx, lasty, 0);

    if(crop_resize_started)
    {
      switch(crop_side)
      {
        case 0:
          beginx = imgx;
          break;
        case 1:
          lastx = imgx;
          break;
        case 2:
          beginy = imgy;
          break;
        case 3:
          lasty = imgy;
          break;
      }

      absrect(&beginx, &beginy, &lastx, &lasty);
      Map::main->rect(beginx, beginy, lastx, lasty, 255);
      stroke->size(beginx, beginy, lastx, lasty);
      draw_main(1);
      stroke->preview(backbuf, ox, oy, zoom);
      redraw();
    }
    else
    {
      if(imgx < beginx)
      {
        crop_side = 0;
        crop_resize_started = 1;
      }
      else if(imgx > lastx)
      {
        crop_side = 1;
        crop_resize_started = 1;
      }
      else if(imgy < beginy)
      {
        crop_side = 2;
        crop_resize_started = 1;
      }
      else if(imgy > lasty)
      {
        crop_side = 3;
        crop_resize_started = 1;
      }
    }
  }
}

void View::crop_release()
{
  if(tool_started == 1)
  {
    tool_started = 2;
  }

  crop_resize_started = 0;
}

void View::getcolor_push()
{
  if(inbox(imgx, imgy, Bitmap::main->cl, Bitmap::main->ct,
                       Bitmap::main->cr, Bitmap::main->cb))
  {
    int c = Bitmap::main->getpixel(imgx, imgy);
    update_color(c);
  }
}

void View::resize(int x, int y, int w, int h)
{
  Fl_Widget::resize(x, y, w, h);

  if(fit)
    zoom_fit(1);

  draw_main(0);
}

void View::draw_main(int refresh)
{
  int sw = w() / zoom;
  int sh = h() / zoom;
  sw += 2;
  sh += 2;

  if(sw > Bitmap::main->w - ox)
    sw = Bitmap::main->w - ox;
  if(sh > Bitmap::main->h - oy)
    sh = Bitmap::main->h - oy;

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

  Bitmap::main->point_stretch(backbuf, ox, oy, sw, sh, 0, 0, dw, dh, overx, overy);

  if(grid)
    draw_grid();

  if(refresh)
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

  d = 224 - zoom;
  if(d < 128)
    d = 128;

  int zx = zoom * gridx;
  int zy = zoom * gridy;
  int qx = (Bitmap::main->overscroll % gridx) * zoom;
  int qy = (Bitmap::main->overscroll % gridy) * zoom;

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
  aspect = (float)Bitmap::main->h / Bitmap::main->w;

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

  px = (ww - pw) >> 1;
  py = (hh - ph) >> 1;
  px += dx;
  py += dy;

  bw = ww * (((float)pw / zoom) / Bitmap::main->w);
  bh = bw * winaspect;

  bx = ox * ((float)pw / Bitmap::main->w) + px;
  by = oy * ((float)ph / Bitmap::main->h) + py;

  // pos.x = bx + bw / 2;
  // pos.y = by + bh / 2;
  // warp mouse here... (unsupported in fltk)

  backbuf->clear(makecol(0, 0, 0));
  Bitmap::main->fast_stretch(backbuf, 0, 0, Bitmap::main->w, Bitmap::main->h,
                          px, py, pw, ph);

  // need to force repaint here or the navigator won't have a border
  redraw();
  Fl::flush();
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

  ox = (bx - px) / ((float)pw / (Bitmap::main->w));
  oy = (by - py) / ((float)ph / (Bitmap::main->h));

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

  window()->make_current();
  fl_overlay_rect(x() + bx, y() + by, bw, bh);
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
    if(ox > Bitmap::main->w - w() / zoom)
      ox = Bitmap::main->w - w() / zoom;
    if(oy > Bitmap::main->h - h() / zoom)
      oy = Bitmap::main->h - h() / zoom;
    if(ox < 0)
      ox = 0;
    if(oy < 0)
      oy = 0;
  }

  draw_main(1);
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
    if(ox > Bitmap::main->w - w() / zoom)
      ox = Bitmap::main->w - w() / zoom;
    if(oy > Bitmap::main->h - h() / zoom)
      oy = Bitmap::main->h - h() / zoom;
    if(ox < 0)
      ox = 0;
    if(oy < 0)
      oy = 0;
  }

  draw_main(1);
}

void View::zoom_fit(int fitting)
{
  if(!fitting)
  {
    fit = 0;
    zoom = 1;
    ox = 0;
    oy = 0;
    draw_main(1);
    return;
  }

  winaspect = (float)h() / w();
  aspect = (float)Bitmap::main->h / Bitmap::main->w;

  if(aspect < winaspect)
    zoom = ((float)w() / Bitmap::main->w);
  else
    zoom = ((float)h() / Bitmap::main->h);

  ox = 0;
  oy = 0;

  fit = 1;
  draw_main(1);
}

void View::zoom_one()
{
  fit = 0;
  zoom = 1;
  ox = 0;
  oy = 0;
  draw_main(1);
}

void View::scroll(int dir, int amount)
{
  int x, y;

  switch(dir)
  {
    case 0:
      x = Bitmap::main->w - w() / zoom;
      if(x < 0)
        return;
      ox += amount / zoom;
      if(ox > x)
        ox = x;
      break;
    case 1:
      ox -= amount / zoom;
      if(ox < 0)
        ox = 0;
      break;
    case 2:
      y = Bitmap::main->h - h() / zoom;
      if(y < 0)
        return;
      oy += amount / zoom;
      if(oy > y)
        oy = y;
      break;
    case 3:
      oy -= amount / zoom;
      if(oy < 0)
        oy = 0;
      break;
  }

  draw_main(1);
}

void View::draw()
{
  if(stroke->active)
  {
    int blitx = stroke->blitx;
    int blity = stroke->blity;
    int blitw = stroke->blitw;
    int blith = stroke->blith;

    if(blitx < 0)
      blitx = 0;
    if(blity < 0)
      blity = 0;
    if(blitx + blitw > w() - 1)
      blitw = w() - 1 - blitx;
    if(blity + blith > h() - 1)
      blith = h() - 1 - blity;
    if(blitw < 1 || blith < 1)
      return;

    if(blitw < 256 && blith < 256)
    {
      fl_push_clip(x() + blitx, y() + blity, blitw, blith);
      image_part->draw(x() + blitx, y() + blity, blitw, blith, blitx, blity);
      fl_pop_clip();
      image_part->uncache();
    }
    else
    {
      int xx, yy;

      for(yy = blity; yy < blity + blith; yy++)
      { 
        int *p = backbuf->row[yy] + blitx;
        unsigned char *d = &alphadata[yy * (backbuf->w * 3) + (blitx * 3)];

        for(xx = blitx; xx < blitx + blitw; xx++)
        { 
          *d++ = getr(*p);
          *d++ = getg(*p);
          *d++ = getb(*p);
          p++;
        }
      }
      fl_push_clip(x() + blitx, y() + blity, blitw, blith);
      image_full->draw(x() + blitx, y() + blity, blitw, blith, blitx, blity);
      fl_pop_clip();
      image_full->uncache();
    }
  }
  else
  {
    int xx, yy;

    for(yy = 0; yy < h(); yy++)
    { 
      int *p = backbuf->row[yy];
      unsigned char *d = &alphadata[yy * (backbuf->w * 3)];

      for(xx = 0; xx < w(); xx++)
      { 
        *d++ = getr(*p);
        *d++ = getg(*p);
        *d++ = getb(*p);
        p++;
      }
    }

    fl_push_clip(x(), y(), w(), h());
    image_full->draw(x(), y(), w(), h(), 0, 0);
    fl_pop_clip();
    image_full->uncache();
  }
}

