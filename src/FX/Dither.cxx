/*
Copyright (c) 2025 Joe Davisson.

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

#include "Dither.H"

enum
{
  THRESHOLD,
  FLOYD,
  ATKINSON
};
 
enum
{
  PALETTE_MODE,
  BW_MODE
};
 
namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *dither_mode;
    InputFloat *bias;
    Fl_Choice *color_mode;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  int range(const int value, const int floor, const int ceiling)
  {
    if(value < floor)
      return floor;
    else if(value > ceiling)
      return ceiling;
    else
      return value;
  }

  int match(const int color_mode, const int c)
  {
    const int l = getl(c);
    Palette *pal = Project::palette;

    switch (color_mode)
    {
      case PALETTE_MODE:
        return pal->data[pal->lookup(c)];
      case BW_MODE:
        if (l < 128)
          return makeRgb(0, 0, 0);
        else
          return makeRgb(255, 255, 255);
      default:
        return makeRgb(0, 0, 0);
    }
  }
}

namespace Floyd
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 7, 0 },
    { 0, 3, 5, 1, 0 },
    { 0, 0, 0, 0, 0 },
  };

  int div = 16;
}

namespace Atkinson
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 0, 0, 1, 0, 0 },
  };

  int div = 8;
}

void Dither::apply(Bitmap *bmp, const int dither_mode,
                   const int color_mode, const double bias)
{
  if (dither_mode == THRESHOLD)
  {
    Progress::show(bmp->h);

    for (int y = 0; y < bmp->h; y++)
    {
      int *p = bmp->row[y];

      for (int x = 0; x < bmp->w; x++)
      {
        bmp->setpixel(x, y, match(color_mode, *p++));
      }

      if (Progress::update(y) < 0)
        return;
    }

    Progress::hide();
    return;
  }

  int (*matrix)[5] = Floyd::matrix;
  int div = Floyd::div;
  int mw = 5, mh = 3;

  std::vector<err_type> err_row(bmp->w);
  std::vector<std::vector<err_type>> err(mh, err_row);

  if (dither_mode == ATKINSON)
  {
    matrix = Atkinson::matrix;
    div = Atkinson::div;
  }

  div += bias;
  Progress::show(bmp->h);

  int dir = 1;
  int x_start = 0;
  int x_end = bmp->w - 1;

  for (int y = 0; y < 3; y++)
  {
    for (int x = x_start; x != x_end + dir; x += dir)
    {
      rgba_type rgba = getRgba(bmp->getpixel(x, y));

      err[y][x].r = Gamma::fix(rgba.r);
      err[y][x].g = Gamma::fix(rgba.g);
      err[y][x].b = Gamma::fix(rgba.b);
    }
  }

  for (int y = 0; y < bmp->h; y++)
  {
    for (int x = x_start; x != x_end + dir; x += dir)
    {
      int c1 = bmp->getpixel(x, y);
      const rgba_type rgba = getRgba(c1);
      const int alpha = rgba.a;

      int old_r = range(err[0][x].r, 0, 65535);
      int old_g = range(err[0][x].g, 0, 65535);
      int old_b = range(err[0][x].b, 0, 65535);

      const int c2 = makeRgb(Gamma::unfix(old_r),
                             Gamma::unfix(old_g),
                             Gamma::unfix(old_b));

      const int pal_color = match(color_mode, c2);

      const rgba_type pal_rgba = getRgba(pal_color);
      bmp->setpixel(x, y, makeRgba(pal_rgba.r, pal_rgba.g, pal_rgba.b, alpha));

      int new_r = Gamma::fix(pal_rgba.r);
      int new_g = Gamma::fix(pal_rgba.g);
      int new_b = Gamma::fix(pal_rgba.b);

      int er, eg, eb;

      er = old_r - new_r;
      eg = old_g - new_g;
      eb = old_b - new_b;

      for (int j = 0; j < mh; j++)
      {
        for (int i = 0; i < mw; i++)
        {
          if (matrix[j][i] > 0)
          {
            int x1;

            if (dir == 1)
              x1 = x - mw / 2 + i;
            else
              x1 = x + mw / 2 - i;
            
            int y1 = y + j;

            if (x1 < 0 || x1 >= bmp->w || y1 < 0 || y1 >= bmp->h)
              continue;

            int r = err[j][x1].r;
            int g = err[j][x1].g;
            int b = err[j][x1].b;

            const int mul_err = matrix[j][i];

            r += ((er * mul_err) / div);
            g += ((eg * mul_err) / div);
            b += ((eb * mul_err) / div);

            err[j][x1].r = r;
            err[j][x1].g = g;
            err[j][x1].b = b;
          }  
        }
      }
    }

    for (int x = x_start; x != x_end + dir; x += dir)
    {
      err[0][x].r = err[1][x].r;
      err[0][x].g = err[1][x].g;
      err[0][x].b = err[1][x].b;

      err[1][x].r = err[2][x].r;
      err[1][x].g = err[2][x].g;
      err[1][x].b = err[2][x].b;

      rgba_type rgba = getRgba(bmp->getpixel(x, y + 3));

      err[2][x].r = Gamma::fix(rgba.r);
      err[2][x].g = Gamma::fix(rgba.g);
      err[2][x].b = Gamma::fix(rgba.b);
    }

    dir = -dir;
    std::swap(x_start, x_end);

    if (Progress::update(y) < 0)
      return;
  }

  Progress::hide();
}

void Dither::close()
{
  Items::dialog->hide();
  Project::undo->push();

  const int dither_mode = Items::dither_mode->value();
  const int color_mode = Items::color_mode->value();
  const double div = Items::bias->value();

  apply(Project::bmp, dither_mode, color_mode, div);
}

void Dither::quit()
{
  Progress::hide();
  Items::dialog->hide();
}

void Dither::begin()
{
  Items::dialog->show();
}

void Dither::init()
{
  int y1 = 16;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(400, 0, "Apply Colors");

  Items::dither_mode = new Fl_Choice(0, y1, 192, 32, "Dither Mode:");
  Items::dither_mode->textsize(16);
  Items::dither_mode->labelsize(16);
  Items::dither_mode->add("No Dithering");
  Items::dither_mode->add("Floyd-Steinberg");
  Items::dither_mode->add("Atkinson");
  Items::dither_mode->value(0);
  Items::dither_mode->measure_label(ww, hh);
  Items::dither_mode->resize(Items::dialog->x() + Items::dialog->w() / 2
                             - (Items::dither_mode->w() + ww) / 2 + ww,
                             Items::dither_mode->y(),
                             Items::dither_mode->w(), Items::dither_mode->h());
  y1 += 32 + 16;

  Items::bias = new InputFloat(Items::dialog, 0, y1, 128, 32,
                                "Bias (-2 to 2)", 0, -2, 2);
  Items::bias->value(0);
  Items::bias->center();

  y1 += 32 + 16;

  Items::color_mode = new Fl_Choice(0, y1, 192, 32, "Color Mode:");
  Items::color_mode->textsize(16);
  Items::color_mode->labelsize(16);
  Items::color_mode->add("Use Palette");
  Items::color_mode->add("Black && White");
  Items::color_mode->value(0);
  Items::color_mode->measure_label(ww, hh);
  Items::color_mode->resize(Items::dialog->x() + Items::dialog->w() / 2
                            - (Items::color_mode->w() + ww) / 2 + ww,
                            Items::color_mode->y(),
                            Items::color_mode->w(), Items::color_mode->h());
  y1 += 32 + 16;

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

