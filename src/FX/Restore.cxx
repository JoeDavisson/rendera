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

#include "Restore.H"

namespace Restore::Items
{
  DialogWindow *dialog;
  Fl_Box *box;
  CheckBox *normalize;
  CheckBox *invert;
  CheckBox *preserve_lum;
  Fl_Button *ok;
  Fl_Button *cancel;
}

void Restore::apply()
{
  Bitmap *bmp = Project::bmp;

  double rr = 0;
  double gg = 0;
  double bb = 0;
  int count = 0;

  const bool keep_lum = Items::preserve_lum->value();

  // determine overall color cast
  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      const rgba_type rgba = getRgba(*p);

      rr += rgba.r;
      gg += rgba.g;
      bb += rgba.b;

      count++;
      p++;
    }
  }

  rr /= count;
  gg /= count;
  bb /= count;

  // adjustment factors
  const double ra = (256.0 / (256 - rr)) / std::sqrt(256.0 / (rr + 1));
  const double ga = (256.0 / (256 - gg)) / std::sqrt(256.0 / (gg + 1));
  const double ba = (256.0 / (256 - bb)) / std::sqrt(256.0 / (bb + 1));

  // begin restore
  Gui::showProgress(bmp->h);

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      const rgba_type rgba = getRgba(*p);
      int r = rgba.r;
      int g = rgba.g;
      int b = rgba.b;
      const int l = getl(*p);

      // apply adjustments
      r = 255 * pow((double)r / 255, ra);
      g = 255 * pow((double)g / 255, ga);
      b = 255 * pow((double)b / 255, ba);

      r = clamp(r, 255);
      g = clamp(g, 255);
      b = clamp(b, 255);

      if(keep_lum)
	*p = Blend::keepLum(makeRgba(r, g, b, rgba.a), l);
      else
	*p = makeRgba(r, g, b, rgba.a);

      p++;
    }

    if(Gui::updateProgress(y) < 0)
      return;
  }

  Gui::hideProgress();
}

void Restore::close()
{
  Items::dialog->hide();
  Project::undo->push();

  if(Items::normalize->value())
    Normalize::apply();
  if(Items::invert->value())
    Invert::apply();

  apply();

  if(Items::invert->value())
    Invert::apply();
}

void Restore::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void Restore::begin()
{
  Items::dialog->show();
}

void Restore::init()
{
  int y1 = 8;

  Items::dialog = new DialogWindow(384, 0, "Restore");
  Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 32, "Attempts to correct color fading.");
  Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
//    Items::box->labelsize(12);
  y1 += 32;
  Items::normalize = new CheckBox(Items::dialog, 0, y1, 16, 16, "Normalize First", 0);
  y1 += 16 + 8;
  Items::normalize->value(0);
  Items::normalize->center();
  Items::invert = new CheckBox(Items::dialog, 0, y1, 16, 16, "Invert First", 0);
  Items::invert->center();
  y1 += 16 + 8;
  Items::preserve_lum = new CheckBox(Items::dialog, 8, y1, 16, 16, "Preserve Luminosity", 0);
  y1 += 16 + 8;
  Items::preserve_lum->center();
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

