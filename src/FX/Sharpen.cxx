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

#include "Sharpen.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *amount;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void Sharpen::apply(Bitmap *bmp, int amount)
{
  Bitmap temp(bmp->cw, bmp->ch);

  Gui::progressShow(bmp->h);

  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = temp.row[y - bmp->cl];

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      int lum = 0;

      for (int j = 0; j < 3; j++) 
      {
        for (int i = 0; i < 3; i++) 
        {
          lum += getl(bmp->getpixel(x + i - 1, y + j - 1))
                   * FilterMatrix::sharpen[i][j];
        }
      }

      const int c = bmp->getpixel(x, y);

      lum = clamp(lum, 255);
      *p = Blend::trans(c, Blend::keepLum(c, lum), 255 - amount * 2.55);
      p++;
    }

    if (Gui::progressUpdate(y) < 0)
      return;
  }

  temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

  Gui::progressHide();

}

void Sharpen::close()
{
  Items::dialog->hide();
  Project::undo->push();

  apply(Project::bmp, atoi(Items::amount->value()));
}

void Sharpen::quit()
{
  Gui::progressHide();
  Items::dialog->hide();
}

void Sharpen::begin()
{
  Items::dialog->show();
}
 
void Sharpen::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(400, 0, "Sharpen");
  Items::amount = new InputInt(Items::dialog, 0, y1, 128, 32, "Amount %", 0, 0, 100);
  y1 += 32 + 16;
  Items::amount->value("10");
  Items::amount->center();

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

