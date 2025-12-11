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
      int c = bmp->getpixel(x, y);
      rgba_type rgba = getRgba(c);

      int old_r = rgba.r;
      int old_g = rgba.g;
      int old_b = rgba.b;
      int alpha = rgba.a;

      int pal_index = Project::palette->lookup(c);
      int pal_color = Project::palette->data[pal_index];

      rgba = getRgba(pal_color);
      Blend::rgbToYcc(rgba.r, rgba.g, rgba.b, &c_y, &c_cb, &c_cr);
      bmp->setpixel(x, y, makeRgba(rgba.r, rgba.g, rgba.b, alpha));

      int new_r = rgba.r;
      int new_g = rgba.g;
      int new_b = rgba.b;

      float er, eg, eb;

      if (fix_gamma)
      {
        er = Gamma::fix(old_r) - Gamma::fix(new_r);
        eg = Gamma::fix(old_g) - Gamma::fix(new_g);
        eb = Gamma::fix(old_b) - Gamma::fix(new_b);
      }
        else
      {
        er = old_r - new_r;
        eg = old_g - new_g;
        eb = old_b - new_b;
      }

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

            if (fix_gamma)
            {
              r = Gamma::fix(r);
              g = Gamma::fix(g);
              b = Gamma::fix(b);
            }

            r += (float)(er * matrix[j][i]) / div;
            g += (float)(eg * matrix[j][i]) / div;
            b += (float)(eb * matrix[j][i]) / div;

            if (fix_gamma)
            {
              r = Gamma::unfix(clamp(r, 65535));
              g = Gamma::unfix(clamp(g, 65535));
              b = Gamma::unfix(clamp(b, 65535));
            }
              else
            {
              r = clamp(r, 255);
              g = clamp(g, 255);
              b = clamp(b, 255);
            }

            c = makeRgba(r, g, b, rgba.a);

            if (dir == 1)
            {
              bmp->setpixelSolid(x - w / 2 + i, y + j, c, 0);
            }
              else
            {
              bmp->setpixelSolid(x + w / 2 - i, y + j, c, 0);
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

