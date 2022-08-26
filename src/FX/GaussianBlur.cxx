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

#include "GaussianBlur.H"

namespace GaussianBlur::Items
{
  DialogWindow *dialog;
  InputInt *size;
  InputInt *blend;
  Fl_Choice *mode;
  Fl_Button *ok;
  Fl_Button *cancel;
}

void GaussianBlur::apply()
{
  Bitmap *bmp = Project::bmp;

  const int size = (atof(Items::size->value()) * 2) * 2;
  const int blend = 255 - atoi(Items::blend->value()) * 2.55;
  const int mode = Items::mode->value();

  // bell curve
  std::vector<int> kernel(size);
  int div = 0;
  const int b = size / 2;

  for(int x = 0; x < size; x++)
  {
    kernel[x] = 255 * std::exp(-((double)((x - b) * (x - b)) / ((b * b) / 2)));
    div += kernel[x];
  }

  Bitmap temp(bmp->cw, bmp->ch);
  Gui::showProgress(bmp->ch);

  // x direction
  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = temp.row[y - bmp->ct];

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int rr = 0;
      int gg = 0;
      int bb = 0;
      int aa = 0;

      for(int i = 0; i < size; i++) 
      {
	const int mul = kernel[i];
	rgba_type rgba = getRgba(bmp->getpixel(x - size / 2 + i, y));

	rr += Gamma::fix(rgba.r) * mul;
	gg += Gamma::fix(rgba.g) * mul;
	bb += Gamma::fix(rgba.b) * mul;
	aa += rgba.a * mul;
      }

      rr = Gamma::unfix(rr / div);
      gg = Gamma::unfix(gg / div);
      bb = Gamma::unfix(bb / div);
      aa /= div;

      *p = makeRgba(rr, gg, bb, aa);
      p++;
    }

    if(Gui::updateProgress(y - bmp->ct) < 0)
      break;
  }

  Gui::showProgress(bmp->ch);

  // y direction
  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int rr = 0;
      int gg = 0;
      int bb = 0;
      int aa = 0;

      for(int i = 0; i < size; i++) 
      {
	const int mul = kernel[i];
	rgba_type rgba = getRgba(temp.getpixel(x - bmp->cl,
                                               y - size / 2 + i - bmp->ct));

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

      if(mode == 1)
	*p = Blend::trans(*p, Blend::keepLum(c3, getl(*p)), blend);
      else if(mode == 2)
	*p = Blend::transAlpha(*p, c3, blend);
      else
	*p = Blend::trans(*p, c3, blend);

      p++;
    }

    if(Gui::updateProgress(y - bmp->ct) < 0)
      break;
  }

  Gui::hideProgress();
}

void GaussianBlur::close()
{
  Items::dialog->hide();
  Project::undo->push();
  apply();
}

void GaussianBlur::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void GaussianBlur::begin()
{
  Items::dialog->show();
}

void GaussianBlur::init()
{
  int y1 = 8;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(256, 0, "Gaussian Blur");
  Items::size = new InputInt(Items::dialog, 0, y1, 96, 24, "Size (1-100)", 0, 1, 100);
  y1 += 24 + 8;
  Items::size->value("1");
  Items::size->center();
  Items::blend = new InputInt(Items::dialog, 0, y1, 96, 24, "Blend %", 0, 0, 100);
  Items::blend->value("100");
  Items::blend->center();
  y1 += 24 + 8;
  Items::mode = new Fl_Choice(0, y1, 96, 24, "Mode:");
  Items::mode->labelsize(12);
  Items::mode->textsize(12);
  Items::mode->add("Normal");
  Items::mode->add("Color Only");
  Items::mode->add("Alpha Only");
  Items::mode->value(0);
  Items::mode->align(FL_ALIGN_LEFT);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 24 + 8;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

