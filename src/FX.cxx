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

#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "CheckBox.H"
#include "FilterMatrix.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "FX.H"
#include "Gamma.H"
#include "Gui.H"
#include "Inline.H"
#include "InputFloat.H"
#include "InputInt.H"
#include "Map.H"
#include "Math.H"
#include "Palette.H"
#include "Project.H"
#include "Separator.H"
#include "Undo.H"
#include "View.H"

namespace
{
  Bitmap *bmp;

  void pushUndo()
  {
    bmp = Project::bmp;
    Undo::push(bmp->cl, bmp->ct, bmp->cw, bmp->ch);
  }
}

namespace Normalize
{
  void apply()
  {
    // search for highest & lowest RGB values
    int r_high = 0;
    int g_high = 0;
    int b_high = 0;
    int r_low = 0xFFFFFF;
    int g_low = 0xFFFFFF;
    int b_low = 0xFFFFFF;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        if(r < r_low)
          r_low = r;
        if(r > r_high)
          r_high = r;
        if(g < g_low)
          g_low = g;
        if(g > g_high)
          g_high = g;
        if(b < b_low)
          b_low = b;
        if(b > b_high)
          b_high = b;
      }
    }

    if(!(r_high - r_low))
      r_high++;
    if(!(g_high - g_low))
      g_high++;
    if(!(b_high - b_low))
      b_high++;

    // scale image
    double r_scale = 255.0 / (r_high - r_low);
    double g_scale = 255.0 / (g_high - g_low);
    double b_scale = 255.0 / (b_high - b_low);

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        const int r = (rgba.r - r_low) * r_scale;
        const int g = (rgba.g - g_low) * g_scale;
        const int b = (rgba.b - b_low) * b_scale;

        *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(Gui::updateProgress(y) < 0)
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

namespace Equalize
{
  void apply()
  {
    std::vector<int> list_r(256, 0);
    std::vector<int> list_g(256, 0);
    std::vector<int> list_b(256, 0);

    const int size = bmp->cw * bmp->ch;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        list_r[r]++;
        list_g[g]++;
        list_b[b]++;
      }
    }

    for(int j = 255; j >= 0; j--)
    {
      for(int i = 0; i < j; i++)
      {
        list_r[j] += list_r[i];
        list_g[j] += list_g[i];
        list_b[j] += list_b[i];
      }
    }

    const double scale = 255.0 / size;

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        r = list_r[r] * scale;
        g = list_g[g] * scale;
        b = list_b[b] * scale;

        *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(Gui::updateProgress(y) < 0)
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

namespace ValueStretch
{
  void apply()
  {
    std::vector<int> list_r(256, 0);
    std::vector<int> list_g(256, 0);
    std::vector<int> list_b(256, 0);

    double rr = 0;
    double gg = 0;
    double bb = 0;
    int count = 0;

    // determine overall color cast
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        rr += rgba.r;
        gg += rgba.g;
        bb += rgba.b;

        count++;
      }
    }

    rr /= count;
    gg /= count;
    bb /= count;

    const int size = bmp->cw * bmp->ch;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        list_r[r]++;
        list_g[g]++;
        list_b[b]++;
      }
    }

    for(int j = 255; j >= 0; j--)
    {
      for(int i = 0; i < j; i++)
      {
        list_r[j] += list_r[i];
        list_g[j] += list_g[i];
        list_b[j] += list_b[i];
      }
    }

    double scale = 255.0 / size;

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        const int ra = list_r[r] * scale;
        const int ga = list_g[g] * scale;
        const int ba = list_b[b] * scale;

        r = ((ra * rr) + (r * (255 - rr))) / 255;
        g = ((ga * gg) + (g * (255 - gg))) / 255;
        b = ((ba * bb) + (b * (255 - bb))) / 255;

        r = std::min(std::max(r, 0), 255);
        g = std::min(std::max(g, 0), 255);
        b = std::min(std::max(b, 0), 255);

        *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(Gui::updateProgress(y) < 0)
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

namespace Saturate
{
  void apply()
  {
    std::vector<int> list_s(256, 0);

    const int size = bmp->cw * bmp->ch;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        int h, s, v;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);

        list_s[s]++;
      }
    }

    for(int j = 255; j >= 0; j--)
      for(int i = 0; i < j; i++)
        list_s[j] += list_s[i];

    const double scale = 255.0 / size;

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = bmp->getpixel(x, y);

        rgba_type rgba = getRgba(c);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        const int l = getl(c);
        int h, s, v;

        Blend::rgbToHsv(r, g, b, &h, &s, &v);

        // don't try to saturate grays
        if(s == 0)
        {
          p++;
          continue;
        }

        const int temp = s;
        s = list_s[s] * scale;

        if(s < temp)
          s = temp;

        Blend::hsvToRgb(h, s, v, &r, &g, &b);
        *p = Blend::trans(*p, Blend::keepLum(makeRgba(r, g, b, rgba.a), l), 255 - s);
        p++;
      }

      if(Gui::updateProgress(y) < 0)
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

namespace RotateHue
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *angle;
    CheckBox *preserve;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int amount)
  {
    const int hh = amount * 4.277;
    const bool keep_lum = Items::preserve->value();

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int c = bmp->getpixel(x, y);

        rgba_type rgba = getRgba(c);

        const int l = getl(c);
        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;
        int h, s, v;

        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        h += hh;

        if(h >= 1536)
          h -= 1536;

        Blend::hsvToRgb(h, s, v, &r, &g, &b);
        c = makeRgba(r, g, b, rgba.a);

        if(keep_lum)
          *p++ = Blend::keepLum(c, l);
        else
          *p++ = c;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::angle->limitValue(-359, 359) < 0)
      return;

    Items::dialog->hide();
    pushUndo();

    int angle = atoi(Items::angle->value());

    if(angle < 0)
      angle += 360;

    apply(angle);
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Rotate Hue");
    Items::angle = new InputInt(Items::dialog, 0, y1, 72, 24, "Angle:", 0);
    y1 += 24 + 8;
    Items::angle->maximum_size(4);
    Items::angle->value("60");
    Items::angle->center();
    Items::preserve = new CheckBox(Items::dialog, 0, y1, 16, 16, "Preserve Luminance", 0);
    Items::preserve->center();
    y1 += 16 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Invert
{
  void apply()
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int c = bmp->getpixel(x, y);

        *p++ = Blend::invert(c, 0, 0);
      }

      if(Gui::updateProgress(y) < 0)
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

namespace InvertAlpha
{
  void apply()
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const rgba_type rgba = getRgba(bmp->getpixel(x, y));
        *p++ = makeRgba(rgba.r, rgba.g, rgba.b, 255 - rgba.a);
      }

      if(Gui::updateProgress(y) < 0)
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

// Corrects uneven dye fading in photographs (especially when there is a severe
// color cast, such as when one of the dyes have faded almost completely).
//
// Sometimes works better before or after the restore filter depending on
// the image.
namespace CorrectionMatrix
{
  void apply()
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = bmp->getpixel(x, y);

        rgba_type rgba = getRgba(c);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;
        int l = getl(c);

        // correction matrix
        int ra = r;
        int ga = (r * 4 + g * 8 + b * 1) / 13;
        int ba = (r * 2 + g * 4 + b * 8) / 14;

        ra = std::max(std::min(ra, 255), 0);
        ga = std::max(std::min(ga, 255), 0);
        ba = std::max(std::min(ba, 255), 0);

        *p++ = Blend::keepLum(makeRgba(ra, ga, ba, rgba.a), l);
      }

      if(Gui::updateProgress(y) < 0)
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

// This algorithm assumes a picture both has a color cast and is faded.
// In most cases (with a good scan) it can restore nearly all the color
// to a faded photograph.
namespace Restore
{
  namespace Items
  {
    DialogWindow *dialog;
    CheckBox *normalize;
    CheckBox *invert;
    CheckBox *correct;
    CheckBox *color_only;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply()
  {
    float rr = 0;
    float gg = 0;
    float bb = 0;
    int count = 0;

    const bool keep_lum = Items::color_only->value();

    // determine overall color cast
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const rgba_type rgba = getRgba(bmp->getpixel(x, y));

        rr += rgba.r;
        gg += rgba.g;
        bb += rgba.b;

        count++;
      }
    }

    rr /= count;
    gg /= count;
    bb /= count;

    // adjustment factors
    const float ra = (256.0f / (256 - rr)) / std::sqrt(256.0f / (rr + 1));
    const float ga = (256.0f / (256 - gg)) / std::sqrt(256.0f / (gg + 1));
    const float ba = (256.0f / (256 - bb)) / std::sqrt(256.0f / (bb + 1));

    // begin restore
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = bmp->getpixel(x, y);
        const rgba_type rgba = getRgba(c);
        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;
        const int l = getl(c);

        // apply adjustments
        r = 255 * powf((float)r / 255, ra);
        g = 255 * powf((float)g / 255, ga);
        b = 255 * powf((float)b / 255, ba);

        r = std::max(std::min(r, 255), 0);
        g = std::max(std::min(g, 255), 0);
        b = std::max(std::min(b, 255), 0);

        if(keep_lum)
          *p++ = Blend::keepLum(makeRgba(r, g, b, rgba.a), l);
        else
          *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();

    if(Items::normalize->value())
      Normalize::apply();
    if(Items::invert->value())
      Invert::apply();

    apply();

    if(Items::invert->value())
      Invert::apply();
    if(Items::correct->value())
      CorrectionMatrix::apply();
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Restore");
    Items::normalize = new CheckBox(Items::dialog, 0, y1, 16, 16, "Normalize First", 0);
    y1 += 16 + 8;
    Items::normalize->value(1);
    Items::normalize->center();
    Items::invert = new CheckBox(Items::dialog, 0, y1, 16, 16, "Invert First", 0);
    y1 += 16 + 8;
    Items::invert->center();
    Items::correct = new CheckBox(Items::dialog, 8, y1, 16, 16, "Use Correction Matrix", 0);
    y1 += 16 + 8;
    Items::correct->center();
    Items::color_only = new CheckBox(Items::dialog, 8, y1, 16, 16, "Affect Color Only", 0);
    y1 += 16 + 8;
    Items::color_only->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace RemoveDust
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *amount;
    CheckBox *invert;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int amount)
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct + 1; y <= bmp->cb - 1; y++)
    {
      int *p = bmp->row[y] + bmp->cl + 1;

      for(int x = bmp->cl + 1; x <= bmp->cr - 1; x++)
      {
        const int test = bmp->getpixel(x, y);
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

        for(int i = 0; i < 8; i++)
        {
          rgba_type rgba = getRgba(c[i]);
          r += rgba.r;
          g += rgba.g;
          b += rgba.b;
        }

        const int avg = makeRgba(r / 8, g / 8, b / 8, geta(test));

        if((getl(avg) - getl(test)) > amount)
          *p = avg;

        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::amount->limitValue(1, 10) < 0)
      return;

    Items::dialog->hide();
    pushUndo();

    if(Items::invert->value())
      Invert::apply();

    apply(atoi(Items::amount->value()));

    if(Items::invert->value())
      Invert::apply();
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Remove Dust");
    Items::amount = new InputInt(Items::dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    Items::amount->value("4");
    Items::amount->center();
    Items::invert = new CheckBox(Items::dialog, 0, y1, 16, 16, "Invert First", 0);
    y1 += 16 + 8;
    Items::invert->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Desaturate
{
  void apply()
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = bmp->getpixel(x, y);
        const int l = getl(c);

        *p++ = makeRgba(l, l, l, geta(c));
      }

      if(Gui::updateProgress(y) < 0)
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

namespace Colorize
{
  void apply()
  {
    rgba_type rgba_color = getRgba(Project::brush->color);

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int c1 = bmp->getpixel(x, y);

        rgba_type rgba = getRgba(c1);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;
        int h, s, v;

        Blend::rgbToHsv(r, g, b, &h, &s, &v);

        int sat = s;

        if(sat < 64)
          sat = 64;

        r = rgba_color.r;
        g = rgba_color.g;
        b = rgba_color.b;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        Blend::hsvToRgb(h, (sat * s) / (sat + s), v, &r, &g, &b);

        *p++ = Blend::colorize(c1, makeRgba(r, g, b, rgba.a), 0);
      }

      if(Gui::updateProgress(y) < 0)
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

namespace ApplyPalette
{
  namespace Items
  {
    DialogWindow *dialog;
    CheckBox *dither;
    CheckBox *gamma;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void applyNormal()
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = (Project::palette->data[(int)Project::palette->lookup(*p)] & 0xFFFFFF) | (geta(*p) << 24);

        *p++ = c;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void applyDither()
  {
/*
    // floyd
    int matrix[2][3] =
    {
      { 0, 0, 7 },
      { 3, 5, 1 },
    };

    const int w = 3;
    const int h = 2;
    const int div = 16;
*/
/*
    // jarvis
    int matrix[3][5] =
    {
      { 0, 0, 0, 7, 5 },
      { 3, 5, 7, 5, 3 },
      { 1, 3, 5, 3, 1 }
    };

    const int w = 5;
    const int h = 3;
    const int div = 48;
*/
/*
    // stucki
    int matrix[3][5] =
    {
      { 0, 0, 0, 8, 4 },
      { 2, 4, 8, 4, 2 },
      { 1, 2, 4, 2, 1 }
    };

    const int w = 5;
    const int h = 3;
    const int div = 42;
*/

    // atkinson
    const int matrix[3][5] =
    {
      { 0, 0, 0, 1, 1 },
      { 0, 1, 1, 1, 0 },
      { 0, 0, 1, 0, 0 }
    };

    const int w = 5;
    const int h = 3;
    const int div = 8;

    Bitmap *bmp = Project::bmp;
    const bool fix_gamma = Items::gamma->value();

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);
        const int alpha = rgba.a;
        const int old_r = rgba.r;
        const int old_g = rgba.g;
        const int old_b = rgba.b;

        const int pal_index = (int)Project::palette->lookup(*p);
        const int c = Project::palette->data[pal_index];

        rgba = getRgba(c);
        *p++ = makeRgba(rgba.r, rgba.g, rgba.b, alpha);

        const int new_r = rgba.r;
        const int new_g = rgba.g;
        const int new_b = rgba.b;
        int er, eg, eb;

        if(fix_gamma)
        {
          er = Gamma::fix(old_r) - Gamma::fix(new_r);
          eg = Gamma::fix(old_g) - Gamma::fix(new_g);
          eb = Gamma::fix(old_b) - Gamma::fix(new_b);
        }
        else
        {
          er = old_r - new_r;
          eg = old_g - new_g;
          eb = old_b - new_b;
        }

        for(int j = 0; j < h; j++)
        {
          for(int i = 0; i < w; i++)
          {
            if(matrix[j][i] > 0)
            {
              rgba_type rgba = getRgba(bmp->getpixel(x - w / 2 + i, y + j));
              int r, g, b;

              if(fix_gamma)
              {
                r = Gamma::fix(rgba.r); 
                g = Gamma::fix(rgba.g); 
                b = Gamma::fix(rgba.b);
              }
              else
              {
                r = rgba.r; 
                g = rgba.g; 
                b = rgba.b; 
              }

              r += (er * matrix[j][i]) / div;
              g += (eg * matrix[j][i]) / div;
              b += (eb * matrix[j][i]) / div;

              if(fix_gamma)
              {
                r = Gamma::unfix(std::max(std::min(r, 65535), 0));
                g = Gamma::unfix(std::max(std::min(g, 65535), 0));
                b = Gamma::unfix(std::max(std::min(b, 65535), 0));
              }
              else
              {
                r = std::max(std::min(r, 255), 0);
                g = std::max(std::min(g, 255), 0);
                b = std::max(std::min(b, 255), 0);
              }

              bmp->setpixelSolid(x - w / 2 + i, y + j,
                                 makeRgba(r, g, b, rgba.a), 0);
            }  
          }
        }
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();

    if(Items::dither->value())
      applyDither();
    else
      applyNormal();
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void dither_callback()
  {
    if(Items::dither->value())
      Items::gamma->activate();
    else
      Items::gamma->deactivate();

    Items::dialog->redraw();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Apply Palette");
    Items::dither = new CheckBox(Items::dialog, 0, y1, 16, 16, "Dithering", 0);
    Items::dither->callback((Fl_Callback *)dither_callback);
    Items::dither->center();
    y1 += 16 + 8;
    Items::gamma = new CheckBox(Items::dialog, 0, y1, 16, 16, "Gamma Correction", 0);
    Items::gamma->deactivate();
    Items::gamma->center();
    y1 += 16 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace StainedGlass
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *detail;
    InputInt *edge;
    CheckBox *uniform;
    CheckBox *sat_alpha;
    CheckBox *draw_edges;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  inline int isEdge(Bitmap *b, const int &x, const int &y,
                           const int &div)
  {
    const int c0 = getl(b->getpixel(x, y)) / div;
    const int c1 = getl(b->getpixel(x + 1, y)) / div;
    const int c2 = getl(b->getpixel(x, y + 1)) / div;
    const int c3 = getl(b->getpixel(x + 1, y + 1)) / div;

    if((c0 == c1) && (c0 == c2) && (c0 == c3))
      return 0;
    else
      return 1;
  }

  inline int isSegmentEdge(Bitmap *b, const int &x, const int &y)
  {
    const int c0 = b->getpixel(x, y);
    const int c1 = b->getpixel(x + 1, y);
    const int c2 = b->getpixel(x, y + 1);
    const int c3 = b->getpixel(x + 1, y + 1);

    if((c0 == c1) && (c0 == c2) && (c0 == c3))
      return 0;
    else
      return 1;
  }

  void apply(int size, int div)
  {
    std::vector<int> seedx(size);
    std::vector<int> seedy(size);
    std::vector<int> color(size);

    for(int i = 0; i < size; i++)
    {
      if(Items::uniform->value())
      {
        seedx[i] = Math::rnd() % bmp->w; 
        seedy[i] = Math::rnd() % bmp->h; 
      }
      else
      {
        seedx[i] = Math::rnd() % bmp->w; 
        seedy[i] = Math::rnd() % bmp->h; 
  
        int count = 0;

        do
        {
          seedx[i] = Math::rnd() % bmp->w; 
          seedy[i] = Math::rnd() % bmp->h; 
          count++;
        }
        while(!isEdge(bmp, seedx[i], seedy[i], div) && count < 10000);
      }

      color[i] = bmp->getpixel(seedx[i], seedy[i]);
    }

    Gui::showProgress(bmp->h);

    // draw segments
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        // find nearest color
        int nearest = 999999999;
        int use = -1;

        for(int i = 0; i < size; i++)
        {
          const int dx = x - seedx[i];
          const int dy = y - seedy[i];
          const int distance = dx * dx + dy * dy;

          if(distance < nearest)
          {
            nearest = distance;
            use = i;
          }
        }

        if(use != -1)
        {
          if(Items::sat_alpha->value())
          {
            rgba_type rgba = getRgba(color[use]);

            int h, s, v;

            Blend::rgbToHsv(rgba.r, rgba.g, rgba.b, &h, &s, &v);
            *p = makeRgba(rgba.r, rgba.g, rgba.b, std::min(192, s / 2 + 128));
          }
          else
          {
            *p = color[use];
          }
        }

        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    // draw edges
    if(Items::draw_edges->value())
    {
      Map *map = Project::map;
      map->clear(0);

      for(int y = bmp->ct * 4; y <= bmp->cb * 4; y++)
      {
        for(int x = bmp->cl * 4; x <= bmp->cr * 4; x++)
        {
          if(isSegmentEdge(bmp, x / 4, y / 4))
            map->setpixelAA(x, y, 255);
        }
      }

      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        int *p = bmp->row[y] + bmp->cl;

        for(int x = bmp->cl; x <= bmp->cr; x++)
        {
          const int c = map->getpixel(x, y);

          *p++ = Blend::trans(bmp->getpixel(x, y), makeRgb(0, 0, 0), 255 - c);
        }
      }
    }

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::detail->limitValue(1, 50000) < 0)
      return;

    if(Items::edge->limitValue(1, 50) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::detail->value()), atoi(Items::edge->value()));
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Stained Glass");
    Items::detail = new InputInt(Items::dialog, 0, y1, 72, 24, "Detail:", 0);
    y1 += 24 + 8;
    Items::detail->value("5000");
    Items::detail->center();
    Items::edge = new InputInt(Items::dialog, 0, y1, 72, 24, "Edge Detect:", 0);
    y1 += 24 + 8;
    Items::edge->value("16");
    Items::edge->center();
    Items::uniform = new CheckBox(Items::dialog, 0, y1, 16, 16, "Uniform", 0);
    Items::uniform->center();
    y1 += 16 + 8;
    Items::sat_alpha = new CheckBox(Items::dialog, 0, y1, 16, 16, "Saturation to Alpha", 0);
    Items::sat_alpha->center();
    y1 += 16 + 8;
    Items::draw_edges = new CheckBox(Items::dialog, 0, y1, 16, 16, "Draw Edges", 0);
    Items::draw_edges->center();
    y1 += 16 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Blur
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *radius;
    InputInt *blend;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int radius, int blend)
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

        *p++ = makeRgba((int)rr, (int)gg, (int)bb, (int)aa);
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::showProgress(bmp->h);

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

        *p = Blend::trans(*p, makeRgba((int)rr, (int)gg, (int)bb, (int)aa), blend);
        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::radius->limitValue(1, 100) < 0)
      return;

    if(Items::blend->limitValue(0, 100) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::radius->value()), atoi(Items::blend->value()) * 2.55);
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Blur");
    Items::radius = new InputInt(Items::dialog, 0, y1, 72, 24, "Radius:", 0);
    y1 += 24 + 8;
    Items::radius->value("1");
    Items::radius->center();
    Items::blend = new InputInt(Items::dialog, 0, y1, 72, 24, "Blend:", 0);
    y1 += 24 + 8;
    Items::blend->value("0");
    Items::blend->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Sharpen
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *amount;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int amount)
  {
    Bitmap temp(bmp->cw, bmp->ch);
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = temp.row[y - bmp->cl];

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int lum = 0;

        for(int j = 0; j < 3; j++) 
        {
          for(int i = 0; i < 3; i++) 
          {
            lum += getl(bmp->getpixel(x + i - 1, y + j - 1))
                     * FilterMatrix::sharpen[i][j];
          }
        }

        const int c = bmp->getpixel(x, y);

        lum = std::min(std::max(lum, 0), 255);
        *p++ = Blend::trans(c, Blend::keepLum(c, lum), 255 - amount * 2.55);
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::amount->limitValue(1, 100) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::amount->value()));
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Sharpen");
    Items::amount = new InputInt(Items::dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    Items::amount->value("10");
    Items::amount->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace UnsharpMask
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

  void apply(int radius, double amount, int threshold)
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

        *p++ = makeRgba((int)rr, (int)gg, (int)bb, (int)aa);
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

        *p++ = makeRgba((int)rr, (int)gg, (int)bb, (int)aa);
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

        if(std::abs(a - b) >= threshold)
        {
          int lum = a - (amount * (a - b)); 
          lum = std::min(std::max(lum, 0), 255);
          *d = Blend::keepLum(*p, lum);
        }

        d++;
        p++;
      }
    }

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::radius->limitValue(1, 100) < 0)
      return;

    if(Items::amount->limitValue(0.1, 10.0) < 0)
      return;

    if(Items::threshold->limitValue(0, 255) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::radius->value()),
          atof(Items::amount->value()),
          atoi(Items::threshold->value()));
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Unsharp Mask");
    Items::radius = new InputInt(Items::dialog, 0, y1, 72, 24, "Radius:", 0);
    y1 += 24 + 8;
    Items::radius->value("1");
    Items::radius->center();
    Items::amount = new InputFloat(Items::dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    Items::amount->value("1.5");
    Items::amount->center();
    Items::threshold = new InputInt(Items::dialog, 0, y1, 72, 24, "Threshold:", 0);
    y1 += 24 + 8;
    Items::threshold->value("0");
    Items::threshold->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace ConvolutionMatrix
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *mode;
    InputInt *amount;
    CheckBox *lum_only;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

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

  enum
  {
    BOX_BLUR,
    GAUSSIAN_BLUR,
    SHARPEN,
    EDGE_DETECT,
    EMBOSS
  };
 
  void apply(int amount, int mode, bool lum_only)
  {
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
    }

    Bitmap temp(bmp->cw, bmp->ch);
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = temp.row[y - bmp->cl];

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int lum = 0;
        int r = 0;
        int g = 0;
        int b = 0;

        if(lum_only)
        {
          for(int j = 0; j < 3; j++) 
          {
            for(int i = 0; i < 3; i++) 
            {
              lum += getl(bmp->getpixel(x + i - 1, y + j - 1)) * matrix[i][j];
            }
          }

          lum /= div;
          lum = std::min(std::max(lum, 0), 255);

          const int c = bmp->getpixel(x, y);

          *p++ = Blend::trans(c, Blend::keepLum(c, lum), 255 - amount * 2.55);
        }
        else
        {
          for(int j = 0; j < 3; j++) 
          {
            for(int i = 0; i < 3; i++) 
            {
              const rgba_type rgba = getRgba(bmp->getpixel(x + i - 1,
                                                           y + j - 1));
              r += rgba.r * matrix[i][j];
              g += rgba.g * matrix[i][j];
              b += rgba.b * matrix[i][j];
            }
          }

          r /= div;
          g /= div;
          b /= div;

          r = std::min(std::max(r, 0), 255);
          g = std::min(std::max(g, 0), 255);
          b = std::min(std::max(b, 0), 255);

          const int c = bmp->getpixel(x, y);

          *p++ = Blend::trans(c, makeRgba(r, g, b, geta(c)), 255 - amount * 2.55);
        }
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::amount->limitValue(1, 100) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::amount->value()),
               Items::mode->value(),
               Items::lum_only->value());
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Convolution Matrix");
    Items::mode = new Fl_Choice(96, y1, 128, 24, "Presets:");
    Items::mode->tooltip("Presets");
    Items::mode->textsize(10);
    Items::mode->add("Box Blur");
    Items::mode->add("Gaussian Blur");
    Items::mode->add("Sharpen");
    Items::mode->add("Edge Detect");
    Items::mode->add("Emboss");
    Items::mode->value(0);
    y1 += 24 + 8;
    Items::amount = new InputInt(Items::dialog, 0, y1, 72, 24, "Amount:", 0);
    Items::amount->value("100");
    Items::amount->center();
    y1 += 24 + 8;
    Items::lum_only = new CheckBox(Items::dialog, 0, y1, 16, 16, "Luminance Only", 0);
    Items::lum_only->center();
    y1 += 16 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Artistic
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *amount;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int amount)
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int r = 0;
        int g = 0;
        int b = 0;
        int count = 0;

        for(int v = -amount; v <= amount; v++) 
        {
          for(int u = -amount; u <= amount; u++) 
          {
            const int c3 = *p;
            const int c1 = bmp->getpixel(x + u, y + v);
            const int c2 = bmp->getpixel(x - u, y - v);

            if(diff24(c3, c1) < diff24(c3, c2))
            {
              r += getr(c1);
              g += getg(c1);
              b += getb(c1);
            }
            else
            {
              r += getr(c2);
              g += getg(c2);
              b += getb(c2);
            }

            count++;
          }
        }

        r /= count;
        g /= count;
        b /= count;

        *p = makeRgba(r, g, b, geta(*p));
        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    if(Items::amount->limitValue(1, 10) < 0)
      return;

    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::amount->value()));
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    Items::dialog->show();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Artistic");
    Items::amount = new InputInt(Items::dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    Items::amount->value("3");
    Items::amount->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Descreen
{
  void apply()
  {
    int w = bmp->cw;
    int h = bmp->ch;

    w--;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    w++;

    h--;
    h |= h >> 1;
    h |= h >> 2;
    h |= h >> 4;
    h |= h >> 8;
    h |= h >> 16;
    h++;

    int size = w > h ? w : h;

    std::vector<float> real(size * size, 0);
    std::vector<float> imag(size * size, 0);
    std::vector<float> real_line(size, 0);
    std::vector<float> imag_line(size, 0);

    // horizontal pass
    for(int y = 0; y < size; y++)
    {
      for(int x = 0; x < size; x++)
      {
        real_line[x] = getv(bmp->getpixel(x, y));
        imag_line[x] = 0;
      }

      Math::forwardFFT(&real_line[0], &imag_line[0], size);

      for(int x = 0; x < size; x++)
      {
        real[x + w * y] = real_line[x];
        imag[x + w * y] = imag_line[x];
      }
    }

    // vertical pass
    for(int x = 0; x < size; x++)
    {
      for(int y = 0; y < size; y++)
      {
        real_line[y] = real[x + w * y];
        imag_line[y] = imag[x + w * y];
      }

      Math::forwardFFT(&real_line[0], &imag_line[0], size);

      for(int y = 0; y < size; y++)
      {
        real[x + w * y] = real_line[y];
        imag[x + w * y] = imag_line[y];
      }
    }
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

void FX::init()
{
  RotateHue::init();
  Restore::init();
  RemoveDust::init();
  ApplyPalette::init();
  StainedGlass::init();
  Blur::init();
  Sharpen::init();
  UnsharpMask::init();
  ConvolutionMatrix::init();
  Artistic::init();
}

void FX::normalize()
{
  Normalize::begin();
}

void FX::equalize()
{
  Equalize::begin();
}

void FX::valueStretch()
{
  ValueStretch::begin();
}

void FX::saturate()
{
  Saturate::begin();
}

void FX::rotateHue()
{
  RotateHue::begin();
}

void FX::invert()
{
  Invert::begin();
}

void FX::invertAlpha()
{
  InvertAlpha::begin();
}

void FX::restore()
{
  Restore::begin();
}

void FX::correctionMatrix()
{
  CorrectionMatrix::begin();
}

void FX::removeDust()
{
  RemoveDust::begin();
}

void FX::desaturate()
{
  Desaturate::begin();
}

void FX::colorize()
{
  Colorize::begin();
}

void FX::applyPalette()
{
  ApplyPalette::begin();
}

void FX::stainedGlass()
{
  StainedGlass::begin();
}

void FX::blur()
{
  Blur::begin();
}

void FX::sharpen()
{
  Sharpen::begin();
}

void FX::unsharpMask()
{
  UnsharpMask::begin();
}

void FX::convolutionMatrix()
{
  ConvolutionMatrix::begin();
}

void FX::artistic()
{
  Artistic::begin();
}

void FX::descreen()
{
  Descreen::begin();
}

