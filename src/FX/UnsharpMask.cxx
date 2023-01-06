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

#include "UnsharpMask.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *radius;
    InputFloat *amount;
    InputInt *threshold;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void UnsharpMask::apply(Bitmap *bmp, int radius, double amount, int threshold)
{
  radius = (radius + 1) * 2 + 1;
  std::vector<int> kernel(radius);
  int div = 0;

  // bell curve
  const int b = radius / 2;

  for(int x = 0; x < radius; x++)
  {
    kernel[x] = 255 * std::exp(-((double)((x - b) * (x - b)) /
					 ((b * b) / 2)));
    div += kernel[x];
  }

  Bitmap temp(bmp->cw, bmp->ch);
  Gui::showProgress(bmp->h);

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

      for(int i = 0; i < radius; i++) 
      {
	rgba_type rgba = getRgba(bmp->getpixel(x - radius / 2 + i, y));
	rr += Gamma::fix(rgba.r) * kernel[i];
	gg += Gamma::fix(rgba.g) * kernel[i];
	bb += Gamma::fix(rgba.b) * kernel[i];
	aa += rgba.a * kernel[i];
      }

      rr /= div;
      gg /= div;
      bb /= div;
      aa /= div;

      rr = Gamma::unfix(rr);
      gg = Gamma::unfix(gg);
      bb = Gamma::unfix(bb);

      *p = makeRgba((int)rr, (int)gg, (int)bb, (int)aa);
      p++;
    }

    if(Gui::updateProgress(y) < 0)
      return;
  }

  Bitmap temp2(bmp->cw, bmp->ch);
  temp.blit(&temp2, 0, 0, 0, 0, temp.w, temp.h);

  Gui::showProgress(bmp->h);

  // y direction
  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = temp2.row[y - bmp->ct];

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int rr = 0;
      int gg = 0;
      int bb = 0;
      int aa = 0;

      for(int i = 0; i < radius; i++) 
      {
	rgba_type rgba = getRgba(temp.getpixel(x - bmp->cl,
					       y - radius / 2 + i - bmp->ct));
	rr += Gamma::fix(rgba.r) * kernel[i];
	gg += Gamma::fix(rgba.g) * kernel[i];
	bb += Gamma::fix(rgba.b) * kernel[i];
	aa += rgba.a * kernel[i];
      }

      rr /= div;
      gg /= div;
      bb /= div;
      aa /= div;

      rr = Gamma::unfix(rr);
      gg = Gamma::unfix(gg);
      bb = Gamma::unfix(bb);

      *p = makeRgba((int)rr, (int)gg, (int)bb, (int)aa);
      p++;
    }

    if(Gui::updateProgress(y) < 0)
      return;
  }

  // blend
  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *d = bmp->row[y] + bmp->cl;
    int *p = temp2.row[y - bmp->ct];

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {

      int a = getl(*p);
      int b = getl(*d);

      if(ExtraMath::abs(a - b) >= threshold)
      {
	int lum = a - (amount * (a - b)); 
	lum = clamp(lum, 255);
	*d = Blend::keepLum(*p, lum);
      }

      d++;
      p++;
    }
  }

  Gui::hideProgress();
}

void UnsharpMask::close()
{
  Items::dialog->hide();
  Project::undo->push();

  const int radius = atoi(Items::radius->value());
  const double amount = atof(Items::amount->value());
  const int threshold = atoi(Items::threshold->value());

  apply(Project::bmp, radius, amount, threshold);
}

void UnsharpMask::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void UnsharpMask::begin()
{
  Items::dialog->show();
}

void UnsharpMask::init()
{
  int y1 = 8;

  Items::dialog = new DialogWindow(256, 0, "Unsharp Mask");
  Items::radius = new InputInt(Items::dialog, 0, y1, 96, 24, "Radius (1-100)", 0, 1, 100);
  y1 += 24 + 8;
  Items::radius->value("1");
  Items::radius->center();
  Items::amount = new InputFloat(Items::dialog, 0, y1, 96, 24, "Amount (0-10)", 0, 0, 10);
  y1 += 24 + 8;
  Items::amount->value("1.5");
  Items::amount->center();
  Items::threshold = new InputInt(Items::dialog, 0, y1, 72, 24, "Threshold (0-255)", 0, 0, 255);
  y1 += 24 + 8;
  Items::threshold->value("0");
  Items::threshold->center();
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

