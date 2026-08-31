/*
Copyright (c) 2026 Joe Davisson.

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

#include "GaussianBlur.H"

namespace
{
  // extends borders so edges are handled correctly
  void extendBorders(Bitmap *bmp, int border)
  {
    const int cl = border;
    const int cr = bmp->w - border - 1;
    const int ct = border;
    const int cb = bmp->h - border - 1;
    const int w = bmp->w;
    const int h = bmp->h;

    // left
    for (int y = ct; y <= cb; y++)
      bmp->hline(0, y, cl - 1, bmp->getpixel(cl, y));

    // right
    for (int y = ct; y <= cb; y++)
      bmp->hline(cr + 1, y, w - 1, bmp->getpixel(cr, y));

    // top
    for (int x = cl; x <= cr; x++)
      bmp->vline(0, x, ct - 1, bmp->getpixel(x, ct));

    // bottom
    for (int x = cl; x <= cr; x++)
      bmp->vline(cb + 1, x, h - 1, bmp->getpixel(x, cb));

    // upper-left
    bmp->rectfill(0, 0, cl - 1, ct - 1, bmp->getpixel(cl, ct));

    // upper-right
    bmp->rectfill(cr, 0, w - 1, ct - 1, bmp->getpixel(cr, ct));

    // lower-left
    bmp->rectfill(0, cb + 1, cl - 1, h - 1, bmp->getpixel(cl, cb));

    // lower-right
    bmp->rectfill(cr + 1, cb + 1, w - 1, h - 1, bmp->getpixel(cr, cb));
  }
}

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputFloat *size;
    InputInt *blend;
    Fl_Choice *mode;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void GaussianBlur::apply(Bitmap *bmp, float size, int blend, int mode)
{
  const int border = 128;

  // make copy, extend borders
  Bitmap src(bmp->w + border * 2, bmp->h + border * 2);
  bmp->blit(&src, 0, 0, border, border, bmp->w, bmp->h);
  extendBorders(&src, border);

  Bitmap temp(src.w, src.h);
  src.blit(&temp, 0, 0, 0, 0, src.w, src.h);

  size += 1.0;

  if (size > border / 2 - 2)
    size = border / 2 - 2;

  Progress::show(6, 1);

  int pass_count = 0;

  if (size < 3.0)
  {
    const float alpha = (size - 1.0) / 2;
    const float mul = 1.0 / size;

    for (int pass = 0; pass < 3; pass++)
    {
      // x direction
      if (Progress::update(pass_count++) < 0)
        break;

      for (int y = src.ct; y <= src.cb; y++)
      {
        for (int x = src.cl; x <= src.cr; x++)
        {
          rgba_type rgba1 = getRgba(src.getpixel(x - 1, y));
          rgba_type rgba2 = getRgba(src.getpixel(x, y));
          rgba_type rgba3 = getRgba(src.getpixel(x + 1, y));

          const float r1 = Gamma::fix(rgba1.r);
          const float g1 = Gamma::fix(rgba1.g);
          const float b1 = Gamma::fix(rgba1.b);
          const float a1 = rgba1.a;
          const float r2 = Gamma::fix(rgba2.r);
          const float g2 = Gamma::fix(rgba2.g);
          const float b2 = Gamma::fix(rgba2.b);
          const float a2 = rgba2.a;
          const float r3 = Gamma::fix(rgba3.r);
          const float g3 = Gamma::fix(rgba3.g);
          const float b3 = Gamma::fix(rgba3.b);
          const float a3 = rgba3.a;

          const int r = Gamma::unfix((alpha * r1 + r2 + alpha * r3) * mul);
          const int g = Gamma::unfix((alpha * g1 + g2 + alpha * g3) * mul);
          const int b = Gamma::unfix((alpha * b1 + b2 + alpha * b3) * mul);
          const int a = (alpha * a1 + a2 + alpha * a3) * mul;

          const int c = makeRgba(r, g, b, a);
          
          temp.setpixel(x - size / 2 + 1, y - size / 2 + 1, c);
        }
      }

      // y direction
      if (Progress::update(pass_count++) < 0)
        break;

      for (int x = src.cl; x <= src.cr; x++)
      {
        for (int y = src.ct; y <= src.cb; y++)
        {
          rgba_type rgba1 = getRgba(temp.getpixel(x, y - 1));
          rgba_type rgba2 = getRgba(temp.getpixel(x, y));
          rgba_type rgba3 = getRgba(temp.getpixel(x, y + 1));

          const float r1 = Gamma::fix(rgba1.r);
          const float g1 = Gamma::fix(rgba1.g);
          const float b1 = Gamma::fix(rgba1.b);
          const float a1 = rgba1.a;
          const float r2 = Gamma::fix(rgba2.r);
          const float g2 = Gamma::fix(rgba2.g);
          const float b2 = Gamma::fix(rgba2.b);
          const float a2 = rgba2.a;
          const float r3 = Gamma::fix(rgba3.r);
          const float g3 = Gamma::fix(rgba3.g);
          const float b3 = Gamma::fix(rgba3.b);
          const float a3 = rgba3.a;

          const int r = Gamma::unfix((alpha * r1 + r2 + alpha * r3) * mul);
          const int g = Gamma::unfix((alpha * g1 + g2 + alpha * g3) * mul);
          const int b = Gamma::unfix((alpha * b1 + b2 + alpha * b3) * mul);
          const int a = (alpha * a1 + a2 + alpha * a3) * mul;

          int c1 = src.getpixel(x, y);
          int c2 = makeRgba(r, g, b, a);

          switch (mode)
          {
            case 0:
              src.setpixel(x, y, Blend::trans(c1, c2, blend));
              break;
            case 1:
              src.setpixel(x, y,
                Blend::trans(c1, Blend::keepLum(c2, getl(c1)), blend));
              break;
            case 2:
              src.setpixel(x, y, Blend::transAlpha(c1, c2, blend));
              break;
          }
        }
      }
    }
  }
    else
  {
    int larger = src.w > src.h ? src.w : src.h;

    std::vector<int> buf_r(larger, 0);
    std::vector<int> buf_g(larger, 0);
    std::vector<int> buf_b(larger, 0);
    std::vector<int> buf_a(larger, 0);

    // force odd value to prevent image shift
    if (((int)size & 1) == 0) { size += 1.0; }

    for (int pass = 0; pass < 3; pass++)
    {
      // x direction
      if (Progress::update(pass_count++) < 0)
        break;

      for (int y = src.ct; y <= src.cb; y++)
      {
        for (int x = 0; x < src.w; x++)
        {
          rgba_type rgba = getRgba(src.getpixel(x, y));
          buf_r[x] = Gamma::fix(rgba.r);
          buf_g[x] = Gamma::fix(rgba.g);
          buf_b[x] = Gamma::fix(rgba.b);
          buf_a[x] = rgba.a;
        }

        int accum_r = 0;
        int accum_g = 0;
        int accum_b = 0;
        int accum_a = 0;
        int div = 1;

        for (int x = src.cl; x <= src.cr; x++)
        {
          const int mx = x - div;

          if (mx >= 0)
          {
            accum_r -= buf_r[mx];
            accum_g -= buf_g[mx];
            accum_b -= buf_b[mx];
            accum_a -= buf_a[mx];
          }

          accum_r += buf_r[x];
          accum_g += buf_g[x];
          accum_b += buf_b[x];
          accum_a += buf_a[x];

          div++;

          if (div > size)
            div = size;

          const int c = makeRgba(Gamma::unfix(accum_r / div),
                                 Gamma::unfix(accum_g / div),
                                 Gamma::unfix(accum_b / div),
                                 accum_a / div);

          temp.setpixel(x - size / 2 + 1, y - size / 2 + 1, c);
        }
      }

      // y direction
      if (Progress::update(pass_count++) < 0)
        break;

      for (int x = src.cl; x <= src.cr; x++)
      {
        for (int y = 0; y < src.h; y++)
        {
          rgba_type rgba = getRgba(temp.getpixel(x, y));
          buf_r[y] = Gamma::fix(rgba.r);
          buf_g[y] = Gamma::fix(rgba.g);
          buf_b[y] = Gamma::fix(rgba.b);
          buf_a[y] = rgba.a;
        }

        int accum_r = 0;
        int accum_g = 0;
        int accum_b = 0;
        int accum_a = 0;
        int div = 1;

        for (int y = src.ct; y <= src.cb; y++)
        {
          const int my = y - div;

          if (my >= 0)
          {
            accum_r -= buf_r[my];
            accum_g -= buf_g[my];
            accum_b -= buf_b[my];
            accum_a -= buf_a[my];
          }

          accum_r += buf_r[y];
          accum_g += buf_g[y];
          accum_b += buf_b[y];
          accum_a += buf_a[y];

          div++;

          if (div > size)
            div = size;

          int c1 = src.getpixel(x, y);

          const int c2 = makeRgba(Gamma::unfix(accum_r / div),
                                  Gamma::unfix(accum_g / div),
                                  Gamma::unfix(accum_b / div),
                                  accum_a / div);

          switch (mode)
          {
            case 0:
              src.setpixel(x, y, Blend::trans(c1, c2, blend));
              break;
            case 1:
              src.setpixel(x, y,
                Blend::trans(c1, Blend::keepLum(c2, getl(c1)), blend));
              break;
            case 2:
              src.setpixel(x, y, Blend::transAlpha(c1, c2, blend));
              break;
          }
        }
      }
    }
  }

  src.blit(bmp, border, border, 0, 0, bmp->w, bmp->h);
  Progress::hide();
}

void GaussianBlur::close()
{
  Items::dialog->hide();
  Project::undo->push();

  float size = Items::size->value();
  int blend = 255 - Items::blend->value() * 2.55;
  int mode = Items::mode->value();

  apply(Project::bmp, size, blend, mode);
}

void GaussianBlur::quit()
{
  Progress::hide();
  Items::dialog->hide();
}

void GaussianBlur::begin()
{
  Items::dialog->show();
}

void GaussianBlur::init()
{
  int y1 = 16;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(400, 0, "Gaussian Blur");

  Items::size = new InputFloat(Items::dialog, 0, y1, 128, 32, "Size (0-59)", 0, 0, 59);
  y1 += 32 + 16;
  Items::size->value(1);
  Items::size->center();

  Items::blend = new InputInt(Items::dialog, 0, y1, 128, 32, "Blend %", 0, 0, 100);
  Items::blend->value(100);
  Items::blend->center();
  y1 += 32 + 16;

  Items::mode = new Fl_Choice(0, y1, 128, 32, "Mode:");
  Items::mode->labelsize(16);
  Items::mode->textsize(16);
  Items::mode->add("Normal");
  Items::mode->add("Color Only");
  Items::mode->add("Alpha Only");
  Items::mode->value(0);
  Items::mode->align(FL_ALIGN_LEFT);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2
                      - (Items::mode->w() + ww) / 2 + ww,
                      Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 32 + 16;

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

