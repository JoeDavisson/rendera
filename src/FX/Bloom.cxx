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

#include "Bloom.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *radius;
    InputInt *blend;
    InputInt *threshold;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void Bloom::apply(Bitmap *bmp, int radius, int threshold, int blend)
{
  radius = (radius + 1) * 2;

  std::vector<int> kernel(radius);
  int div = 0;

  // bell curve
  const int b = radius / 2;

  for (int x = 0; x < radius; x++)
  {
    kernel[x] = 255 * std::exp(-((double)((x - b) * (x - b)) /
                                         ((b * b) / 2)));
    div += kernel[x];
  }

  Bitmap temp(bmp->cw, bmp->ch);
  Gui::progressShow(bmp->h);

  // x direction
  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = temp.row[y - bmp->ct];

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      int rr = 0;
      int gg = 0;
      int bb = 0;
      int aa = 0;

      for (int i = 0; i < radius; i++) 
      {
        const int mul = kernel[i];
        rgba_type rgba = getRgba(bmp->getpixel(x - radius / 2 + i, y));

        if (getlUnpacked(rgba.r, rgba.g, rgba.b) > threshold)
        {
          rr += Gamma::fix(rgba.r) * mul;
          gg += Gamma::fix(rgba.g) * mul;
          bb += Gamma::fix(rgba.b) * mul;
          aa += rgba.a * mul;
        }
      }

      rr = Gamma::unfix(rr / div);
      gg = Gamma::unfix(gg / div);
      bb = Gamma::unfix(bb / div);
      aa /= div;

      *p = makeRgba(rr, gg, bb, aa);
      p++;
    }

    if (Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressShow(bmp->h);

  // y direction
  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      int rr = 0;
      int gg = 0;
      int bb = 0;
      int aa = 0;

      for (int i = 0; i < radius; i++) 
      {
        const int mul = kernel[i];
        rgba_type rgba = getRgba(temp.getpixel(x - bmp->cl,
                                               y - radius / 2 + i - bmp->ct));

        rr += Gamma::fix(rgba.r) * mul;
        gg += Gamma::fix(rgba.g) * mul;
        bb += Gamma::fix(rgba.b) * mul;
        aa += rgba.a * mul;
      }

      rr = Gamma::unfix(rr / div);
      gg = Gamma::unfix(gg / div);
      bb = Gamma::unfix(bb / div);
      aa /= div;

      int c3 = makeRgba(rr, gg, bb, aa);

      *p = Blend::lighten(*p, c3, blend);
      p++;
    }

    if (Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void Bloom::close()
{
  Items::dialog->hide();
  Project::undo->push();

  const int radius = atoi(Items::radius->value());
  const int threshold = atoi(Items::threshold->value());
  const int blend = 255 - atoi(Items::blend->value()) * 2.55;

  apply(Project::bmp, radius, threshold, blend);
}

void Bloom::quit()
{
  Gui::progressHide();
  Items::dialog->hide();
}

void Bloom::begin()
{
  Items::dialog->show();
}

void Bloom::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(400, 0, "Bloom");

  Items::radius = new InputInt(Items::dialog, 0, y1, 128, 32, "Radius (0-100)", 0, 1, 100);
  Items::radius->value("16");
  Items::radius->center();
  y1 += 32 + 16;

  Items::threshold = new InputInt(Items::dialog, 0, y1, 128, 32, "Threshold (0-255)", 0, 0, 255);
  Items::threshold->value("128");
  Items::threshold->center();
  y1 += 32 + 16;

  Items::blend = new InputInt(Items::dialog, 0, y1, 128, 32, "Blend %", 0, 0, 100);
  Items::blend->value("25");
  Items::blend->center();
  y1 += 32 + 16;

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

