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

#include "RotateHue.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Widget *preview;
    Widget *hue;
    Fl_Repeat_Button *inc_hue;
    Fl_Repeat_Button *dec_hue;
    CheckBox *preserve_lum;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void RotateHue::apply(Bitmap *dest, bool show_progress)
{
  const int hh = (((Items::hue->var + 180) % 360) * 6) * .712;
  const bool keep_lum = Items::preserve_lum->value();

  FX::drawPreview(Project::bmp, Items::preview->bitmap);

  if(show_progress)
    Gui::showProgress(dest->h);

  for(int y = dest->ct; y <= dest->cb; y++)
  {
    int *p = dest->row[y] + dest->cl;

    for(int x = dest->cl; x <= dest->cr; x++)
    {
      int c = *p;

      rgba_type rgba = getRgba(c);

      const int l = getl(c);
      int r = rgba.r;
      int g = rgba.g;
      int b = rgba.b;
      int h, s, v;

      Blend::rgbToHsv(r, g, b, &h, &s, &v);
      h += hh;
      h %= 1536;

      Blend::hsvToRgb(h, s, v, &r, &g, &b);
      c = makeRgba(r, g, b, rgba.a);

      if(keep_lum)
        *p = Blend::keepLum(c, l);
      else
        *p = c;

      p++;
    }

    if(show_progress)
      if(Gui::updateProgress(y) < 0)
        return;
  }

  Gui::hideProgress();
}

void RotateHue::begin()
{
  Items::hue->var = 180;
  FX::drawPreview(Project::bmp, Items::preview->bitmap);
  Items::preview->redraw();
  Items::hue->do_callback();
  Items::dialog->show();
}

void RotateHue::close()
{
  Items::dialog->hide();
  Project::undo->push();
  apply(Project::bmp, true);
}

void RotateHue::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void RotateHue::init()
{
  int y1 = 8;

  Items::dialog = new DialogWindow(424, 0, "Rotate Hue");
  Items::preview = new Widget(Items::dialog, 8, y1, 408, 408, 0, 1, 1, 0);
  y1 += 408 + 8;
  Items::dec_hue = new Fl_Repeat_Button(8, y1, 20, 24, "@<");
  Items::dec_hue->callback((Fl_Callback *)decHue);
  Items::hue = new Widget(Items::dialog, 8 + 20 + 4, y1, 360, 24, 0, 1, 24, (Fl_Callback *)setHue);
  Items::hue->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::hue->labelfont(FL_COURIER);
  Items::inc_hue = new Fl_Repeat_Button(8 + 16 + 4 + 360 + 8, y1, 20, 24, "@>");
  Items::inc_hue->callback((Fl_Callback *)incHue);
  y1 += 24 + 8 + 24;
  Items::preserve_lum = new CheckBox(Items::dialog, 0, y1, 16, 16, "Preserve Luminosity", (Fl_Callback *)setHue);
  Items::preserve_lum->center();
  y1 += 16 + 8;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

void RotateHue::setHue()
{
  int hx = Items::hue->var % 360;

  Items::hue->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR));

  for(int x = 0; x < 360; x++)
  {
    if(!(x % 60))
      Items::hue->bitmap->vline(8, x, 23, getFltkColor(FL_FOREGROUND_COLOR), 160);
    else if(!(x % 30))
      Items::hue->bitmap->vline(16, x, 23, getFltkColor(FL_FOREGROUND_COLOR), 160);
    else if(!(x % 15))
      Items::hue->bitmap->vline(20, x, 23, getFltkColor(FL_FOREGROUND_COLOR), 160);
  }

  Items::hue->bitmap->rect(0, 0, Items::hue->bitmap->w - 1, Items::hue->bitmap->h - 1, makeRgb(0, 0, 0), 0);
  Items::hue->bitmap->xorVline(0, hx, 23);
  Items::hue->redraw();

  char degree[16];

  Items::hue->copy_label("                ");
  sprintf(degree, "%d\xB0", (int)(hx - 180));

  Items::hue->copy_label(degree);
  apply(Items::preview->bitmap, false);
  Items::preview->redraw();
}

void RotateHue::incHue()
{
  Items::hue->var++;
  if(Items::hue->var > 359)
    Items::hue->var = 359;
  setHue();
}

void RotateHue::decHue()
{
  Items::hue->var--;
  if(Items::hue->var < 0)
    Items::hue->var = 0;
  setHue();
}

