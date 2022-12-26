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

#include <FL/fl_draw.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Clone.H"
#include "File.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

#if defined WIN32
  #include <windows.h>
#endif

namespace
{
  #if defined linux
    XImage *ximage;
  #elif defined WIN32
    BITMAPINFO *bi;
    HDC buffer_dc;
    HBITMAP hbuffer;
    int *backbuf_data;
  #else
    Fl_RGB_Image *wimage;
  #endif

  int oldx1 = 0;
  int oldy1 = 0;

  inline void gridSetpixel(const Bitmap *bmp, const int x, const int y,
                           const int c, const int t)
  {
    if(x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)
      return;

    int *p = bmp->row[y] + x;
    *p = blendFast(*p, c, t);
  }

  inline void gridHline(Bitmap *bmp, int x1, const int y, int x2,
                        const int c, const int t)
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

    for(int x = x1; x <= x2; x++)
    {
      *p = blendFast(*p, c, t);
      p++;
    }
  }

  void updateView(int sx, int sy, int dx, int dy, int w, int h)
  {
    #if defined linux
      XPutImage(fl_display, fl_window, fl_gc, ximage, sx, sy, dx, dy, w, h);
    #elif defined WIN32
      BitBlt(fl_gc, dx, dy, w, h, buffer_dc, sx, sy, SRCCOPY);
    #else
      fl_push_clip(dx, dy, w, h);
      wimage->draw(dx, dy, w, h, sx, sy);
      wimage->uncache();
      fl_pop_clip();
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
  fit = false;
  moving = false;
  panning = false;
  last_ox = 0;
  last_oy = 0;
  grid = 0;
  gridsnap = 0;
  gridx = 8;
  gridy = 8;
  oldimgx = 0;
  oldimgy = 0;
  rendering = false;
  bgr_order = false;
  ignore_tool = false;

  #if defined linux
    backbuf = new Bitmap(Fl::w(), Fl::h());

    // try to detect pixelformat (almost always RGB or BGR)
    if(fl_visual->visual->blue_mask == 0xFF)
      bgr_order = true;

    ximage = XCreateImage(fl_display, fl_visual->visual, 24, ZPixmap, 0,
                          (char *)backbuf->data, backbuf->w, backbuf->h, 32, 0);
  #elif defined WIN32
    bgr_order = true;
    buffer_dc = CreateCompatibleDC(fl_gc);
    
    bi = new BITMAPINFO;

    ZeroMemory(&bi->bmiHeader, sizeof(BITMAPINFOHEADER));

    bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi->bmiHeader.biBitCount = 32;

    bi->bmiHeader.biPlanes = 1;
    bi->bmiHeader.biClrUsed = 0;

    bi->bmiHeader.biWidth = Fl::w();
    bi->bmiHeader.biHeight = -Fl::h();

    hbuffer = CreateDIBSection(buffer_dc, bi, DIB_RGB_COLORS,
                               (void **)&backbuf_data, 0, 0);

    backbuf = new Bitmap(Fl::w(), Fl::h(), backbuf_data);

    SelectObject(buffer_dc, hbuffer);
  #else
    backbuf = new Bitmap(Fl::w(), Fl::h());
    wimage = new Fl_RGB_Image((unsigned char *)backbuf->data,
                                Fl::w(), Fl::h(), 4, 0);
  #endif

  resize(group->x() + x, group->y() + y, w, h);
}

View::~View()
{
  delete backbuf;
}

int View::handle(int event)
{
  if(rendering)
    return 0;

  int overscroll = Project::overscroll;

  mousex = Fl::event_x() - x();
  mousey = Fl::event_y() - y();

  imgx = mousex / zoom + ox;
  imgy = mousey / zoom + oy;

  switch(Gui::getTool())
  {
    case Tool::PAINT:
      if(gridsnap)
      {
	if((Project::stroke->type != Stroke::FREEHAND)
          && (Project::stroke->type != Stroke::REGION))
	{
          imgx -= overscroll;
          if(imgx % gridx < gridx / 2)
            imgx -= imgx % gridx;
          else
            imgx += gridx - imgx % gridx - 1;
          imgx += overscroll;

          imgy -= overscroll;
          if(imgy % gridy < gridy / 2)
            imgy -= imgy % gridy;
          else
            imgy += gridy - imgy % gridy - 1;
          imgy += overscroll;
        }
      }
      break;
    case Tool::CROP:
    case Tool::SELECT:
      if(gridsnap)
      {
        imgx -= overscroll;
        if(imgx % gridx < gridx / 2)
          imgx -= imgx % gridx;
//        else
//          imgx += gridx - imgx % gridx - 1;
        imgx += overscroll;

        imgy -= overscroll;
        if(imgy % gridy < gridy / 2)
          imgy -= imgy % gridy;
//        else
//          imgy += gridy - imgy % gridy - 1;
        imgy += overscroll;
      }
      break;
    default:
      break;
  }

  // do it this way to prevent multiple button presses
  button1 = Fl::event_button1() ? 1 : 0;
  button2 = Fl::event_button2() ? 2 : 0;
  button3 = Fl::event_button3() ? 4 : 0;
  button = button1 | button2 | button3;
  dclick = Fl::event_clicks() ? true : false;
  shift = Fl::event_shift() ? true : false;
  ctrl = Fl::event_ctrl() ? true : false;
  alt = Fl::event_alt() ? true : false;

  switch(event)
  {
    case FL_FOCUS:
    {
      return 1;
    }

    case FL_UNFOCUS:
    {
      return 1;
    }

    case FL_ENTER:
    {
      changeCursor();
/*
      switch(Gui::getTool())
      {
        case Tool::GETCOLOR:
        case Tool::FILL:
          window()->cursor(FL_CURSOR_CROSS);
          break;
        case Tool::CROP:
        case Tool::SELECT:
          window()->cursor(FL_CURSOR_CROSS);
          Project::tool->redraw(this);
          break;
        case Tool::OFFSET:
          window()->cursor(FL_CURSOR_HAND);
          break;
        default:
          window()->cursor(FL_CURSOR_DEFAULT);
          break;
      }
*/

      return 1;
    }

    case FL_LEAVE:
    {
      window()->cursor(FL_CURSOR_DEFAULT);

      return 1;
    }

    case FL_PUSH:
    {
      // gives viewport focus when clicked on
      if(Fl::focus() != this)
        Fl::focus(this);

//FIXME buggy
/*
      Project::stroke->origin = Project::stroke->origin_always;
      Project::stroke->constrain = Project::stroke->constrain_always;
*/

      switch(button)
      {
        case 1:
/*
          if(ctrl)
            Project::stroke->origin = 1;

          if(shift)
            Project::stroke->constrain = 1;
*/
          if(alt)
          {
            // update clone target
            Clone::x = imgx;
            Clone::y = imgy;
            Clone::state = 1;
            Clone::moved = true;
            redraw();
          }
          else
          {
            Project::tool->push(this);
          }

          break;
        case 2:
          if(!moving)
          {
            if(shift)
            {
              // begin image navigation
              moving = true;
              beginMove();
            }
            else
            {
              // begin image panning
              last_ox = (w() - 1 - mousex) / zoom - ox;
              last_oy = (h() - 1 - mousey) / zoom - oy;
            }

           break;
          }
        case 4:
          Project::tool->push(this);
          break;
        default:
          break;
      } 

      oldimgx = imgx;
      oldimgy = imgy;

      return 1;
    }

    case FL_DRAG:
    {
      // gives viewport focus when clicked on
      if(Fl::focus() != this)
        Fl::focus(this);

      switch(button)
      {
        case 1:
          Project::tool->drag(this);
          break;
        case 2:
          if(moving)
          {
            // continue image navigation
            move();
          }
          else
          {
            // continue image panning
            panning = true;
            ox = (w() - 1 - mousex) / zoom - last_ox;
            oy = (h() - 1 - mousey) / zoom - last_oy; 

            clipOrigin();
            drawMain(false);
            Project::tool->redraw(this);
            redraw();
          }

          break;
      } 

      oldimgx = imgx;
      oldimgy = imgy;

      return 1;
    }

    case FL_RELEASE:
    {
      Project::tool->release(this);

      if(moving)
        moving = false;

      if(panning)
        panning = false;

      if(Project::tool->isActive())
        Project::tool->redraw(this);

      return 1;
    }

    case FL_MOVE:
    {
      Project::tool->move(this);
      redraw();

      // update coordinates display
      char coords[256];
      int coordx = imgx - Project::overscroll;
      int coordy = imgy - Project::overscroll;
      coordx = clamp(coordx, Project::bmp->cw - 1);
      coordy = clamp(coordy, Project::bmp->ch - 1);
      sprintf(coords, "(%d, %d)", coordx, coordy);
      Gui::updateCoords(coords);

      oldimgx = imgx;
      oldimgy = imgy;

      return 1;
    }

    case FL_MOUSEWHEEL:
    {
      // ignore wheel during image navigation
      if(moving || panning)
        break;

      if(Fl::event_dy() >= 0)
      {
        zoomOut(mousex, mousey);
      }
      else
      {
        zoomIn(mousex, mousey);
      }

      Project::tool->redraw(this);

      return 1;
    }

    case FL_DND_ENTER:
    {
      return 1;
    }

    case FL_DND_LEAVE:
    {
      return 1;
    }

    case FL_DND_DRAG:
    {
      return 1;
    }

    case FL_DND_RELEASE:
    {
      return 1;
    }

    case FL_PASTE:
    {
      // drag n drop
      if(strncasecmp(Fl::event_text(), "file://", 7) == 0)
      {
        const int length = Fl::event_length();
        char *fn = new char[length];

        // printf("length = %d\n", length);

        // remove "file:///" from path?
        #ifdef WIN32
          strcpy(fn, Fl::event_text());
        #else
          strcpy(fn, Fl::event_text() + 7);
        #endif

        // convert to utf-8 (e.g. %20 becomes space)
        File::decodeURI(fn);

        int index = 0;

        for(int i = 0, n = length; i < n; i += 0)
        {
          if(fn[i] == '\r' || fn[i] =='\n')
          {
            fn[i] = '\0';
            File::loadFile(fn + index);

            #ifdef WIN32
              i += 1;
            #else
              i += 8;
            #endif

            index = i;
          }
          else
          {
            i++;
          }
        }

        delete[] fn;
      }

      changeCursor();

      return 1;
    }
  }

  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  Fl_Widget::resize(x, y, w, h);

  if(fit)
    zoomFit(true);

  ignore_tool = true;
  ox = 0;
  oy = 0;
  drawMain(true);
}

// when a tool is active, set ignore_tool=true before
// calling if the entire view should be updated 
void View::redraw()
{
  damage(FL_DAMAGE_ALL);
  Fl::flush();
}

void View::drawMain(bool refresh)
{
  int sw = w() / zoom;
  int sh = h() / zoom;

  sw += 2;
  sh += 2;

  int dw = sw * zoom;
  int dh = sh * zoom;

  int overx = dw - w();
  int overy = dh - h();

  if(zoom < 2)
  {
    overx = 0;
    overy = 0;
  }

  backbuf->clear(getFltkColor(FL_BACKGROUND2_COLOR));

  int offx = 0;
  int offy = 0;

  if(ox < 0)
    offx = -ox;
  if(oy < 0)
    offy = -oy;

  Bitmap *bmp = Project::bmp;

  bmp->pointStretch(backbuf,
                    ox, oy, sw - offx, sh - offy,
                    offx * zoom, offy * zoom,
                    dw - offx * zoom, dh - offy * zoom,
                    overx, overy, bgr_order);

  if(grid)
    drawGrid();

  if(refresh)
    redraw();
}

void View::drawGrid()
{
  int x1, y1, x2, y2, t, i;
  int offx = 0, offy = 0;

  if(zoom < 1)
    return;

  if(zoom < 2 && (gridx == 1 || gridy == 1))
    return;

  x2 = w() - 1;
  y2 = h() - 1;

  t = 216 - zoom;
  if(t < 96)
    t = 96;

  int zx = zoom * gridx;
  int zy = zoom * gridy;
  int qx = (Project::bmp->overscroll % gridx) * zoom;
  int qy = (Project::bmp->overscroll % gridy) * zoom;

  y1 = 0 - zy + (offy * zoom) + qy - (int)(oy * zoom) % zy;

  do
  {
    x1 = 0 - zx + (offx * zoom) + qx - (int)(ox * zoom) % zx;
    gridHline(backbuf, x1, y1, x2, makeRgb(255, 255, 255), t);
    gridHline(backbuf, x1, y1 + zy - 1, x2, makeRgb(0, 0, 0), t);
    i = 0;

    do
    {
      x1 = 0 - zx + (offx * zoom) + qx - (int)(ox * zoom) % zx;

      do
      {
        gridSetpixel(backbuf, x1, y1, makeRgb(255, 255, 255), t);
        gridSetpixel(backbuf, x1 + zx - 1, y1, makeRgb(0, 0, 0), t);
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


void View::changeCursor()
{
  switch(Gui::getTool())
  {
    case Tool::GETCOLOR:
    case Tool::FILL:
      window()->cursor(FL_CURSOR_CROSS);
      break;
    case Tool::CROP:
    case Tool::SELECT:
      window()->cursor(FL_CURSOR_CROSS);
      Project::tool->redraw(this);
      break;
    case Tool::OFFSET:
      window()->cursor(FL_CURSOR_HAND);
      break;
    default:
      window()->cursor(FL_CURSOR_DEFAULT);
      break;
  }
}

void View::drawCloneCursor()
{
  if(moving)
    return;

  int x = Clone::x;
  int y = Clone::y;
  int dx = Clone::dx;
  int dy = Clone::dy;
  int state = Clone::state;
  int mirror = Clone::mirror;
  int w = Project::bmp->w - 1;
  int h = Project::bmp->h - 1;

  int x1 = imgx - dx;
  int y1 = imgy - dy;

  if(Project::tool->isActive())
  {
    switch(mirror)
    {
      case 0:
        break;
      case 1:
        x1 = (w - x1) - (w - x * 2);
        break;
      case 2:
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
    if(state == 0)
    {
      x1 = (x1 - ox) * zoom;
      y1 = (y1 - oy) * zoom;
    }
    else if(state == 1)
    {
      x1 = (x - ox) * zoom;
      y1 = (y - oy) * zoom;
    }
  }

  backbuf->rect(x1 - 9, y1 - 1, x1 + 9, y1 + 1, makeRgb(0, 0, 0), 64);
  backbuf->rect(x1 - 10, y1 - 2, x1 + 10, y1 + 2, makeRgb(0, 0, 0), 160);
  backbuf->rect(x1 - 1, y1 - 9, x1 + 1, y1 + 9, makeRgb(0, 0, 0), 64);
  backbuf->rect(x1 - 2, y1 - 10, x1 + 2, y1 + 10, makeRgb(0, 0, 0), 160);
  backbuf->xorRectfill(x1 - 8, y1, x1 + 8, y1);
  backbuf->xorRectfill(x1, y1 - 8, x1, y1 + 8);

  updateView(oldx1 - 12, oldy1 - 12,
             this->x() + oldx1 - 12, this->y() + oldy1 - 12, 26, 26);
  updateView(x1 - 12, y1 - 12,
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
  aspect = (float)Project::bmp->h / Project::bmp->w;

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

  bw = ww * (((float)pw / zoom) / Project::bmp->w);
  bh = bw * winaspect;

  bx = ox * ((float)pw / Project::bmp->w) + px;
  by = oy * ((float)ph / Project::bmp->h) + py;

  // pos.x = bx + bw / 2;
  // pos.y = by + bh / 2;
  // warp mouse here... (unsupported in fltk)

  backbuf->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));

  Project::bmp->fastStretch(backbuf,
                             0, 0, Project::bmp->w, Project::bmp->h,
                             px, py, pw, ph, bgr_order);

  lastbx = bx;
  lastby = by;
  lastbw = bw;
  lastbh = bh;

  backbuf->xorRect(bx, by, bx + bw - 1, by + bh - 1);

  ignore_tool = true;
  redraw();
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

  ox = (bx - px) / ((float)pw / (Project::bmp->w));
  oy = (by - py) / ((float)ph / (Project::bmp->h));

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

  // draw bounding box
  backbuf->xorRect(lastbx, lastby, lastbx + lastbw - 1, lastby + lastbh - 1);
  backbuf->xorRect(bx, by, bx + bw - 1, by + bh - 1);

  ignore_tool = true;
  redraw();

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

    clipOrigin();
  }

    drawMain(false);
    Project::tool->redraw(this);
    redraw();

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

    clipOrigin();
  }

  drawMain(true);

    drawMain(false);
    Project::tool->redraw(this);
    redraw();

  Gui::checkZoom();
}

void View::zoomFit(bool fit_to_view)
{
  if(!fit_to_view)
  {
    fit = false;
    zoom = 1;
    ox = 0;
    oy = 0;
    drawMain(true);
    Gui::checkZoom();
    return;
  }

  winaspect = (float)h() / w();
  aspect = (float)Project::bmp->h / Project::bmp->w;

  if(aspect < winaspect)
    zoom = ((float)w() / Project::bmp->w);
  else
    zoom = ((float)h() / Project::bmp->h);

  ox = 0;
  oy = 0;

  fit = true;
  drawMain(true);
  Gui::checkZoom();
}

void View::zoomOne()
{
  fit = false;
  zoom = 1;
  ox = 0;
  oy = 0;
  drawMain(true);
  Gui::checkZoom();
}

void View::scroll(int dir, int amount)
{
  int x, y;

  switch(dir)
  {
    case 0:
    {
      x = Project::bmp->w - w() / zoom;

      if(x < 0)
        return;

      ox += amount / zoom;

      if(ox > x)
        ox = x;

      break;
    }
    case 1:
    {
      ox -= amount / zoom;

      if(ox < 0)
        ox = 0;

      break;
    }
    case 2:
    {
      y = Project::bmp->h - h() / zoom;

      if(y < 0)
        return;

      oy += amount / zoom;

      if(oy > y)
        oy = y;

      break;
    }
    case 3:
    {
      oy -= amount / zoom;

      if(oy < 0)
        oy = 0;

      break;
    }
  }

  if(Project::tool->isActive())
    Project::tool->redraw(this);
  else
    drawMain(true);
}

void View::clipOrigin()
{
  if(ox < (Project::bmp->cl + 1) - w() / zoom)
    ox = (Project::bmp->cl + 1) - w() / zoom;
  if(oy < (Project::bmp->ct + 1) - h() / zoom)
    oy = (Project::bmp->ct + 1) - h() / zoom;
  if(ox > Project::bmp->cr)
    ox = Project::bmp->cr;
  if(oy > Project::bmp->cb)
    oy = Project::bmp->cb;
}

// do not call directly, call redraw() instead
void View::draw()
{
  if(Project::tool->isActive())
  {
    if(ignore_tool)
    {
      ignore_tool = false;
      updateView(0, 0, x(), y(), w(), h());
      if(Gui::getClone())
        drawCloneCursor();
      return;
    }
    else
    {
      int blitx = Project::stroke->blitx;
      int blity = Project::stroke->blity;
      int blitw = Project::stroke->blitw;
      int blith = Project::stroke->blith;

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

      updateView(blitx, blity, x() + blitx, y() + blity, blitw, blith);

      if(Gui::getClone())
        drawCloneCursor();
    }
  }
  else
  {
    updateView(0, 0, x(), y(), w(), h());
    if(Gui::getClone())
      drawCloneCursor();
  }
}

