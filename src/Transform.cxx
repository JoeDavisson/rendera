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
#include <cmath>

#include "Bitmap.H"
#include "CheckBox.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "Gamma.H"
#include "Gui.H"
#include "Inline.H"
#include "InputFloat.H"
#include "InputInt.H"
#include "Map.H"
#include "Project.H"
#include "Separator.H"
#include "Transform.H"
#include "Undo.H"
#include "View.H"

namespace
{
  Bitmap *bmp;
  int overscroll;

  void pushUndo()
  {
    bmp = Project::bmp;
    overscroll = bmp->overscroll;
    Undo::push();
  }
}

namespace Resize
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *width;
    InputInt *height;
    CheckBox *keep_aspect;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    Items::width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    Items::height->value(s);
    Items::dialog->show();
  }

  void checkWidth()
  {
    if(Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[8];
      int w = atoi(Items::width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      Items::height->value(s);
    }
  }

  void checkHeight()
  {
    if(Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[8];
      int h = atoi(Items::height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      Items::width->value(s);
    }
  }

  void checkKeepAspect()
  {
    if(Items::keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(Items::width->limitValue(1, 10000) < 0)
      return;

    if(Items::height->limitValue(1, 10000) < 0)
      return;

    int w = atoi(Items::width->value());
    int h = atoi(Items::height->value());

    Items::dialog->hide();
    pushUndo();

    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    bmp->blit(temp, overscroll, overscroll, overscroll, overscroll,
                    bmp->cw, bmp->ch);

    delete Project::bmp;
    Project::bmp = temp;

    delete Project::map;
    Project::map = new Map(Project::bmp->w, Project::bmp->h);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(true);
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Resize Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 72, 24, "Width:", 0);
    Items::width->center();
    Items::width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 72, 24, "Height:", 0);
    Items::height->center();
    Items::height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::keep_aspect = new CheckBox(Items::dialog, 0, y1, 16, 16, "Keep Aspect", 0);
    Items::keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Items::keep_aspect->value();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace Scale
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *width;
    InputInt *height;
    CheckBox *keep_aspect;
    CheckBox *wrap;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int dw, int dh, bool wrap_edges)
  {
    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    const int sx = overscroll;
    const int sy = overscroll;
    const int sw = bmp->cw;
    const int sh = bmp->ch;
    const int dx = overscroll;
    const int dy = overscroll;

    const float ax = ((float)sw / dw);
    const float ay = ((float)sh / dh);

    if(sw < 1 || sh < 1)
      return;

    if(dw < 1 || dh < 1)
      return;

    Bitmap *temp = new Bitmap(dw, dh, overscroll);
  
    // average colors if scaling down
    int mipx = 1, mipy = 1;
    if(sw > dw)
      mipx = (sw / dw);
    if(sh > dh)
      mipy = (sh / dh);

    const int div = mipx * mipy;

    if((mipx > 1) || (mipy > 1))
    {
      for(int y = 0; y <= sh - mipy; y += mipy)
      {
        for(int x = 0; x <= sw - mipx; x += mipx)
        {
          int r = 0, g = 0, b = 0, a = 0;

          for(int j = 0; j < mipy; j++)
          {
            for(int i = 0; i < mipx; i++)
            {
              const int c = bmp->getpixel(sx + x + i, sy + y + j);
              rgba_type rgba = getRgba(c);
              r += Gamma::fix(rgba.r);
              g += Gamma::fix(rgba.g);
              b += Gamma::fix(rgba.b);
              a += rgba.a;
            }
          }

          r /= div;
          g /= div;
          b /= div;
          a /= div;

          r = Gamma::unfix(r);
          g = Gamma::unfix(g);
          b = Gamma::unfix(b);

          const int c = makeRgba(r, g, b, a);

          for(int j = 0; j < mipy; j++)
          {
            for(int i = 0; i < mipx; i++)
            {
              *(bmp->row[sy + y + j] + sx + x + i) = c;
            }
          }
        }
      }
    }

    Gui::showProgress(dh);

    for(int y = 0; y < dh; y++) 
    {
      int *d = temp->row[dy + y] + dx;
      const float vv = (y * ay);
      const int v1 = vv;
      const float v = vv - v1;

      if(sy + v1 >= bmp->h - 1)
        break;

      int v2 = v1 + 1;

      if(v2 >= sh)
      {
        if(wrap_edges)
          v2 -= sh;
        else
          v2--;
      }

      int *c[4];
      c[0] = c[1] = bmp->row[sy + v1] + sx;
      c[2] = c[3] = bmp->row[sy + v2] + sx;

      for(int x = 0; x < dw; x++) 
      {
        const float uu = (x * ax);
        const int u1 = uu;
        const float u = uu - u1;

        if(sx + u1 >= bmp->w - 1)
          break;

        int u2 = u1 + 1;

        if(u2 >= sw)
        {
          if(wrap_edges)
            u2 -= sw;
          else
            u2--;
        }

        c[0] += u1;
        c[1] += u2;
        c[2] += u1;
        c[3] += u2;

        float f[4];

        f[0] = (1.0f - u) * (1.0f - v);
        f[1] = u * (1.0f - v);
        f[2] = (1.0f - u) * v;
        f[3] = u * v;

        float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

        for(int i = 0; i < 4; i++)
        {
          rgba_type rgba = getRgba(*c[i]);
          r += (float)Gamma::fix(rgba.r) * f[i];
          g += (float)Gamma::fix(rgba.g) * f[i];
          b += (float)Gamma::fix(rgba.b) * f[i];
          a += rgba.a * f[i];
        }

        r = Gamma::unfix((int)r);
        g = Gamma::unfix((int)g);
        b = Gamma::unfix((int)b);

        *d++ = makeRgba((int)r, (int)g, (int)b, (int)a);

        c[0] -= u1;
        c[1] -= u2;
        c[2] -= u1;
        c[3] -= u2;
      }

      if(Gui::updateProgress(y) < 0)
        break;
    }

    Gui::hideProgress();

    delete Project::bmp;
    Project::bmp = temp;

    delete Project::map;
    Project::map = new Map(Project::bmp->w, Project::bmp->h);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(true);
  }

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    Items::width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    Items::height->value(s);
    Items::dialog->show();
  }

  void checkWidth()
  {
    if(Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[8];
      int w = atoi(Items::width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      Items::height->value(s);
    }
  }

  void checkHeight()
  {
    if(Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[8];
      int h = atoi(Items::height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      Items::width->value(s);
    }
  }

  void checkKeepAspect()
  {
    if(Items::keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(Items::width->limitValue(1, 10000) < 0)
      return;

    if(Items::height->limitValue(1, 10000) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::width->value()),
          atoi(Items::height->value()),
          Items::wrap->value());
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Scale Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 72, 24, "Width:", 0);
    Items::width->center();
    Items::width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 72, 24, "Height:", 0);
    Items::height->center();
    Items::height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::keep_aspect = new CheckBox(Items::dialog, 0, y1, 16, 16, "Keep Aspect", 0);
    Items::keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Items::keep_aspect->center();
    Items::wrap = new CheckBox(Items::dialog, 0, y1, 16, 16, "Wrap Edges", 0);
    y1 += 16 + 8;
    Items::wrap->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace Rotate
{
  namespace Items
  {
    DialogWindow *dialog;
    InputFloat *angle;
    InputFloat *scale;
    CheckBox *tile;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(float angle, float scale, int overscroll, bool tile)
  {
    Bitmap *bmp = Project::bmp;

    // angle correction
    angle += 90;

    // rotation
    int du_col = (int)((std::sin(angle * (3.14159f / 180)) * scale) * 65536);
    int dv_col = (int)((std::sin((angle + 90) * (3.14159f / 180)) * scale) * 65536);
    int du_row = -dv_col;
    int dv_row = du_col;

    const int ww = (bmp->cr - bmp->cl) / 2;
    const int hh = (bmp->cb - bmp->ct) / 2;

    const int xx = bmp->cl;
    const int yy = bmp->ct;

    // origin
    const int ox = xx + ww;
    const int oy = yy + hh;
	
    // project new corners
    int x0 = xx - ox;
    int y0 = yy - oy;
    int x1 = xx + (ww * 2) - ox;
    int y1 = yy - oy;
    int x2 = xx - ox;
    int y2 = yy + (hh * 2) - oy;
    int x3 = xx + (ww * 2) - ox;
    int y3 = yy + (hh * 2) - oy;

    const int oldx0 = x0;
    const int oldy0 = y0;
    const int oldx1 = x1;
    const int oldy1 = y1;
    const int oldx2 = x2;
    const int oldy2 = y2;
    const int oldx3 = x3;
    const int oldy3 = y3;

    // rotate
    x0 = xx + ((oldx0 * du_col + oldy0 * du_row) >> 16);
    y0 = yy + ((oldx0 * dv_col + oldy0 * dv_row) >> 16);
    x1 = xx + ((oldx1 * du_col + oldy1 * du_row) >> 16);
    y1 = yy + ((oldx1 * dv_col + oldy1 * dv_row) >> 16);
    x2 = xx + ((oldx2 * du_col + oldy2 * du_row) >> 16);
    y2 = yy + ((oldx2 * dv_col + oldy2 * dv_row) >> 16);
    x3 = xx + ((oldx3 * du_col + oldy3 * du_row) >> 16);
    y3 = yy + ((oldx3 * dv_col + oldy3 * dv_row) >> 16);

    // find new bounding box
    const int bx1 = std::min(x0, std::min(x1, std::min(x2, x3))) - scale * 2;
    const int by1 = std::min(y0, std::min(y1, std::min(y2, y3))) - scale * 2;
    const int bx2 = std::max(x0, std::max(x1, std::max(x2, x3))) + scale * 2;
    const int by2 = std::max(y0, std::max(y1, std::max(y2, y3))) + scale * 2;
    int bw = (bx2 - bx1) + 1;
    int bh = (by2 - by1) + 1;

    // create image with new size
    Bitmap *temp = new Bitmap(bw, bh, overscroll);
    temp->rectfill(temp->cl, temp->ct, temp->cr, temp->cb,
                   makeRgba(0, 0, 0, 0), 0);

    bw /= 2;
    bh /= 2;

    // rotation
    du_col = (int)((std::sin(angle * (3.14159f / 180)) / scale) * 65536);
    dv_col = (int)((std::sin((angle + 90) * (3.14159f / 180)) / scale) * 65536);
    du_row = -dv_col;
    dv_row = du_col;

    int row_u = (bmp->w / 2) << 16;
    int row_v = (bmp->h / 2) << 16;

    row_u -= bw * du_col + bh * du_row;
    row_v -= bw * dv_col + bh * dv_row;

    Gui::showProgress(by2 - by1);

    // draw image
    for(int y = by1; y <= by2; y++)
    {
      int u = row_u;
      int v = row_v;

      row_u += du_row;
      row_v += dv_row;

      const int yy = ((temp->ch) / 2) + y;
      if(yy < temp->ct || yy > temp->cb)
        continue;

      for(int x = bx1; x <= bx2; x++)
      {
        int uu = u >> 16;
        int vv = v >> 16;

        u += du_col;
        v += dv_col;

        // clip source image
        if(tile)
        {
          while(uu < bmp->cl)
            uu += (bmp->cr - bmp->cl) + 1;
          while(vv < bmp->ct)
            vv += (bmp->cb - bmp->ct) + 1;
          while(uu > bmp->cr)
            uu -= (bmp->cr - bmp->cl) + 1;
          while(vv > bmp->cb)
            vv -= (bmp->cb - bmp->ct) + 1;
        }
        else
        {
          if(uu < bmp->cl || uu > bmp->cr || vv < bmp->ct || vv > bmp->cb)
            continue;
        }

        const int xx = ((temp->cw) / 2) + x;
        if(xx < temp->cl || xx > temp->cr)
          continue;

        int c = *(bmp->row[vv] + uu);
        *(temp->row[yy] + xx) = c;
      }

      Gui::updateProgress(y - by1);
    }

    Gui::hideProgress();

    delete Project::bmp;
    Project::bmp = temp;

    delete Project::map;
    Project::map = new Map(Project::bmp->w, Project::bmp->h);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(true);
}

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "0");
    Items::angle->value(s);
    Items::dialog->show();
  }

  void close()
  {
    if(Items::angle->limitValue(-359.99, 359.99) < 0)
      return;

    if(Items::scale->limitValue(.1, 10.0) < 0)
      return;

    Items::dialog->hide();
    pushUndo();

    apply(atof(Items::angle->value()), atof(Items::scale->value()),
          Project::overscroll, Items::tile->value());
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Rotate Image");
    Items::angle = new InputFloat(Items::dialog, 0, y1, 72, 24, "Angle:", 0);
    Items::angle->center();
    y1 += 24 + 8;
    Items::angle->value("0");
    Items::scale = new InputFloat(Items::dialog, 0, y1, 72, 24, "Scale:", 0);
    Items::scale->center();
    y1 += 24 + 8;
    Items::scale->value("1.0");
    Items::tile = new CheckBox(Items::dialog, 0, y1, 16, 16, "Tile", 0);
    y1 += 16 + 8;
    Items::tile->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

void Transform::init()
{
  Resize::init();
  Scale::init();
  Rotate::init();
}

void Transform::mirror()
{
  pushUndo();
  Project::bmp->mirror();
  Gui::getView()->drawMain(true);
}

void Transform::flip()
{
  pushUndo();
  Project::bmp->flip();
  Gui::getView()->drawMain(true);
}

void Transform::resize()
{
  Resize::begin();
}

void Transform::scale()
{
  Scale::begin();
}

void Transform::rotate()
{
  Rotate::begin();
}

