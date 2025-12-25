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

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *mode;
    InputInt *limit;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  float toLinear(const float val)
  {
    return std::pow(val / 255.0, 2.2);
  }

  float toRGB(const float val)
  {
    return std::pow(val, 1.0 / 2.2) * 255.0;
  }

  float range(const float value, const float floor, const float ceiling)
  {
    if(value < floor)
      return floor;
    else if(value > ceiling)
      return ceiling;
    else
      return value;
  }
}

enum
{
  THRESHOLD,
  FLOYD,
  ATKINSON
};
 
namespace Threshold
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };

  const int div = 1;
}

namespace Floyd
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 7, 0 },
    { 0, 3, 5, 1, 0 },
    { 0, 0, 0, 0, 0 },
  };

  const int div = 16;
}

namespace Atkinson
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 0, 0, 1, 0, 0 }
  };

  const int div = 8;
}

void Dither::apply(Bitmap *bmp, const int mode, const int limit)
{
  int (*matrix)[5] = Threshold::matrix;
  int w = 5, h = 3;
  int div = 1;
  const float limit_power = 1.0 + (float)(limit + 0) / 10;

  std::vector<err_type> err_row(bmp->w);
  std::vector<std::vector<err_type>> err(h, err_row);

  switch (mode)
  {
    case THRESHOLD:
      matrix = Threshold::matrix;
      div = Threshold::div;
      break;
    case FLOYD:
      matrix = Floyd::matrix;
      div = Floyd::div;
      break;
    case ATKINSON:
      matrix = Atkinson::matrix;
      div = Atkinson::div;
      break;
    default:
      matrix = Threshold::matrix;
      div = Threshold::div;
      break;
  }

  Progress::show(bmp->h);

  int dir = 1;
  int x_start = 0;
  int x_end = bmp->w - 1;

  for (int y = 0; y < 3; y++)
  {
    for (int x = x_start; x != x_end + dir; x += dir)
    {
      rgba_type rgba = getRgba(bmp->getpixel(x, y));

      err[y][x].r = toLinear(rgba.r);
      err[y][x].g = toLinear(rgba.g);
      err[y][x].b = toLinear(rgba.b);
    }
  }

  for (int y = 0; y < bmp->h; y++)
  {
    for (int x = x_start; x != x_end + dir; x += dir)
    {
      int c = bmp->getpixel(x, y);
      rgba_type rgba = getRgba(c);

      float old_r = range(err[0][x].r, 0.0, 1.0);
      float old_g = range(err[0][x].g, 0.0, 1.0);
      float old_b = range(err[0][x].b, 0.0, 1.0);
      int alpha = rgba.a;

      c = makeRgb(toRGB(old_r), toRGB(old_g), toRGB(old_b));

      int pal_index = Project::palette->lookup(c);
      int pal_color = Project::palette->data[pal_index];

      rgba = getRgba(pal_color);
      bmp->setpixel(x, y, makeRgba(rgba.r, rgba.g, rgba.b, alpha));

      float new_r = toLinear(rgba.r);
      float new_g = toLinear(rgba.g);
      float new_b = toLinear(rgba.b);

      float er, eg, eb;

      er = old_r - new_r;
      eg = old_g - new_g;
      eb = old_b - new_b;

      for (int j = 0; j < h; j++)
      {
        for (int i = 0; i < w; i++)
        {
          if (matrix[j][i] > 0)
          {
            int x1;

            if (dir == 1)
              x1 = x - w / 2 + i;
            else
              x1 = x + w / 2 - i;
            
            int y1 = y + j;

            if (x1 < 0 || x1 >= bmp->w || y1 < 0 || y1 >= bmp->h)
              continue;

            float r = err[j][x1].r;
            float g = err[j][x1].g;
            float b = err[j][x1].b;

            const float mul_err = std::pow(matrix[j][i], limit_power);
            const float div_err = std::pow(div, limit_power);

            r += (er * mul_err) / div_err;
            g += (eg * mul_err) / div_err;
            b += (eb * mul_err) / div_err;

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

      err[2][x].r = toLinear(rgba.r);
      err[2][x].g = toLinear(rgba.g);
      err[2][x].b = toLinear(rgba.b);
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

  const int mode = Items::mode->value();
  const int limit = Items::limit->value();

  apply(Project::bmp, mode, limit);
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

  Items::mode = new Fl_Choice(0, y1, 192, 32, "Dither:");
  Items::mode->tooltip("Dither Mode");
  Items::mode->textsize(16);
  Items::mode->labelsize(16);
  Items::mode->add("No Dithering");
  Items::mode->add("Floyd-Steinberg");
  Items::mode->add("Atkinson");
  Items::mode->value(0);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2
                      - (Items::mode->w() + ww) / 2 + ww,
                      Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 32 + 16;

  Items::limit = new InputInt(Items::dialog, 0, y1, 96, 32,
                              "Limit (0 - 9)", 0, 0, 9);
  Items::limit->value(0);
  Items::limit->center();

  y1 += 32 + 16;

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

