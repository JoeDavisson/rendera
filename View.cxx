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

#include "View.H"
#include "Bitmap.H"
#include "Blend.H"
#include "Palette.H"
#include "Tool.H"
#include "Gui.H"
#include "Widget.H"
#include "Stroke.H"
#include "File.H"

namespace
{
  #ifdef linux
    XImage *ximage;
  #else
    Fl_RGB_Image *image;
  #endif

  int oldx1 = 0;
  int oldy1 = 0;

  inline void gridSetpixel(const Bitmap *bmp, const int &x, const int &y,
                           const int &c, const int &t)
  {
    if(x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)
      return;

    int *p = bmp->row[y] + x;
    *p = blendFast(*p, c, t);
  }

  inline void gridHline(Bitmap *bmp, int x1, const int &y, int &x2,
                        const int &c, const int &t)
  {
    if(y < 0 || y >= bmp->h)
      return;

    if(x1 < 0)
      x1 = 0;
    if(x1 > bmp->w - 1)
      x1 = bmp->w - 1;
    if(x2 < 0)
      x2 = 0;
    if(x2 > bmp->w - 1)
      x2 = bmp->w - 1;

    int *p = bmp->row[y] + x1;
    int x;

    for(x = x1; x <= x2; x++)
    {
      *p = blendFast(*p, c, t);
      p++;
    }
  }

  void screenBlit(int sx, int sy, int dx, int dy, int w, int h)
  {
    #ifdef linux
      XPutImage(fl_display, fl_window, fl_gc, ximage, sx, sy, dx, dy, w, h);
    #else
      fl_push_clip(dx, dy, w, h);
      image->draw(dx, dy, w, h, sx, sy);
      fl_pop_clip()
    #endif
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
  moving = 0;
  grid = 0;
  gridx = 8;
  gridy = 8;
  oldimgx = 0;
  oldimgy = 0;

  tool = Tool::paint;

  backbuf = new Bitmap(Fl::w(), Fl::h());
  bgr_order = 0;

  #ifdef linux
    // try to detect pixelformat (almost always RGB or BGR)
    if(fl_visual->visual->blue_mask == 0xFF)
      bgr_order = 1;

    ximage = XCreateImage(fl_display, fl_visual->visual, 24, ZPixmap, 0,
                          (char *)backbuf->data, backbuf->w, backbuf->h, 32, 0);
  #else
    image = new Fl_RGB_Image((unsigned char *)backbuf->data,
                             Fl::w(), Fl::h(), 4, 0);
  #endif

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

  // do it this way to prevent multiple button presses
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
      switch(Gui::getTool())
      {
        case 1:
          window()->cursor(FL_CURSOR_CROSS);
          break;
        case 2:
          window()->cursor(FL_CURSOR_CROSS);
          break;
        case 3:
          window()->cursor(FL_CURSOR_HAND);
          break;
        default:
          window()->cursor(FL_CURSOR_DEFAULT);
          break;
      }
      return 1;
    case FL_LEAVE:
      window()->cursor(FL_CURSOR_DEFAULT);
      return 1;
    case FL_PUSH:
      take_focus();

      switch(button)
      {
        case 1:
          if(shift)
          {
            Bitmap::clone_x = imgx;
            Bitmap::clone_y = imgy;
            Bitmap::clone_moved = 1;
            redraw();
            break;
          }

          tool->push(this);
          break;
        case 2:
          if(moving == 0)
          {
            moving = 1;
            beginMove();
            break;
          }
      } 
      oldimgx = imgx;
      oldimgy = imgy;
      return 1;
    case FL_DRAG:
      take_focus();

      switch(button)
      {
        case 1:
          tool->drag(this);
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
      tool->release(this);

      if(moving)
      {
        moving = 0;
        if(tool->active)
          tool->redraw(this);
        else
          drawMain(1);
      }

      if(tool->started)
        tool->stroke->preview(backbuf, ox, oy, zoom);

      return 1;
    case FL_MOVE:
      tool->move(this);

      oldimgx = imgx;
      oldimgy = imgy;
      return 1;
    case FL_MOUSEWHEEL:
      if(moving)
        break;

      if(Fl::event_dy() >= 0)
      {
        zoomOut(mousex, mousey);
      }
      else
      {
        zoomIn(mousex, mousey);
      }

      if(tool->active)
        tool->redraw(this);
      return 1;
    case FL_KEYDOWN:
      if(Fl::event_key() == FL_Escape)
      {
        tool->active = 0;
        tool->started = 0;
        drawMain(1);
        break;
      }

      switch(Fl::event_key())
      {
        // interferes with Fl_Choice
        //case 32:
        //  Bitmap::clone_x = imgx;
        //  Bitmap::clone_y = imgy;
        //  Bitmap::clone_moved = 1;
        //  redraw();
        //  break;
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
    case FL_DND_ENTER:
      return 1;
    case FL_DND_LEAVE:
      return 1;
    case FL_DND_DRAG:
      return 1;
    case FL_DND_RELEASE:
      return 1;
    case FL_PASTE:
      if(strncasecmp(Fl::event_text(), "file://", 7) == 0)
      {
        char fn[256];

        // remove "file://" from path
        strcpy(fn, Fl::event_text() + 7);

        // convert to utf-8 (e.g. %20 becomes space)
        File::decodeURI(fn);

        // strip newline
        unsigned int i;
        for(i = 0; i < strlen(fn); i++)
        {
          if(fn[i] == '\r')
            fn[i] = '\0';
          if(fn[i] == '\n')
            fn[i] = '\0';
        }

        // try to load the file
        File::loadFile(fn);
      }
      return 1;
  }
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  Fl_Widget::resize(x, y, w, h);

  if(fit)
    zoomFit(1);

  drawMain(1);
}

void View::redraw()
{
  damage(FL_DAMAGE_ALL);
  Fl::flush();
}

void View::drawMain(int refresh)
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

  backbuf->clear(makeRgb(128, 128, 128));

  Bitmap::main->pointStretch(backbuf, ox, oy, sw, sh,
                             0, 0, dw, dh, overx, overy, bgr_order);

  if(grid)
    drawGrid();

  if(refresh)
    redraw();
}

void View::drawGrid()
{
  int x1, y1, x2, y2, d, i;
  int offx = 0, offy = 0;

  if(zoom < 1)
    return;

  if(zoom < 2 && (gridx == 1 || gridy == 1))
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
    gridHline(backbuf, x1, y1, x2, makeRgb(255, 255, 255), d);
    gridHline(backbuf, x1, y1 + zy - 1, x2, makeRgb(0, 0, 0), d);
    i = 0;
    do
    {
      x1 = 0 - zx + (offx * zoom) + qx - (int)(ox * zoom) % zx;
      do
      {
        gridSetpixel(backbuf, x1, y1, makeRgb(255, 255, 255), d);
        gridSetpixel(backbuf, x1 + zx - 1, y1, makeRgb(0, 0, 0), d);
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

void View::drawCloneCursor()
{
  if(moving)
    return;

  int x = Bitmap::clone_x;
  int y = Bitmap::clone_y;
  int dx = Bitmap::clone_dx;
  int dy = Bitmap::clone_dy;
  int mirror = Bitmap::clone_mirror;
  int w = Bitmap::main->w - 1;
  int h = Bitmap::main->h - 1;

  int x1 = imgx - dx;
  int y1 = imgy - dy;

  if(tool->active)
  {
    switch(mirror)
    {
      case 0:
        x1 = x1;
        y1 = y1;
        break;
      case 1:
        x1 = (w - x1) - (w - x * 2);
        y1 = y1;
        break;
      case 2:
        x1 = x1;
        y1 = (h - y1) - (h - y * 2);
        break;
      case 3:
        x1 = (w - x1) - (w - x * 2);
        y1 = (h - y1) - (h - y * 2);
        break;
    }

    x1 = (x1 - ox) * zoom;
    y1 = (y1 - oy) * zoom;
  }
  else
  {
    x1 = (x - ox) * zoom;
    y1 = (y - oy) * zoom;
  }

  backbuf->xorRectfill(x1 - 12, y1, x1 + 13, y1);
  backbuf->xorRectfill(x1, y1 - 12, x1, y1 + 13);

  screenBlit(oldx1 - 12, oldy1 - 12,
               this->x() + oldx1 - 12, this->y() + oldy1 - 12, 26, 26);
  screenBlit(x1 - 12, y1 - 12,
               this->x() + x1 - 12, this->y() + y1 - 12, 26, 26);

  oldx1 = x1;
  oldy1 = y1;
}

void View::beginMove()
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

  backbuf->clear(makeRgb(128, 128, 128));

  //Bitmap::main->pointStretch(backbuf,
   //                           0, 0, Bitmap::main->w, Bitmap::main->h,
  //                            px, py, pw, ph, 0, 0, bgr_order);
  Bitmap::main->fastStretch(backbuf,
                             0, 0, Bitmap::main->w, Bitmap::main->h,
                             px, py, pw, ph, bgr_order);

  lastbx = bx;
  lastby = by;
  lastbw = bw;
  lastbh = bh;

  backbuf->xorRect(bx, by, bx + bw - 1, by + bh - 1);

  int temp = tool->active;
  tool->active = 0;
  redraw();
  tool->active = temp;
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

  if(ox < 0)
    ox = 0;
  if(oy < 0)
    oy = 0;

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

  backbuf->xorRect(lastbx, lastby, lastbx + lastbw - 1, lastby + lastbh - 1);
  backbuf->xorRect(bx, by, bx + bw - 1, by + bh - 1);

  int temp = tool->active;
  tool->active = 0;
  redraw();
  tool->active = temp;

  lastbx = bx;
  lastby = by;
  lastbw = bw;
  lastbh = bh;
}

void View::zoomIn(int x, int y)
{
  if(fit)
    return;

  zoom *= 2;

  if(zoom > 64)
  {
    zoom = 64;
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

  drawMain(1);

  if(tool->started)
    tool->stroke->preview(backbuf, ox, oy, zoom);

  Gui::checkZoom();
}

void View::zoomOut(int x, int y)
{
  if(fit)
    return;

  float oldzoom = zoom;
  zoom /= 2;

  if(zoom < .125)
  {
    zoom = .125;
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

  drawMain(1);

  if(tool->started)
    tool->stroke->preview(backbuf, ox, oy, zoom);

  Gui::checkZoom();
}

void View::zoomFit(int fitting)
{
  if(!fitting)
  {
    fit = 0;
    zoom = 1;
    ox = 0;
    oy = 0;
    drawMain(1);
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
  drawMain(1);
  Gui::checkZoom();
}

void View::zoomOne()
{
  fit = 0;
  zoom = 1;
  ox = 0;
  oy = 0;
  drawMain(1);
  Gui::checkZoom();
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

  if(tool->active)
    tool->redraw(this);
  else
    drawMain(1);
}

void View::draw()
{
  if(tool->active)
  {
    int blitx = tool->stroke->blitx;
    int blity = tool->stroke->blity;
    int blitw = tool->stroke->blitw;
    int blith = tool->stroke->blith;

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

    screenBlit(blitx, blity, x() + blitx, y() + blity, blitw, blith);
    if(Gui::getClone())
      drawCloneCursor();
  }
  else
  {
    screenBlit(0, 0, x(), y(), w(), h());
    if(Gui::getClone())
      drawCloneCursor();
  }
}

