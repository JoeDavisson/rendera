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
  if (size < 1.0)
  {
    applySmall(bmp, size, blend, mode);
  }
    else
  {
    applyLarge(bmp, (size + 1.0) / 2, blend, mode);
  }
}

void GaussianBlur::applySmall(Bitmap *bmp, float size, int blend, int mode)
{
  int w = bmp->w;
  int h = bmp->h;

  Bitmap src(w, h);
  Bitmap temp(w, h);
  bmp->blit(&src, 0, 0, 0, 0, w, h);

  size += 1.0;

  float alpha = (size - 1) / 2;
  float div = size;

  Progress::show(6, 1);
  int pass_count = 0;

  for (int pass = 0; pass < 3; pass++)
  {
    if (Progress::update(pass_count++) < 0) { break; }
        
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        int left = std::max(0, x - 1);
        int right = std::min(w - 1, x + 1);

        rgba_type rgba1 = getRgba(*(src.row[y] + left));
        rgba_type rgba2 = getRgba(*(src.row[y] + x));
        rgba_type rgba3 = getRgba(*(src.row[y] + right));

        float acc_r = (alpha * Gamma::fix(rgba1.r) +
                               Gamma::fix(rgba2.r) +
                       alpha * Gamma::fix(rgba3.r)) / div;

        float acc_g = (alpha * Gamma::fix(rgba1.g) +
                               Gamma::fix(rgba2.g) +
                       alpha * Gamma::fix(rgba3.g)) / div;

        float acc_b = (alpha * Gamma::fix(rgba1.b) +
                               Gamma::fix(rgba2.b) +
                       alpha * Gamma::fix(rgba3.b)) / div;

        float acc_a = (alpha * rgba1.a + rgba2.a + alpha * rgba3.a) / div;

        *(temp.row[y] + x) = makeRgba(Gamma::unfix(acc_r),
                                      Gamma::unfix(acc_g),
                                      Gamma::unfix(acc_b),
                                      acc_a);
      }
    }

    if (Progress::update(pass_count++) < 0) { break; }
        
    for (int x = 0; x < w; x++)
    {
      for (int y = 0; y < h; y++)
      {
        int up = std::max(0, y - 1);
        int down = std::min(h - 1, y + 1);

        rgba_type rgba1 = getRgba(*(temp.row[up] + x));
        rgba_type rgba2 = getRgba(*(temp.row[y] + x));
        rgba_type rgba3 = getRgba(*(temp.row[down] + x));

        float acc_r = (alpha * Gamma::fix(rgba1.r) +
                               Gamma::fix(rgba2.r) +
                       alpha * Gamma::fix(rgba3.r)) / div;

        float acc_g = (alpha * Gamma::fix(rgba1.g) +
                               Gamma::fix(rgba2.g) +
                       alpha * Gamma::fix(rgba3.g)) / div;

        float acc_b = (alpha * Gamma::fix(rgba1.b) +
                               Gamma::fix(rgba2.b) +
                       alpha * Gamma::fix(rgba3.b)) / div;

        float acc_a = (alpha * rgba1.a + rgba2.a + alpha * rgba3.a) / div;

        int c1 = *(src.row[y] + x);
        int c2 = makeRgba(Gamma::unfix(acc_r),
                          Gamma::unfix(acc_g),
                          Gamma::unfix(acc_b),
                          acc_a);
        int c;

        if (mode == 0)
        {
          c = Blend::trans(c1, c2, blend);
        }
        else if (mode == 1)
        {
          c = Blend::trans(c1, Blend::keepLum(c2, getl(c1)), blend);
        }
          else
        {
          c = Blend::transAlpha(c1, c2, blend);
        }

        *(src.row[y] + x) = c;
      }
    }
  }

  src.blit(bmp, 0, 0, 0, 0, w, h);
  Progress::hide();
}

void GaussianBlur::applyLarge(Bitmap *bmp, float radius, int blend, int mode)
{
  int r = (int)radius;
  int div = 2 * r + 1;
  int w = bmp->w;
  int h = bmp->h;

  Bitmap src(w, h);
  Bitmap temp(w, h);
  bmp->blit(&src, 0, 0, 0, 0, w, h);

  Progress::show(6, 1);
  int pass_count = 0;

  for (int pass = 0; pass < 3; pass++)
  {
    if (Progress::update(pass_count++) < 0) { break; }

    for (int y = 0; y < h; y++)
    {
      int acc_r = 0, acc_g = 0, acc_b = 0, acc_a = 0;

      for (int x = -r; x <= r; x++)
      {
        int offset_x = clamp(x, w - 1);

        rgba_type rgba = getRgba(*(src.row[y] + offset_x));

        acc_r += Gamma::fix(rgba.r);
        acc_g += Gamma::fix(rgba.g);
        acc_b += Gamma::fix(rgba.b);
        acc_a += rgba.a;
      }

      *(temp.row[y]) = makeRgba(Gamma::unfix(acc_r / div),
                                Gamma::unfix(acc_g / div),
                                Gamma::unfix(acc_b / div),
                                acc_a / div);

      for (int x = 1; x < w; x++)
      {
        int old_edge = clamp(x - r - 1, w - 1);
        int new_edge = clamp(x + r, w - 1);

        rgba_type rgba_out = getRgba(*(src.row[y] + old_edge));
        rgba_type rgba_in = getRgba(*(src.row[y] + new_edge));

        acc_r += Gamma::fix(rgba_in.r) - Gamma::fix(rgba_out.r);
        acc_g += Gamma::fix(rgba_in.g) - Gamma::fix(rgba_out.g);
        acc_b += Gamma::fix(rgba_in.b) - Gamma::fix(rgba_out.b);
        acc_a += rgba_in.a - rgba_out.a;

        *(temp.row[y] + x) = makeRgba(Gamma::unfix(acc_r / div),
                                      Gamma::unfix(acc_g / div),
                                      Gamma::unfix(acc_b / div),
                                      acc_a / div);
      }
    }

    if (Progress::update(pass_count++) < 0) { break; }

    for (int x = 0; x < w; x++)
    {
      int acc_r = 0, acc_g = 0, acc_b = 0, acc_a = 0;

      for (int y = -r; y <= r; y++)
      {
        int offset_y = clamp(y, h - 1);

        rgba_type rgba = getRgba(*(temp.row[offset_y] + x));

        acc_r += Gamma::fix(rgba.r);
        acc_g += Gamma::fix(rgba.g);
        acc_b += Gamma::fix(rgba.b);
        acc_a += rgba.a;
      }

      int c1 = *(src.row[0] + x);

      int c2 = makeRgba(Gamma::unfix(acc_r / div),
                        Gamma::unfix(acc_g / div),
                        Gamma::unfix(acc_b / div),
                        acc_a / div);

      *(src.row[0] + x) = Blend::trans(c1, c2, blend);

      for (int y = 1; y < h; y++)
      {
        int old_edge = clamp(y - r - 1, h - 1);
        int new_edge = clamp(y + r, h - 1);

        rgba_type rgba_out = getRgba(*(temp.row[old_edge] + x));
        rgba_type rgba_in  = getRgba(*(temp.row[new_edge] + x));

        acc_r += Gamma::fix(rgba_in.r) - Gamma::fix(rgba_out.r);
        acc_g += Gamma::fix(rgba_in.g) - Gamma::fix(rgba_out.g);
        acc_b += Gamma::fix(rgba_in.b) - Gamma::fix(rgba_out.b);
        acc_a += rgba_in.a - rgba_out.a;

        int c1 = *(src.row[y] + x);

        int c2 = makeRgba(Gamma::unfix(acc_r / div),
                          Gamma::unfix(acc_g / div),
                          Gamma::unfix(acc_b / div),
                          acc_a / div);

        int c;

        if (mode == 0)
        {
          c = Blend::trans(c1, c2, blend);
        }
        else if (mode == 1)
        {
          c = Blend::trans(c1, Blend::keepLum(c2, getl(c1)), blend);
        }
          else
        {
          c = Blend::transAlpha(c1, c2, blend);
        }

        *(src.row[y] + x) = c;
      }
    }
  }

  src.blit(bmp, 0, 0, 0, 0, w, h);
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

  Items::size = new InputFloat(Items::dialog, 0, y1, 128, 32, "Size (.01-100)", 0, 0.01, 100);
  Items::size->value(1);
  Items::size->center();
  y1 += 32 + 16;

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

