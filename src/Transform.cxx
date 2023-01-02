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
#include <cmath>

#include <FL/Fl_Choice.H>

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

#include "FX/GaussianBlur.H"

namespace
{
  Bitmap *bmp;
  int overscroll;

  void pushUndo()
  {
    bmp = Project::bmp;
    overscroll = bmp->overscroll;
    Project::undo->push();
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

  void close()
  {
    int w = atoi(Items::width->value());
    int h = atoi(Items::height->value());

    if(Project::enoughMemory(w, h) == false)
      return;

    Items::dialog->hide();
    pushUndo();

    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    temp->rectfill(temp->cl, temp->ct, temp->cr, temp->cb, makeRgba(0, 0, 0, 0), 0);
    bmp->blit(temp, overscroll, overscroll, overscroll, overscroll,
                    bmp->cw, bmp->ch);

    Project::replaceImageFromBitmap(temp);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
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
    Items::width = new InputInt(Items::dialog, 0, y1, 96, 24, "Width", (Fl_Callback *)checkWidth, 1, 32768);
    Items::width->center();
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 96, 24, "Height", (Fl_Callback *)checkHeight, 1, 32768);
    Items::height->center();
    y1 += 24 + 8;
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::keep_aspect = new CheckBox(Items::dialog, 0, y1, 16, 16, "Keep Aspect", 0);
    Items::keep_aspect->center();
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
    InputInt *percent;
    CheckBox *keep_aspect;
    Fl_Choice *mode;
    CheckBox *wrap;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void blur(Bitmap *bmp, int size)
  {
    GaussianBlur::apply(bmp, size / 2, 0, 0);
  }

  float cubic(const float p[4], const float x)
  {
    return p[1] + 0.5f * x * (p[2] - p[0] + x * (2.0f * p[0] - 5.0f * p[1]
           + 4.0f * p[2] - p[3] + x * (3.0f * (p[1] - p[2]) + p[3] - p[0])));
  }

  float bicubic(const float p[4][4], const float x, const float y)
  {
    float temp[4];

    temp[0] = cubic(p[0], y);
    temp[1] = cubic(p[1], y);
    temp[2] = cubic(p[2], y);
    temp[3] = cubic(p[3], y);

    return cubic(temp, x);
  }

  void apply(const int dw, const int dh, const bool wrap_edges)
  {
    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    const int sx = overscroll;
    const int sy = overscroll;
    const int sw = bmp->cw;
    const int sh = bmp->ch;
    const int dx = overscroll;
    const int dy = overscroll;

    if(sw < 1 || sh < 1)
      return;

    if(dw < 1 || dh < 1)
      return;

    Bitmap *temp = new Bitmap(dw, dh, overscroll);

    const float ax = ((float)sw / dw);
    const float ay = ((float)sh / dh);

    if(Items::mode->value() == 0)
    {
      // nearest
      for(int y = 0; y < dh; y++) 
      {
        int *d = temp->row[dy + y] + dx;
        const int yy = y * ay;

        for(int x = 0; x < dw; x++) 
        {
          const int xx = x * ax;

          *d++ = *(bmp->row[yy + sy] + xx + sx);
        }
      }
    }
    else if(Items::mode->value() == 1)
    {
      // bilinear
      int mipx = 1, mipy = 1;

      if(sw > dw)
        mipx = (sw / dw);
      if(sh > dh)
        mipy = (sh / dh);

      if(mipx > 1 || mipy > 1)
      {
        int size = mipx > mipy ? mipx : mipy;
        blur(bmp, size);
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
    }
    else if(Items::mode->value() == 2)
    {
      // bicubic
      int mipx = 1, mipy = 1;

      if(sw > dw)
        mipx = (float)sw / dw;
      if(sh > dh)
        mipy = (float)sh / dh;

      if(mipx > 1 || mipy > 1)
      {
        int size = mipx < mipy ? mipx : mipy;
        blur(bmp, size);
      }

      float r[4][4];
      float g[4][4];
      float b[4][4];
      float a[4][4];

      Gui::showProgress(dh);

      for(int y = 0; y < dh; y++) 
      {
        int *d = temp->row[dy + y] + dx;

        const float vv = (y * ay);
        const int v1 = vv;
        const float v = vv - v1;

        for(int x = 0; x < dw; x++) 
        {
          const float uu = (x * ax);
          const int u1 = uu;
          const float u = uu - u1;

          for(int j = 0; j < 4; j++)
          {
            int yy = v1 + j - 1;

            if(wrap_edges)
            {
              if(yy >= sh)
                yy -= sh;
            }

            if(yy > sh - 1)
              yy = sh - 1;

            for(int i = 0; i < 4; i++)
            {
              int xx = u1 + i - 1;

              if(wrap_edges)
              {
                if(xx >= sw)
                  xx -= sw;
              }

              if(xx > sw - 1)
                xx = sw - 1;

              const rgba_type rgba = getRgba(*(bmp->row[sy + yy] + sx + xx));

              r[i][j] = Gamma::fix(rgba.r);
              g[i][j] = Gamma::fix(rgba.g);
              b[i][j] = Gamma::fix(rgba.b);
              a[i][j] = Gamma::fix(rgba.a);
            }
          }

          const int rr = Gamma::unfix(clamp(bicubic(r, u, v), 65535));
          const int gg = Gamma::unfix(clamp(bicubic(g, u, v), 65535));
          const int bb = Gamma::unfix(clamp(bicubic(b, u, v), 65535));
          const int aa = Gamma::unfix(clamp(bicubic(a, u, v), 65535));

          *d++ = makeRgba(rr, gg, bb, aa);
        }

        if(Gui::updateProgress(y) < 0)
          break;
      }
    }

    Gui::hideProgress();
    Project::replaceImageFromBitmap(temp);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->drawMain(true);
  }

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    Items::width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    Items::height->value(s);
    Items::percent->value("100");
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

  void checkPercent()
  {
    int w = Project::bmp->cw;
    int h = Project::bmp->ch;
    char s[16];

    w = (float)w * ((float)atoi(Items::percent->value()) / 100);
    h = (float)h * ((float)atoi(Items::percent->value()) / 100);

    if(w < 1)
      w = 1;

    if(h < 1)
      h = 1;

    if(snprintf(s, sizeof(s), "%d", w) > 0)
      Items::width->value(s);

    if(snprintf(s, sizeof(s), "%d", h) > 0)
      Items::height->value(s);
  }

  void close()
  {
    int w = atoi(Items::width->value());
    int h = atoi(Items::height->value());

    if(Project::enoughMemory(w, h) == false)
      return;

    Items::dialog->hide();
    pushUndo();

    apply(w, h, Items::wrap->value());
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;
    int ww = 0;
    int hh = 0;

    Items::dialog = new DialogWindow(256, 0, "Scale Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 96, 24, "Width", (Fl_Callback *)checkWidth, 1, 32768);
    Items::width->center();
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 96, 24, "Height", (Fl_Callback *)checkHeight, 1, 32768);
    Items::height->center();
    y1 += 24 + 8;
    Items::percent = new InputInt(Items::dialog, 0, y1, 96, 24, "%", (Fl_Callback *)checkPercent, 1, 1000);
    Items::percent->value("100");
    Items::percent->center();
    y1 += 24 + 8;
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::keep_aspect = new CheckBox(Items::dialog, 0, y1, 16, 16, "Keep Aspect", 0);
    Items::keep_aspect->value(1);
    y1 += 16 + 8;
    Items::mode = new Fl_Choice(0, y1, 96, 24, "Mode:");
    Items::mode->labelsize(12);
    Items::mode->textsize(12);
    Items::mode->add("Nearest");
    Items::mode->add("Bilinear");
    Items::mode->add("Bicubic");
    Items::mode->value(2);
    Items::mode->align(FL_ALIGN_LEFT);
    Items::mode->measure_label(ww, hh);
    Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
    y1 += 24 + 8;
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

namespace RotateArbitrary
{
  namespace Items
  {
    DialogWindow *dialog;
    InputFloat *angle;
    InputFloat *scale;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(double angle, double scale, int overscroll/*, bool tile*/)
  {
    Bitmap *bmp = Project::bmp;

    // angle correction
    angle += 90;

    // rotation
    int du_col = (int)((std::sin(angle * (3.14159 / 180)) * scale) * 65536);
    int dv_col = (int)((std::sin((angle + 90) * (3.14159 / 180)) * scale) * 65536);
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
    du_col = (int)((std::sin(angle * (3.14159 / 180)) / scale) * 65536);
    dv_col = (int)((std::sin((angle + 90) * (3.14159 / 180)) / scale) * 65536);
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

        if(uu < bmp->cl || uu > bmp->cr || vv < bmp->ct || vv > bmp->cb)
          continue;

        const int xx = ((temp->cw) / 2) + x;
        if(xx < temp->cl || xx > temp->cr)
          continue;

        int c = *(bmp->row[vv] + uu);
        *(temp->row[yy] + xx) = c;
      }

      Gui::updateProgress(y - by1);
    }

    Gui::hideProgress();
    Project::replaceImageFromBitmap(temp);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
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
    Items::dialog->hide();
    pushUndo();

    apply(atof(Items::angle->value()), atof(Items::scale->value()),
          Project::overscroll/*, Items::tile->value()*/);
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Arbitrary Rotation");
    Items::angle = new InputFloat(Items::dialog, 0, y1, 96, 24, "Angle", 0, -359.99, 359.99);
    Items::angle->center();
    y1 += 24 + 8;
    Items::angle->value("0");
    Items::scale = new InputFloat(Items::dialog, 0, y1, 96, 24, "Scale (1-4)", 0, 1, 4);
    Items::scale->center();
    Items::scale->value("1.0");
    y1 += 24 + 8;
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
  RotateArbitrary::init();
}

void Transform::flipHorizontal()
{
  pushUndo();
  Project::bmp->flipHorizontal();
  Gui::getView()->drawMain(true);
}

void Transform::flipVertical()
{
  pushUndo();
  Project::bmp->flipVertical();
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

void Transform::rotateArbitrary()
{
  RotateArbitrary::begin();
}

void Transform::rotate90()
{
  pushUndo();

  int w = Project::bmp->w;
  int h = Project::bmp->h;

  // make copy
  Bitmap temp(w, h);
  Project::bmp->blit(&temp, 0, 0, 0, 0, w, h);

  // create rotated image
  delete Project::bmp;
  Project::bmp = new Bitmap(h, w);

  int *p = &temp.data[0];

  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
       *(Project::bmp->row[x] + h - 1 - y) = *p++;   
    } 
  } 

  Gui::getView()->drawMain(true);
}

void Transform::rotate180()
{
  pushUndo();
  Project::bmp->rotate180();
  Gui::getView()->drawMain(true);
}

