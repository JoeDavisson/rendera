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

#include "Dither.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *mode;
    CheckBox *gamma;
    CheckBox *lum_only;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

enum
{
  THRESHOLD,
  FLOYD,
  JARVIS,
  STUCKI,
  ATKINSON,
  SIERRA
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

namespace Stucki
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 8, 4 },
    { 2, 4, 8, 4, 2 },
    { 1, 2, 4, 2, 1 }
  };

  const int div = 42;
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

namespace Sierra
{
  int matrix[3][5] =
  {
    { 0, 0, 0, 5, 3 },
    { 2, 4, 5, 4, 2 },
    { 0, 2, 3, 2, 0 }
  };

  const int div = 32;
}

void Dither::apply(Bitmap *bmp, int mode, bool fix_gamma, bool lum_only)
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
    case STUCKI:
      matrix = Stucki::matrix;
      div = Stucki::div;
      break;
    case ATKINSON:
      matrix = Atkinson::matrix;
      div = Atkinson::div;
      break;
    case SIERRA:
      matrix = Sierra::matrix;
      div = Sierra::div;
      break;
    default:
      matrix = Threshold::matrix;
      div = Threshold::div;
      break;
  }

  Gui::progressShow(bmp->h);

  if (lum_only)
  {
    for (int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for (int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int alpha = geta(*p);
        const int old_l = getl(*p);
        const int pal_index = Project::palette->lookup(Blend::keepLum(*p, old_l));
        const int cp = Project::palette->data[pal_index];

        rgba_type rgba = getRgba(cp);
        *p = makeRgba(rgba.r, rgba.g, rgba.b, alpha);

        const int new_l = getl(*p);
        int el;

        if (fix_gamma)
        {
          el = Gamma::fix(old_l) - Gamma::fix(new_l);

          if (el < -16383) el = -16383;
          if (el > 16383) el = 16383;
        }
          else
        {
          el = old_l - new_l;

          if (el < -127) el = -127;
          if (el > 127) el = 127;
        }

        for (int j = 0; j < h; j++)
        {
          for (int i = 0; i < w; i++)
          {
            if (matrix[j][i] > 0)
            {
              int c = bmp->getpixel(x - w / 2 + i, y + j);
              int l = getl(c);

              if (fix_gamma)
                l = Gamma::fix(l); 

              l += (el * matrix[j][i]) / div;

              if (fix_gamma)
                l = Gamma::unfix(clamp(l, 65535));
              else
                l = clamp(l, 255);

              rgba = getRgba(Blend::keepLum(c, l));

              bmp->setpixelSolid(x - w / 2 + i, y + j,
                               makeRgba(rgba.r, rgba.g, rgba.b, rgba.a), 0);
            }  
          }
        }

        p++;
      }

      if (Gui::progressUpdate(y) < 0)
        return;
    }
  }
    else
  {
    for (int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for (int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);
        const int alpha = rgba.a;
        const int old_r = rgba.r;
        const int old_g = rgba.g;
        const int old_b = rgba.b;

        const int pal_index = Project::palette->lookup(*p);
        const int c = Project::palette->data[pal_index];

        rgba = getRgba(c);
        *p = makeRgba(rgba.r, rgba.g, rgba.b, alpha);

        const int new_r = rgba.r;
        const int new_g = rgba.g;
        const int new_b = rgba.b;
        int er, eg, eb;

        if (fix_gamma)
        {
          er = Gamma::fix(old_r) - Gamma::fix(new_r);
          eg = Gamma::fix(old_g) - Gamma::fix(new_g);
          eb = Gamma::fix(old_b) - Gamma::fix(new_b);

          if (er < -16383) er = -16383;
          if (er > 16383) er = 16383;
          if (eg < -16383) eg = -16383;
          if (eg > 16383) eg = 16383;
          if (eb < -16383) eb = -16383;
          if (eb > 16383) eb = 16383;
        }
          else
        {
          er = old_r - new_r;
          eg = old_g - new_g;
          eb = old_b - new_b;

          if (er < -127) er = -127;
          if (er > 127) er = 127;
          if (eg < -127) eg = -127;
          if (eg > 127) eg = 127;
          if (eb < -127) eb = -127;
          if (eb > 127) eb = 127;
        }

        for (int j = 0; j < h; j++)
        {
          for (int i = 0; i < w; i++)
          {
            if (matrix[j][i] > 0)
            {
              rgba = getRgba(bmp->getpixel(x - w / 2 + i, y + j));
              int r, g, b;

              if (fix_gamma)
              {
                r = Gamma::fix(rgba.r); 
                g = Gamma::fix(rgba.g); 
                b = Gamma::fix(rgba.b);
              }
                else
              {
                r = rgba.r; 
                g = rgba.g; 
                b = rgba.b; 
              }

              r += (er * matrix[j][i]) / div;
              g += (eg * matrix[j][i]) / div;
              b += (eb * matrix[j][i]) / div;

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

              bmp->setpixelSolid(x - w / 2 + i, y + j,
                               makeRgba(r, g, b, rgba.a), 0);
            }  
          }

        }

        p++;
      }

      if (Gui::progressUpdate(y) < 0)
        return;
    }
  }

  Gui::progressHide();
}

void Dither::close()
{
  Items::dialog->hide();
  Project::undo->push();

  const int mode = Items::mode->value();
  const bool fix_gamma = Items::gamma->value();
  const bool lum_only = Items::lum_only->value();

  apply(Project::bmp, mode, fix_gamma, lum_only);
}

void Dither::quit()
{
  Gui::progressHide();
  Items::dialog->hide();
}

void Dither::begin()
{
  Items::dialog->show();
}

void Dither::init()
{
  int y1 = 8;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(256, 0, "Apply Colors");
  Items::mode = new Fl_Choice(0, y1, 128, 24, "Dither:");
  Items::mode->tooltip("Dither Mode");
  Items::mode->textsize(10);
  Items::mode->add("No Dithering");
  Items::mode->add("Floyd-Steinberg");
  Items::mode->add("Jarvis-Judice-Ninke");
  Items::mode->add("Stucki");
  Items::mode->add("Atkinson");
  Items::mode->add("Sierra");
  Items::mode->value(0);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 24 + 8;
  Items::gamma = new CheckBox(Items::dialog, 0, y1, 16, 16, "Gamma Correction", 0);
  Items::gamma->center();
  y1 += 16 + 8;
  Items::lum_only = new CheckBox(Items::dialog, 0, y1, 16, 16, "Luminosity Based", 0);
  Items::lum_only->center();
  y1 += 16 + 8;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

