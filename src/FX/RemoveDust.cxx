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

#include "RemoveDust.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *amount;
    CheckBox *invert;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void RemoveDust::apply(Bitmap *bmp, int amount)
{
  Gui::progressShow(bmp->h);

  for (int y = bmp->ct + 1; y <= bmp->cb - 1; y++)
  {
    int *p = bmp->row[y] + bmp->cl + 1;

    for (int x = bmp->cl + 1; x <= bmp->cr - 1; x++)
    {
      const int test = *p;
      int c[8];

      c[0] = bmp->getpixel(x + 1, y);
      c[1] = bmp->getpixel(x - 1, y);
      c[2] = bmp->getpixel(x, y + 1);
      c[3] = bmp->getpixel(x, y - 1);
      c[4] = bmp->getpixel(x - 1, y - 1);
      c[5] = bmp->getpixel(x + 1, y - 1);
      c[6] = bmp->getpixel(x - 1, y + 1);
      c[7] = bmp->getpixel(x + 1, y + 1);

      int r = 0;
      int g = 0;
      int b = 0;

      for (int i = 0; i < 8; i++)
      {
        rgba_type rgba = getRgba(c[i]);
        r += rgba.r;
        g += rgba.g;
        b += rgba.b;
      }

      const int avg = makeRgba(r / 8, g / 8, b / 8, geta(test));

      if ((getl(avg) - getl(test)) > amount)
        *p = avg;

      p++;
    }

    if (Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void RemoveDust::close()
{
  Items::dialog->hide();
  Project::undo->push();

  if (Items::invert->value())
    Invert::apply(Project::bmp);

  apply(Project::bmp, atoi(Items::amount->value()));

  if (Items::invert->value())
    Invert::apply(Project::bmp);
}

void RemoveDust::quit()
{
  Gui::progressHide();
  Items::dialog->hide();
}

void RemoveDust::begin()
{
  Items::dialog->show();
}

void RemoveDust::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(400, 0, "Remove Dust");

  Items::amount = new InputInt(Items::dialog, 0, y1, 128, 32, "Amount (1-10)", 0, 1, 10);
  y1 += 32 + 16;
  Items::amount->value("4");
  Items::amount->center();

  Items::invert = new CheckBox(Items::dialog, 0, y1, 16, 16, "Invert First", 0);
  y1 += 16 + 16;
  Items::invert->center();

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

