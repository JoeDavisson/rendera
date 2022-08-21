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

#include "BoxFilters.H"

enum
{
  BOX_BLUR,
  GAUSSIAN_BLUR,
  SHARPEN,
  EDGE_DETECT,
  EMBOSS,
  EMBOSS_REVERSE
};

namespace
{
  void copyMatrix(const int src[3][3], int dest[3][3])
  {
    for(int j = 0; j < 3; j++) 
    {
      for(int i = 0; i < 3; i++) 
      {
	dest[i][j] = src[i][j];
      }
    }
  }
}

namespace BoxFilters::Items
{
  DialogWindow *dialog;
  Fl_Choice *mode;
  InputInt *amount;
  Fl_Button *ok;
  Fl_Button *cancel;
}

void BoxFilters::apply()
{
  Bitmap *bmp = Project::bmp;
  int amount = atoi(Items::amount->value());
  int mode = Items::mode->value();

  int div = 1;
  int matrix[3][3];

  switch(mode)
  {
    case BOX_BLUR:
      copyMatrix(FilterMatrix::blur, matrix);
      div = 9;
      break;
    case GAUSSIAN_BLUR:
      copyMatrix(FilterMatrix::gaussian, matrix);
      div = 16;
      break;
    case SHARPEN:
      copyMatrix(FilterMatrix::sharpen, matrix);
      div = 1;
      break;
    case EDGE_DETECT:
      copyMatrix(FilterMatrix::edge, matrix);
      div = 1;
      break;
    case EMBOSS:
      copyMatrix(FilterMatrix::emboss, matrix);
      div = 1;
      break;
    case EMBOSS_REVERSE:
      copyMatrix(FilterMatrix::emboss_reverse, matrix);
      div = 1;
      break;
  }

  Bitmap temp(bmp->cw, bmp->ch);
  Gui::showProgress(bmp->h);

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = temp.row[y - bmp->cl];

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int r = 0;
      int g = 0;
      int b = 0;

      for(int j = 0; j < 3; j++) 
      {
	for(int i = 0; i < 3; i++) 
	{
	  const rgba_type rgba = getRgba(bmp->getpixel(x + i - 1, y + j - 1));

	  r += rgba.r * matrix[i][j];
	  g += rgba.g * matrix[i][j];
	  b += rgba.b * matrix[i][j];
	}
      }

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

void BoxFilters::close()
{
  Items::dialog->hide();
  Project::undo->push();
  apply();
}

void BoxFilters::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void BoxFilters::begin()
{
  Items::dialog->show();
}

void BoxFilters::init()
{
  int y1 = 8;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(256, 0, "Box Filters");
  Items::mode = new Fl_Choice(0, y1, 128, 24, "Filter:");
  Items::mode->textsize(10);
  Items::mode->add("Box Blur");
  Items::mode->add("Gaussian Blur");
  Items::mode->add("Sharpen");
  Items::mode->add("Edge Detect");
  Items::mode->add("Emboss");
  Items::mode->add("Emboss (Inverse)");
  Items::mode->value(0);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 24 + 8;
  Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount %", 0, 0, 100);
  Items::amount->value("50");
  Items::amount->center();
  y1 += 24 + 8;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}
