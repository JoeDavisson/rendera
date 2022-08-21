/*
Copyright (c) 2021 Joe Davisson.

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

#include "Sobel.H"

namespace Sobel::Items
{
  DialogWindow *dialog;
  InputInt *amount;
  Fl_Button *ok;
  Fl_Button *cancel;
}

void Sobel::apply()
{
  Bitmap *bmp = Project::bmp;
  int amount = atoi(Items::amount->value());
  int div = 1;

  Bitmap temp(bmp->cw, bmp->ch);
  Gui::showProgress(bmp->h);

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = temp.row[y - bmp->cl];

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int r1 = 0;
      int g1 = 0;
      int b1 = 0;
      int r2 = 0;
      int g2 = 0;
      int b2 = 0;

      for(int j = 0; j < 3; j++) 
      {
	for(int i = 0; i < 3; i++) 
	{
	  const rgba_type rgba = getRgba(bmp->getpixel(x + i - 1, y + j - 1));

	  r1 += rgba.r * FilterMatrix::sobel1[i][j];
	  r2 += rgba.r * FilterMatrix::sobel2[i][j];
	  g1 += rgba.g * FilterMatrix::sobel1[i][j];
	  g2 += rgba.g * FilterMatrix::sobel2[i][j];
	  b1 += rgba.b * FilterMatrix::sobel1[i][j];
	  b2 += rgba.b * FilterMatrix::sobel2[i][j];
	}
      }

      int r = std::sqrt(r1 * r1 + r2 * r2);
      int g = std::sqrt(g1 * g1 + g2 * g2);
      int b = std::sqrt(b1 * b1 + b2 * b2);

      r /= div;
      g /= div;
      b /= div;

      r = clamp(r, 255);
      g = clamp(g, 255);
      b = clamp(b, 255);

      const int c = bmp->getpixel(x, y);

      *p = Blend::trans(c, makeRgba(r, g, b, geta(c)), 255 - amount * 2.55);
      p++;
    }

    if(Gui::updateProgress(y) < 0)
      return;
  }

  temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

  Gui::hideProgress();
}

void Sobel::close()
{
  Items::dialog->hide();
  Project::undo->push();
  apply();
}

void Sobel::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void Sobel::begin()
{
  Items::dialog->show();
}

void Sobel::init()
{
  int y1 = 8;

  Items::dialog = new DialogWindow(256, 0, "Sobel Edge Detection");
  Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount %", 0, 0, 100);
  Items::amount->value("100");
  Items::amount->center();
  y1 += 24 + 8;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

