/*
Copyright (c) 2015 Joe Davisson.

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

#include <cmath>
#include <vector>

#include <FL/fl_draw.H>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Choice.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "CheckBox.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "FilterMatrix.H"
#include "Fractal.H"
#include "FX2.H"
#include "Gamma.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
#include "InputFloat.H"
#include "InputInt.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "ExtraMath.H"
#include "Quantize.H"
#include "Separator.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

namespace
{
  Bitmap *bmp;

  void pushUndo()
  {
    bmp = Project::bmp;
    Undo::push();
  }

  void drawPreview(Bitmap *src, Bitmap *dest)
  {
    if(src->cw >= src->ch)
    {
      float aspect = (float)(src->ch) / (src->cw);
      int height = (float)dest->cw * aspect;

      dest->clear(getFltkColor(FL_BACKGROUND2_COLOR));
      src->fastStretch(dest, src->cl, src->ct,
                      src->cw, src->ch,
                      0, (dest->h - height) / 2, dest->w, height, false);
      dest->rect(0, (dest->h - height) / 2, dest->w, ((dest->h - height) / 2) + height, makeRgb(0, 0, 0), 0);
    }
    else
    {
      float aspect = (float)(src->cw) / (src->ch);
      int width = (float)dest->ch * aspect;

      dest->clear(getFltkColor(FL_BACKGROUND2_COLOR));
      src->fastStretch(dest, src->cl, src->ct,
                      src->cw, src->ch,
                      (dest->w - width) / 2, 0, width, dest->w, false);
      dest->rect((dest->w - width) / 2, 0, ((dest->w - width) / 2) + width, dest->w, makeRgb(0, 0, 0), 0);
    }

    dest->rect(0, 0, dest->w - 1, dest->h - 1, makeRgb(0, 0, 0), 128);
  }
}

namespace Marble
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
    Fl_Button *ok;
    Fl_Button *cancel;

    Bitmap *temp;
    int old_marb_var;
    int old_turb_var;
    int old_blend_var;
    int old_threshold_var;
  }

  void apply(Bitmap *dest)
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

    switch(Items::mode->value())
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
    
    for(int y = 0; y < h; y++)
    {
      int *p = dest->row[y + dest->ct] + dest->cl;

      for(int x = 0; x < w; x++)
      {
        int mix = marble.getpixel(x, y) - threshold;

        if(mix < 0)
          mix = 0;

        if(Items::type->value() == 1)
          *p = current_blend(*p, color, (scaleVal(trans, 255 - mix) & 63) * 4);
        else
          *p = current_blend(*p, color, scaleVal(trans, 255 - mix));

        p++;
      }
    }
  }

  void update()
  {
    bmp->blit(Items::temp, bmp->cl, bmp->ct, 0, 0, bmp->cw, bmp->ch);
    apply(Items::temp);
    drawPreview(Items::temp, Items::preview->bitmap);
    Items::preview->redraw();
  }

  void setMarb()
  {
    if(Items::marb->var == Items::old_marb_var)
      return;

    update();
    Items::old_marb_var = Items::marb->var;
  }

  void setTurb()
  {
    if(Items::turb->var == Items::old_turb_var)
      return;

    update();
    Items::old_turb_var = Items::turb->var;
  }

  void setBlend()
  {
    if(Items::blend->var == Items::old_blend_var)
      return;

    update();
    Items::old_blend_var = Items::blend->var;
  }

  void setThreshold()
  {
    if(Items::threshold->var == Items::old_threshold_var)
      return;

    update();
    Items::old_threshold_var = Items::threshold->var;
  }

  void getColor()
  {
    Dialog::editor();
    Items::color->bitmap->clear(Project::brush->color);
    Items::color->bitmap->rect(0, 0, Items::color->bitmap->w - 1, Items::color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
    Items::color->redraw();

    update();
  }

  void close()
  {
    Items::temp->blit(bmp, 0, 0, bmp->cl, bmp->ct, Items::temp->w, Items::temp->h);
    Items::dialog->hide();
    Gui::getView()->drawMain(true);
    delete Items::temp;
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
    delete Items::temp;
  }

  void begin()
  {
    pushUndo();
    bmp = Project::bmp;
    Items::temp = new Bitmap(bmp->cw, bmp->ch);
    bmp->blit(Items::temp, bmp->cl, bmp->ct, 0, 0, bmp->cw, bmp->ch);
    apply(Items::temp);
    drawPreview(Items::temp, Items::preview->bitmap);
    Items::preview->redraw();
    Items::color->bitmap->clear(Project::brush->color);
    Items::color->bitmap->rect(0, 0, Items::color->bitmap->w - 1, Items::color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
    Items::color->redraw();
    Items::dialog->show();
  }

  void init()
  {
    int x1 = 8;
    int y1 = 8;

    Items::dialog = new DialogWindow(304 + 16, 0, "Marble");
    Items::preview = new Widget(Items::dialog, 8, y1, 304, 304, 0, 1, 1, (Fl_Callback *)update);
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
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace ForwardFFT
{
  void apply()
  {
    int w = bmp->cw;
    int h = bmp->ch;

    // resize image
    Project::resizeImage(w * 2, h);
    bmp = Project::bmp;

    std::vector<float> real(w * h, 0);
    std::vector<float> imag(w * h, 0);
    std::vector<float> real_row(w, 0);
    std::vector<float> imag_row(w, 0);
    std::vector<float> real_col(h, 0);
    std::vector<float> imag_col(h, 0);

    Gui::showProgress(3);

    for(int channel = 0; channel < 3; channel++)
    {
      // forward horizontal pass
      for(int y = 0; y < h; y++)
      {
        int *p = bmp->row[y + bmp->ct] + bmp->cl;
        for(int x = 0; x < w; x++, p++)
        {
          const rgba_type rgba = getRgba(*p);

          switch(channel)
          {
            case 0:
              real_row[x] = rgba.r;
              break;
            case 1:
              real_row[x] = rgba.g;
              break;
            case 2:
              real_row[x] = rgba.b;
              break;
          }

          imag_row[x] = 0;
        }

        ExtraMath::forwardFFT(&real_row[0], &imag_row[0], w);

        for(int x = 0; x < w; x++)
        {
          real[x + w * y] = real_row[x];
          imag[x + w * y] = imag_row[x];
        }
      }

      // forward vertical pass
      for(int x = 0; x < w; x++)
      {
        for(int y = 0; y < h; y++)
        {
          real_col[y] = real[x + w * y];
          imag_col[y] = imag[x + w * y];
        }

        ExtraMath::forwardFFT(&real_col[0], &imag_col[0], h);

        for(int y = 0; y < h; y++)
        {
          real[x + w * y] = real_col[y];
          imag[x + w * y] = imag_col[y];
        }
      }

      // convert to image
      for(int y = 0; y < h; y++)
      {
        for(int x = 0; x < w; x++)
        {
          float re = real[x + w * y];
          float im = imag[x + w * y];
          float mag = log10f(sqrtf(re * re + im * im)) * 32;
          float phase = (atan2f(im, re) + 3.14159f) * 32;
          int val1 = clamp((int)mag, 255);
          int val2 = clamp((int)phase, 255);

          int xx = (x + w / 2) % w;
          int yy = (y + h / 2) % h;
          xx += bmp->cl;
          yy += bmp->ct;
          const rgba_type rgba1 = getRgba(bmp->getpixel(xx, yy));
          const rgba_type rgba2 = getRgba(bmp->getpixel(xx + w, yy));

          switch(channel)
          {
            case 0:
              bmp->setpixel(xx, yy, makeRgb(val1, rgba1.g, rgba1.b));
              bmp->setpixel(xx + w, yy, makeRgb(val2, rgba2.g, rgba2.b));
              break;
            case 1:
              bmp->setpixel(xx, yy, makeRgb(rgba1.r, val1, rgba1.b));
              bmp->setpixel(xx + w, yy, makeRgb(rgba2.r, val2, rgba2.b));
              break;
            case 2:
              bmp->setpixel(xx, yy, makeRgb(rgba1.r, rgba1.g, val1));
              bmp->setpixel(xx + w, yy, makeRgb(rgba2.r, rgba2.g, val2));
              break;
          }
        }
      }

      if(Gui::updateProgress(channel) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

namespace InverseFFT
{
  void apply()
  {
    int w = bmp->cw / 2;
    int h = bmp->ch;

    std::vector<float> real(w * h, 0);
    std::vector<float> imag(w * h, 0);
    std::vector<float> real_row(w, 0);
    std::vector<float> imag_row(w, 0);
    std::vector<float> real_col(h, 0);
    std::vector<float> imag_col(h, 0);

    Gui::showProgress(3);

    for(int channel = 0; channel < 3; channel++)
    {
      // convert from image
      for(int y = 0; y < h; y++)
      {
        for(int x = 0; x < w; x++)
        {
          int xx = (x + w / 2) % w;
          int yy = (y + h / 2) % h;
          xx += bmp->cl;
          yy += bmp->ct;

          int p1 = bmp->getpixel(xx, yy);
          int p2 = bmp->getpixel(xx + w, yy);
          float c1 = 0, c2 = 0;

          switch(channel)
          {
            case 0:
              c1 = getr(p1);
              c2 = getr(p2);
              break;
            case 1:
              c1 = getg(p1);
              c2 = getg(p2);
              break;
            case 2:
              c1 = getb(p1);
              c2 = getb(p2);
              break;
          }

          float mag = powf(10.0f, c1 / 32);
          float phase = c2 / 32 - 3.14159f;

          real[x + w * y] = mag * cosf(phase);
          imag[x + w * y] = mag * sinf(phase);
        }
      }

      // inverse horizontal pass
      for(int y = 0; y < h; y++)
      {
        for(int x = 0; x < w; x++)
        {
          real_row[x] = real[x + w * y];
          imag_row[x] = imag[x + w * y];
        }

        ExtraMath::inverseFFT(&real_row[0], &imag_row[0], w);

        for(int x = 0; x < w; x++)
        {
          real[x + w * y] = real_row[x];
          imag[x + w * y] = imag_row[x];
        }
      }

      // inverse vertical pass
      for(int x = 0; x < w; x++)
      {
        for(int y = 0; y < h; y++)
        {
          real_col[y] = real[x + w * y];
          imag_col[y] = imag[x + w * y];
        }

        ExtraMath::inverseFFT(&real_col[0], &imag_col[0], h);

        for(int y = 0; y < h; y++)
        {
          real[x + w * y] = real_col[y];
          imag[x + w * y] = imag_col[y];
        }
      }

      // convert to image
      for(int y = 0; y < h; y++)
      {
        int *p = bmp->row[y + bmp->ct] + bmp->cl;
        for(int x = 0; x < w; x++, p++)
        {
          float re = real[x + w * y];
          int val = clamp((int)re, 255);

          const rgba_type rgba = getRgba(*p);

          switch(channel)
          {
            case 0:
              *p = makeRgb(val, rgba.g, rgba.b);
              break;
            case 1:
              *p = makeRgb(rgba.r, val, rgba.b);
              break;
            case 2:
              *p = makeRgb(rgba.r, rgba.g, val);
              break;
          }
        }
      }

      if(Gui::updateProgress(channel) < 0)
        return;
    }

    Gui::hideProgress();
    Project::resizeImage(w, h);
    bmp = Project::bmp;
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

void FX2::init()
{
  Marble::init();
}

void FX2::marble()
{
  Marble::begin();
}

void FX2::forwardFFT()
{
  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  if(ExtraMath::isPowerOfTwo(w) && ExtraMath::isPowerOfTwo(h))
  {
    ForwardFFT::begin();
  }
  else
  {
    Dialog::message("Error", "Image dimensions must\nbe powers of two.");
  }
}

void FX2::inverseFFT()
{
  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  if(ExtraMath::isPowerOfTwo(w) && ExtraMath::isPowerOfTwo(h))
  {
    InverseFFT::begin();
  }
  else
  {
    Dialog::message("Error", "Image dimensions must\nbe powers of two.");
  }
}

