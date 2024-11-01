/*
Copyright (c) 2024 Joe Davisson.

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

#include "FX/GaussianBlur.H"

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
  #else
    Fl_RGB_Image *wimage;
  #endif

  int oldx1 = 0;
  int oldy1 = 0;

  inline void gridSetpixel(const Bitmap *bmp, const int x, const int y,
                           const int c, const int t)
  {
    if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)
      return;

    int *p = bmp->row[y] + x;
    *p = blendFast(*p, c, t);
  }

  inline void gridHline(Bitmap *bmp, int x1, const int y, int x2,
                        const int c, const int t)
  {
    if (y < 0 || y >= bmp->h)
      return;

    if (x1 < 0)
      x1 = 0;
    if (x1 > bmp->w - 1)
      x1 = bmp->w - 1;
    if (x2 < 0)
      x2 = 0;
    if (x2 > bmp->w - 1)
      x2 = bmp->w - 1;

    int *p = bmp->row[y] + x1;

    for (int x = x1; x <= x2; x++)
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
  aspect = ASPECT_NORMAL;
  panning = false;
  resizing = false;
  last_ox = 0;
  last_oy = 0;
  last_mousex = 0;
  last_mousey = 0;
  grid = false;
  gridsnap = false;
  gridx = 8;
  gridy = 8;
  oldimgx = 0;
  oldimgy = 0;
  rendering = false;
  bgr_order = false;
  filter = false;

  //FIXME this should handle desktop resolution changes
  #if defined linux
    backbuf = new Bitmap(Fl::w(), Fl::h());

    // try to detect pixelformat (almost always RGB or BGR)
    if (fl_visual->visual->blue_mask == 0xff)
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

    backbuf = new Bitmap(Fl::w(), Fl::h());

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
  if (rendering)
    return 0;

  mousex = Fl::event_x() - x();
  mousey = Fl::event_y() - y();

  int ax = 1;
  int ay = 1;

  switch (aspect)
  {
    case ASPECT_NORMAL:
      break;
    case ASPECT_WIDE:
      ax = 2;
      break;
    case ASPECT_TALL:
      ay = 2;
      break;
  }

  imgx = mousex / zoom + ox * ax;
  imgy = mousey / zoom + oy * ay;
  imgx /= ax;
  imgy /= ay;

  switch (Gui::getTool())
  {
    case Tool::PAINT:
      if (gridsnap)
      {
        if ((Project::stroke->type != Stroke::FREEHAND)
          && (Project::stroke->type != Stroke::REGION))
        {
          if (imgx % gridx < gridx / 2)
            imgx -= imgx % gridx;
          else
            imgx += gridx - imgx % gridx - 1;

          if (imgy % gridy < gridy / 2)
            imgy -= imgy % gridy;
          else
            imgy += gridy - imgy % gridy - 1;
        }
      }
      break;
    case Tool::SELECT:
      if (gridsnap)
      {
        if (imgx % gridx < gridx / 2)
          imgx -= imgx % gridx;
//        else
//          imgx += gridx - imgx % gridx - 1;

        if (imgy % gridy < gridy / 2)
          imgy -= imgy % gridy;
//        else
//          imgy += gridy - imgy % gridy - 1;
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

  switch (event)
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
      window()->cursor(FL_CURSOR_CROSS);
      changeCursor();
      return 1;
    }

    case FL_LEAVE:
    {
      window()->cursor(FL_CURSOR_DEFAULT);
      return 1;
    }

    // key presses are handled in Gui.cxx
    //  case FL_KEYDOWN:
    //  {
    //    break;
    //  }

    case FL_PUSH:
    {
      // gives viewport focus when clicked on
      if (Fl::focus() != this)
        Fl::focus(this);

      switch (button)
      {
        case 1:
          if (ctrl)
          {
            // update clone target
            Clone::x = imgx;
            Clone::y = imgy;
            Clone::dx = 0;
            Clone::dy = 0;
            Clone::moved = true;
            Clone::state = Clone::PLACED;
            redraw();
          }
            else
          {
            Project::tool->push(this);
          }

          break;
        case 2:
          // begin image panning
          last_ox = (w() - 1 - (mousex / ax)) / zoom - ox;
          last_oy = (h() - 1 - (mousey / ay)) / zoom - oy;
         break;
        case 4:
          // brush resize
          last_mousex = mousex;
          last_mousey = mousey;
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
      if (Fl::focus() != this)
        Fl::focus(this);

      switch (button)
      {
        case 1:
          Project::tool->drag(this);
          break;
        case 2:
          // continue image panning
          panning = true;
          ox = (w() - 1 - (mousex / ax)) / zoom - last_ox;
          oy = (h() - 1 - (mousey / ay)) / zoom - last_oy; 

          clipOrigin();
          Project::tool->redraw(this);

          saveCoords();
          break;
        case 4:
          resizing = true;
          const int mx = (mousex / ax) / zoom - Project::brush->size / 2;
          const int my = (mousey / ay) / zoom - Project::brush->size / 2;
          const int lx = (last_mousex / ax) / zoom - Project::brush->size / 2;
          const int ly = (last_mousey / ay) / zoom - Project::brush->size / 2;
          const int dx = std::abs(mx - lx);
          const int dy = std::abs(my - ly);
          int size = std::sqrt(dx * dx + dy * dy);

          if (size < 1)
            size = 1;

          if (size > 512)
            size = 512;

          Gui::paintChangeSize(size);
          Project::map->clear(0);
          Project::stroke->drawBrush(ox + (last_mousex / ax) / zoom,
                                     oy + (last_mousey / ay) / zoom,
                                     255);
          Project::stroke->makeBlitRect(imgx - 40, imgy - 40,
                                        imgx + 40, imgy + 40,
                                        ox, oy, size, zoom);
          drawMain(false);
          Project::stroke->previewPaint(this);
          redraw();
          break;
      } 

      oldimgx = imgx;
      oldimgy = imgy;

      return 1;
    }

    case FL_RELEASE:
    {
      Project::tool->release(this);

      panning = false;
      resizing = false;

      if (Project::tool->isActive())
        Project::tool->redraw(this);

      return 1;
    }

    case FL_MOVE:
    {
      Project::tool->move(this);

      // update coordinates display
      char coords[256];
      int coordx = imgx;
      int coordy = imgy;
      coordx = clamp(coordx, Project::bmp->cw - 1);
      coordy = clamp(coordy, Project::bmp->ch - 1);
      snprintf(coords, sizeof(coords), "(%d, %d)", coordx, coordy);
      Gui::statusCoords(coords);

      oldimgx = imgx;
      oldimgy = imgy;

      return 1;
    }

    case FL_MOUSEWHEEL:
    {
      // ignore wheel during image navigation or resizing brush
      if (panning || resizing)
        break;

      if (Fl::event_dy() >= 0)
      {
        zoomOut(mousex / ax, mousey / ay);
      }
        else
      {
        zoomIn(mousex / ax, mousey / ay);
      }

/*
      Gui::zoomLevel();
      Project::tool->redraw(this);
      saveCoords();
*/

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
      changeCursor();

      #ifndef WIN32
        if (strncasecmp(Fl::event_text(), "file://", 7) != 0)
          return 1;
      #endif

      const int length = Fl::event_length();
      char fn[length];

      memset(fn, 0, sizeof(char) * length);

      #ifdef WIN32
        strcpy(fn, Fl::event_text());
      #else
        strcpy(fn, Fl::event_text() + 7);
      #endif

      // convert to utf-8 (e.g. %20 becomes space)
      File::decodeURI(fn);

      int index = 0;

      // separate individual file paths in the list
      for (int i = 0; i < length; i++)
        if (fn[i] == '\n')
          fn[i] = '\0';

      // try to load all the files in list
      for (int i = 0; i < length; )
      {
        if (i == length - 1 || fn[i] == '\0')
        {
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
          i += 1;
        }
      }

      return 1;
    }

    return 1;
  }

  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  drawMain(false);
  Fl_Widget::resize(x, y, w, h);
}

// call if the entire view should be updated 
void View::redraw()
{
  damage(FL_DAMAGE_ALL);
  Fl::flush();
}

void View::changeAspect(int new_aspect)
{
  aspect = new_aspect;
  ox = 0;
  oy = 0;
  drawMain(true);
}

void View::drawMain(bool refresh)
{
  int sw = w() / zoom;
  int sh = h() / zoom;

  int dw = sw * zoom;
  int dh = sh * zoom;

  backbuf->clear(getFltkColor(FL_BACKGROUND2_COLOR));

  int offx = 0;
  int offy = 0;

  if (ox < 0)
    offx = -ox;

  if (oy < 0)
    offy = -oy;

  Bitmap *bmp = Project::bmp;

  if (zoom >= 1.0 || filter == false || panning == true ||
      rendering == true || Project::tool->isActive())
  {
    bmp->pointStretch(backbuf,
                      ox, oy,
                      sw - offx, sh - offy,
                      offx * zoom, offy * zoom,
                      dw - offx * zoom, dh - offy * zoom,
                      bgr_order);
  }
    else
  {
    bmp->filteredStretch(backbuf,
                         ox, oy,
                         sw - offx, sh - offy,
                         offx * zoom, offy * zoom,
                         dw - offx * zoom, dh - offy * zoom,
                         bgr_order);
  }

  if (grid)
    drawGrid();

  if (refresh)
    redraw();
}

void View::drawGrid()
{
  int x1, y1, x2, y2, t, i;
  int offx = 0, offy = 0;

  if (zoom < 1)
    return;

  if (zoom < 2 && (gridx == 1 || gridy == 1))
    return;

  x2 = w() - 1;
  y2 = h() - 1;

  t = 216 - zoom;

  if (t < 96)
    t = 96;

  int zx = zoom * gridx;
  int zy = zoom * gridy;
  int qx = 0;
  int qy = 0;

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
      while (x1 <= x2);

      y1++;
      i++;
    }
    while (i < zy);
  }
  while (y1 <= y2);
}


void View::changeCursor()
{
  switch (Gui::getTool())
  {
    case Tool::GETCOLOR:
    case Tool::FILL:
      window()->cursor(FL_CURSOR_CROSS);
      break;
    case Tool::SELECT:
    case Tool::TEXT:
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
  if (Gui::getTool() != Tool::PAINT && Gui::getTool() != Tool::TEXT)
    return;

  int x = Clone::x;
  int y = Clone::y;
  int dx = Clone::dx;
  int dy = Clone::dy;
  int state = Clone::state;

  int x1 = imgx - dx;
  int y1 = imgy - dy;

  if (state == Clone::RESET || state == Clone::PLACED)
  {
    x1 = (x - ox) * zoom;
    y1 = (y - oy) * zoom;
  }
    else
  {
    x1 = (x1 - ox) * zoom;
    y1 = (y1 - oy) * zoom;
  }

  switch (aspect)
  {
    case ASPECT_NORMAL:
      break;
    case ASPECT_WIDE:
      x1 *= 2;
      break;
    case ASPECT_TALL:
      y1 *= 2;
      break;
  }

  backbuf->rect(x1 - 8, y1 - 1, x1 + 8, y1 + 1, makeRgb(0, 0, 0), 0);
  backbuf->rect(x1 - 1, y1 - 8, x1 + 1, y1 + 8, makeRgb(0, 0, 0), 0);
  backbuf->xorRectfill(x1 - 7, y1, x1 + 7, y1);
  backbuf->xorRectfill(x1, y1 - 7, x1, y1 + 7);
  backbuf->rectfill(x1 - 7, y1, x1 + 7, y1,
                    convertFormat(makeRgb(255, 0, 192), bgr_order), 128);
  backbuf->rectfill(x1, y1 - 7, x1, y1 + 7,
                    convertFormat(makeRgb(255, 0, 192), bgr_order), 128);

  updateView(oldx1 - 12, oldy1 - 12,
             this->x() + oldx1 - 12, this->y() + oldy1 - 12, 26, 26);
  updateView(x1 - 12, y1 - 12,
             this->x() + x1 - 12, this->y() + y1 - 12, 26, 26);

  oldx1 = x1;
  oldy1 = y1;
}

void View::zoomIn(int x, int y)
{
  zoom *= 2;

  if (zoom > 64)
  {
    zoom = 64;
  }
    else
  {
    ox += x / zoom;
    oy += y / zoom;

    clipOrigin();
  }

  Gui::zoomLevel();
  Project::tool->redraw(this);
  saveCoords();
}

void View::zoomOut(int x, int y)
{
  float oldzoom = zoom;
  zoom /= 2;

  if (zoom < .0625)
  {
    zoom = .0625;
  }
    else
  {
    ox -= x / oldzoom;
    oy -= y / oldzoom;

    clipOrigin();
  }

  Gui::zoomLevel();
  Project::tool->redraw(this);
  saveCoords();
}

void View::zoomOne()
{
  zoom = 1;
  ox = 0;
  oy = 0;

  Gui::zoomLevel();
  Project::tool->redraw(this);
  saveCoords();
}

void View::scroll(int dir, int amount)
{
  int x, y;

  switch (dir)
  {
    case 0:
    {
      x = Project::bmp->w - w() / zoom;

      if (x < 0)
        return;

      ox += amount / zoom;

      if (ox > x)
        ox = x;

      break;
    }
    case 1:
    {
      ox -= amount / zoom;

      if (ox < 0)
        ox = 0;

      break;
    }
    case 2:
    {
      y = Project::bmp->h - h() / zoom;

      if (y < 0)
        return;

      oy += amount / zoom;

      if (oy > y)
        oy = y;

      break;
    }
    case 3:
    {
      oy -= amount / zoom;

      if (oy < 0)
        oy = 0;

      break;
    }
  }

  saveCoords();

  if (Project::tool->isActive())
    Project::tool->redraw(this);
  else
    drawMain(true);
}

void View::clipOrigin()
{
  if (ox < (Project::bmp->cl + 1) - w() / zoom)
    ox = (Project::bmp->cl + 1) - w() / zoom;
  if (oy < (Project::bmp->ct + 1) - h() / zoom)
    oy = (Project::bmp->ct + 1) - h() / zoom;
  if (ox > Project::bmp->cr)
    ox = Project::bmp->cr;
  if (oy > Project::bmp->cb)
    oy = Project::bmp->cb;
}

void View::saveCoords()
{
  // save coords/zoom for current image
  Project::ox_list[Project::current] = ox;
  Project::oy_list[Project::current] = oy;
  Project::zoom_list[Project::current] = zoom;
}

// do not call directly, call redraw() instead
void View::draw()
{
  int ax = 1;
  int ay = 1;

  switch (aspect)
  {
    case ASPECT_NORMAL:
      break;
    case ASPECT_WIDE:
      ax = 2;
      backbuf->doubleHorizontal();
      break;
    case ASPECT_TALL:
      ay = 2;
      backbuf->doubleVertical();
      break;
  }

  if (Project::tool->isActive())
  {
    int blitx = Project::stroke->blitx;
    int blity = Project::stroke->blity;
    int blitw = Project::stroke->blitw;
    int blith = Project::stroke->blith;

    if (blitx < 0)
      blitx = 0;

    if (blity < 0)
      blity = 0;

    if (blitx + blitw > w() - 1)
      blitw = w() - 1 - blitx;

    if (blity + blith > h() - 1)
      blith = h() - 1 - blity;

    if (blitw < 1 || blith < 1)
      return;

    updateView(blitx * ax, blity * ay, x() + blitx * ax, y() + blity * ay,
               blitw * ax, blith * ay);

    if (Gui::getClone())
      drawCloneCursor();
  }
    else
  {
    updateView(0, 0, x(), y(), w(), h());

    if (Gui::getClone())
      drawCloneCursor();
  }
}

