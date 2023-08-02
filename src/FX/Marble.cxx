/*
Copyright (c) 2023 Joe Davisson.

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

#include "Marble.H"
#include "Fractal.H"
#include "Images.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Widget *preview;
    Widget *marb;
    Widget *turb;
    Widget *blend;
    Widget *threshold;
    Widget *color;
    Fl_Choice *type;
    Fl_Choice *mode;
    Fl_Button *palette_editor;
    Fl_Button *change;
    Fl_Button *ok;
    Fl_Button *cancel;

    Bitmap *temp;
    int old_marb_var;
    int old_turb_var;
    int old_blend_var;
    int old_threshold_var;
  }
}

void Marble::apply(Bitmap *dest)
{
  int w = dest->cw;
  int h = dest->ch;

  Map plasma(w, h);
  Map marble(w, h);
  Map marbx(w, h);
  Map marby(w, h);

  Fractal::plasma(&plasma, (Items::turb->var + 1) << 10);
  Fractal::plasma(&marbx, (Items::turb->var + 1) << 10);
  Fractal::plasma(&marby, (Items::turb->var + 1) << 10);
  Fractal::marble(&plasma, &marble, &marbx, &marby, (Items::marb->var + 1) << 2, 50, Items::type->value());

  int color = Project::brush->color;
  int trans = Items::blend->var * 10.96;
  int threshold = Items::threshold->var * 10.96;
  int (*current_blend)(const int, const int, const int) = &Blend::trans;

  switch (Items::mode->value())
  {
    case 1:
      current_blend = &Blend::lighten;
      break;
    case 2:
      current_blend = &Blend::darken;
      break;
    default:
      current_blend = &Blend::trans;
      break;
  }
  
  for (int y = 0; y < h; y++)
  {
    int *p = dest->row[y + dest->ct] + dest->cl;

    for (int x = 0; x < w; x++)
    {
      int mix = marble.getpixel(x, y) - threshold;

      if (mix < 0)
        mix = 0;

      if (Items::type->value() == 1)
        *p = current_blend(*p, color, (scaleVal(trans, 255 - mix) & 63) * 4);
      else
        *p = current_blend(*p, color, scaleVal(trans, 255 - mix));

      p++;
    }
  }
}

void Marble::close()
{
  Bitmap *bmp = Project::bmp;
  Items::temp->blit(bmp, 0, 0, bmp->cl, bmp->ct, Items::temp->w, Items::temp->h);
  Items::dialog->hide();
  Gui::getView()->drawMain(true);
  delete Items::temp;
}

void Marble::quit()
{
  Gui::progressHide();
  Items::dialog->hide();
  delete Items::temp;
}

void Marble::begin()
{
  Bitmap *bmp = Project::bmp;
  Project::undo->push();
  Items::temp = new Bitmap(bmp->cw, bmp->ch);
  bmp->blit(Items::temp, bmp->cl, bmp->ct, 0, 0, bmp->cw, bmp->ch);
  apply(Items::temp);
  FX::drawPreview(Items::temp, Items::preview->bitmap);
  Items::preview->redraw();
  Items::color->bitmap->clear(Project::brush->color);
  Items::color->bitmap->rect(0, 0, Items::color->bitmap->w - 1, Items::color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
  Items::color->redraw();
  Items::dialog->show();
}

void Marble::init()
{
  int x1 = 8;
  int y1 = 8;

  Items::dialog = new DialogWindow(304 + 16, 0, "Marble");
  Items::preview = new Widget(Items::dialog, 8, y1, 304, 304, 0, 1, 1, 0);
  y1 += 304 + 8;
  Items::marb = new Widget(Items::dialog, 8, y1, 144, 24, "Marbleize", images_marbleize_large_png, 12, 24, (Fl_Callback *)setMarb);
  Items::marb->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::marb->labelfont(FL_COURIER);
  Items::marb->labelsize(12);
  Items::turb = new Widget(Items::dialog, 8 + 144 + 16, y1, 144, 24, "Turbulence", images_turbulence_large_png, 12, 24, (Fl_Callback *)setTurb);
  Items::turb->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::turb->labelfont(FL_COURIER);
  Items::turb->labelsize(12);
  y1 += 24 + 16 + 8;
  Items::blend = new Widget(Items::dialog, 8, y1, 144, 24, "Blend", images_transparency_large_png, 6, 24, (Fl_Callback *)setBlend);
  Items::blend->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::blend->labelfont(FL_COURIER);
  Items::blend->labelsize(12);
  Items::threshold = new Widget(Items::dialog, 8 + 144 + 16, y1, 144, 24, "Threshold", images_transparency_large_png, 6, 24, (Fl_Callback *)setThreshold);
  Items::threshold->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::threshold->labelfont(FL_COURIER);
  Items::threshold->labelsize(12);
  y1 += 24 + 16 + 8;
  Items::type = new Fl_Choice(x1, y1, 96, 24, 0);
  Items::type->textsize(12);
  Items::type->labelsize(12);
  Items::type->tooltip("Type");
  Items::type->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::type->labelfont(FL_COURIER);
  Items::type->add("Marble");
  Items::type->add("Wood");
  Items::type->add("Malachite");
  Items::type->add("Corrosion");
  Items::type->add("Metal");
  Items::type->add("Weathered");
  Items::type->add("Immiscible");
  Items::type->value(0);
  Items::type->callback((Fl_Callback *)update);
  x1 += 96 + 8;
  Items::mode = new Fl_Choice(x1, y1, 96, 24, 0);
  Items::mode->textsize(12);
  Items::mode->labelsize(12);
  Items::mode->tooltip("Blending Mode");
  Items::mode->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  Items::mode->labelfont(FL_COURIER);
  Items::mode->add("Normal");
  Items::mode->add("Lighten");
  Items::mode->add("Darken");
  Items::mode->value(0);
  Items::mode->callback((Fl_Callback *)update);
  x1 += 96 + 8;
  Items::palette_editor = new Fl_Button(x1, y1, 64, 24, "Color...");
  Items::palette_editor->labelsize(12);
  Items::palette_editor->callback((Fl_Callback *)getColor);
  x1 += 64 + 8;
  Items::color = new Widget(Items::dialog, x1, y1, 24, 24, 0, 0, 0, 0);
  y1 += 24 + 8;
  Items::change = new Fl_Button(8, y1 + 8, 96, 24, "Randomize");
  Items::change->labelsize(12);
  Items::change->callback((Fl_Callback *)update);
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

void Marble::update()
{
  Bitmap *bmp = Project::bmp;
  bmp->blit(Items::temp, bmp->cl, bmp->ct, 0, 0, bmp->cw, bmp->ch);
  apply(Items::temp);
  FX::drawPreview(Items::temp, Items::preview->bitmap);
  Items::preview->redraw();
}

void Marble::setMarb()
{
  if (Items::marb->var == Items::old_marb_var)
    return;

  update();
  Items::old_marb_var = Items::marb->var;
}

void Marble::setTurb()
{
  if (Items::turb->var == Items::old_turb_var)
    return;

  update();
  Items::old_turb_var = Items::turb->var;
}

void Marble::setBlend()
{
  if (Items::blend->var == Items::old_blend_var)
    return;

  update();
  Items::old_blend_var = Items::blend->var;
}

void Marble::setThreshold()
{
  if (Items::threshold->var == Items::old_threshold_var)
    return;

  update();
  Items::old_threshold_var = Items::threshold->var;
}

void Marble::getColor()
{
  Editor::begin();
  Items::color->bitmap->clear(Project::brush->color);
  Items::color->bitmap->rect(0, 0, Items::color->bitmap->w - 1, Items::color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
  Items::color->redraw();

  update();
}

