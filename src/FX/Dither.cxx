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
    CheckBox *gamma;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

enum
{
  THRESHOLD,
  FLOYD,
  JARVIS,
  ATKINSON,
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

namespace Jarvis
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 7, 5 },
    { 3, 5, 7, 5, 3 },
    { 1, 3, 5, 3, 1 }
  };

  const int div = 48;
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

void Dither::apply(Bitmap *bmp, int mode, bool fix_gamma)
{
  int (*matrix)[5] = Threshold::matrix;
  int w = 5, h = 3;
  int div = 1;

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
    case JARVIS:
      matrix = Jarvis::matrix;
      div = Jarvis::div;
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
  int x_end = bmp->cr;

  float c_y, c_cb, c_cr;

  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    for (int x = x_start; x != x_end + dir; x += dir)
    {
      const int c = bmp->getpixel(x, y);
      rgba_type rgba = getRgba(c);

      Blend::rgbToYcc(rgba.r, rgba.g, rgba.b, &c_y, &c_cb, &c_cr);
      const int alpha = rgba.a;
      const int old_y = c_y;
//      const int old_cb = c_cb;
//      const int old_cr = c_cr;

      const int pal_index = Project::palette->lookup(c);
      const int pal_color = Project::palette->data[pal_index];

      rgba = getRgba(pal_color);
      Blend::rgbToYcc(rgba.r, rgba.g, rgba.b, &c_y, &c_cb, &c_cr);
      bmp->setpixel(x, y, makeRgba(rgba.r, rgba.g, rgba.b, alpha));

      const float new_y = c_y;
//      const float new_cb = c_cb;
//      const float new_cr = c_cr;

      float err_y;

      if (fix_gamma)
      {
        err_y = Gamma::fix(old_y) - Gamma::fix(new_y);
      }
        else
      {
        err_y = old_y - new_y;
      }

//      float err_cb = old_cb - new_cb;
//      float err_cr = old_cr - new_cr;

      for (int j = 0; j < h; j++)
      {
        for (int i = 0; i < w; i++)
        {
          if (matrix[j][i] > 0)
          {
            if (dir == 1)
            {
              rgba = getRgba(bmp->getpixel(x - w / 2 + i, y + j));
            }
              else
            {
              rgba = getRgba(bmp->getpixel(x + w / 2 - i, y + j));
            }

            int r = rgba.r;
            int g = rgba.g;
            int b = rgba.b;

            Blend::rgbToYcc(r, g, b, &c_y, &c_cb, &c_cr);

            if (fix_gamma)
            {
              c_y = Gamma::fix(c_y);
            }

            c_y += (err_y * matrix[j][i]) / div;

            if (fix_gamma)
            {
              c_y = Gamma::unfix(clamp(c_y, 65535));
            }
              else
            {
              c_y = clamp(c_y, 255);
            }

            Blend::yccToRgb(c_y, c_cb, c_cr, &r, &g, &b);

            if (dir == 1)
            {
              bmp->setpixelSolid(x - w / 2 + i, y + j,
                                 makeRgba(r, g, b, rgba.a), 0);
            }
              else
            {
              bmp->setpixelSolid(x + w / 2 - i, y + j,
                                 makeRgba(r, g, b, rgba.a), 0);
            }
          }  
        }
      }
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
  const bool fix_gamma = Items::gamma->value();

  apply(Project::bmp, mode, fix_gamma);
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
  Items::mode->add("Jarvis-Judice-Ninke");
  Items::mode->add("Atkinson");
  Items::mode->value(0);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 32 + 16;

  Items::gamma = new CheckBox(Items::dialog, 0, y1, 16, 16, "Gamma Correction", 0);
  Items::gamma->center();
  y1 += 16 + 16;

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

