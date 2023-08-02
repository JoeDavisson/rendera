/*
Copyright (c) 2023 Joe Davisson.

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

/*
Fast gaussian blur based on the idea of using an "accumulator" I learned
about here: https://blog.ivank.net/fastest-gaussian-blur.html

My implementation only works for sizes >= 3, so a box filter is used
for 1 & 2 instead.
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
    InputInt *size;
    InputInt *blend;
    Fl_Choice *mode;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void GaussianBlur::apply(Bitmap *bmp, float size, int blend, int mode)
{
  if (size < 1)
  {
    float f = size - (int)size;
    blend = 255 - f * 255;
    size = 1;
  }

  const int border = 128;
  const int matrix[9] = { 0, 1, 0, 1, 2, 1, 0, 1, 0 };

  // make copy, extend borders
  Bitmap src(bmp->w + border * 2, bmp->h + border * 2);
  bmp->blit(&src, 0, 0, border, border, bmp->w, bmp->h);
  extendBorders(&src, border);

  Bitmap temp(src.w, src.h);
  src.blit(&temp, 0, 0, 0, 0, src.w, src.h);

  // use alternative blur for sizes 1 & 2
  if (size < 3)
  {
    Gui::progressShow(bmp->h);

    for (int pass = 0; pass < size; pass++)
    {
      for (int y = bmp->ct; y <= bmp->cb; y++)
      {
        for (int x = bmp->cl; x <= bmp->cr; x++)
        {
          int r = 0;
          int g = 0;
          int b = 0;
          int a = 0;

          for (int i = 0; i < 9; i++)
          {
            const rgba_type rgba = getRgba(bmp->getpixel(x + i % 3 - 1,
                                                         y + i / 3 - 1));

            r += Gamma::fix(rgba.r) << matrix[i];
            g += Gamma::fix(rgba.g) << matrix[i];
            b += Gamma::fix(rgba.b) << matrix[i];
            a += rgba.a;
          }

          r >>= 4;
          g >>= 4;
          b >>= 4;
          a /= 9;

          r = Gamma::unfix(r);
          g = Gamma::unfix(g);
          b = Gamma::unfix(b);

          const int c1 = src.getpixel(x, y);
          const int c2 = makeRgba(r, g, b, a);

          switch (mode)
          {
            case 0:
              temp.setpixel(x + border, y + border,
                            Blend::trans(c1, c2, 0));
              break;
            case 1:
              temp.setpixel(x + border, y + border,
                Blend::trans(c1, Blend::keepLum(c2, getl(c1)), 0));
              break;
            case 2:
              temp.setpixel(x + border, y + border,
                            Blend::transAlpha(c1, c2, 0));
              break;
          }
        }

        if (Gui::progressUpdate(y) < 0)
          break;
      }
    }

    for (int y = 0; y < bmp->h; y++)
    {
      for (int x = 0; x < bmp->w; x++)
      {
        bmp->setpixelSolid(x, y, temp.getpixel(border + x, border + y), blend);
      }
    }
  
    Gui::progressHide();
    return;
  }

  if (size > border / 2 - 2)
    size = border / 2 - 2;

  // force odd value to prevent image shift
  if (((int)size & 1) == 0)
    size += 1;

  int larger = src.w > src.h ? src.w : src.h;

  std::vector<int> buf_r(larger, 0);
  std::vector<int> buf_g(larger, 0);
  std::vector<int> buf_b(larger, 0);
  std::vector<int> buf_a(larger, 0);

  Gui::progressShow(6, 1);

  int pass_count = 0;

  for (int pass = 0; pass < 3; pass++)
  {
    // x direction
    if (Gui::progressUpdate(pass_count++))
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
    if (Gui::progressUpdate(pass_count++))
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

  src.blit(bmp, border, border, 0, 0, bmp->w, bmp->h);
  Gui::progressHide();
}

void GaussianBlur::close()
{
  Items::dialog->hide();
  Project::undo->push();

  int size = atof(Items::size->value());
  int blend = 255 - atoi(Items::blend->value()) * 2.55;
  int mode = Items::mode->value();

  apply(Project::bmp, size, blend, mode);
}

void GaussianBlur::quit()
{
  Gui::progressHide();
  Items::dialog->hide();
}

void GaussianBlur::begin()
{
  Items::dialog->show();
}

void GaussianBlur::init()
{
  int y1 = 8;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(256, 0, "Gaussian Blur");

  Items::size = new InputInt(Items::dialog, 0, y1, 96, 24, "Size (1-60)", 0, 1, 60);
  y1 += 24 + 8;
  Items::size->value("1");
  Items::size->center();

  Items::blend = new InputInt(Items::dialog, 0, y1, 96, 24, "Blend %", 0, 0, 100);
  Items::blend->value("100");
  Items::blend->center();
  y1 += 24 + 8;

  Items::mode = new Fl_Choice(0, y1, 96, 24, "Mode:");
  Items::mode->labelsize(12);
  Items::mode->textsize(12);
  Items::mode->add("Normal");
  Items::mode->add("Color Only");
  Items::mode->add("Alpha Only");
  Items::mode->value(0);
  Items::mode->align(FL_ALIGN_LEFT);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2
                      - (Items::mode->w() + ww) / 2 + ww,
                      Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 24 + 8;

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

