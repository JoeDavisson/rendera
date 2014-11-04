/*
Copyright (c) 2014 Joe Davisson.

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

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Dialog.H"
#include "FastRnd.H"
#include "FX.H"
#include "Gamma.H"
#include "Gui.H"
#include "InputInt.H"
#include "Map.H"
#include "Octree.H"
#include "Palette.H"
#include "Project.H"
#include "Separator.H"
#include "Undo.H"
#include "View.H"
namespace
{
  Bitmap *bmp;
  int overscroll;

  void pushUndo()
  {
    bmp = Project::bmp;
    overscroll = bmp->overscroll;
    Undo::push(overscroll, overscroll, bmp->cw, bmp->ch);
  }

  void beginProgress()
  {
    bmp = Project::bmp;
    Dialog::showProgress(bmp->h / 64);
  }

  void endProgress()
  {
    Dialog::hideProgress();
    Gui::getView()->drawMain(true);
  }

  int updateProgress(int y)
  {
    // user cancelled operation
    if(Fl::get_key(FL_Escape))
    {
      Gui::getView()->drawMain(true);
      return -1;
    }

    // only redraw every 64 rasters
    if(!(y % 64))
    {
      Gui::getView()->drawMain(true);
      Dialog::updateProgress();
    }

    return 0;
  }
}

namespace Normalize
{
  void apply()
  {
    // search for highest & lowest RGB values
    int x, y;
    int r_high = 0;
    int g_high = 0;
    int b_high = 0;
    int r_low = 0xFFFFFF;
    int g_low = 0xFFFFFF;
    int b_low = 0xFFFFFF;

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

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

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        const int r = (rgba.r - r_low) * r_scale;
        const int g = (rgba.g - g_low) * g_scale;
        const int b = (rgba.b - b_low) * b_scale;

        *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
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
    int *list_r = new int[256];
    int *list_g = new int[256];
    int *list_b = new int[256];

    int x, y;
    int i, j;

    for(i = 0; i < 256; i++)
    {
      list_r[i] = 0;
      list_g[i] = 0;
      list_b[i] = 0;
    }

    int size = bmp->cw * bmp->ch;

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        list_r[r]++;
        list_g[g]++;
        list_b[b]++;
      }
    }

    for(j = 255; j >= 0; j--)
    {
      for(i = 0; i < j; i++)
      {
        list_r[j] += list_r[i];
        list_g[j] += list_g[i];
        list_b[j] += list_b[i];
      }
    }

    double scale = 255.0 / size;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        r = list_r[r] * scale;
        g = list_g[g] * scale;
        b = list_b[b] * scale;

        *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(updateProgress(y) < 0)
      {
        delete[] list_r;
        delete[] list_g;
        delete[] list_b;
        return;
      }
    }

    delete[] list_r;
    delete[] list_g;
    delete[] list_b;

    endProgress();
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
    int *list_r = new int[256];
    int *list_g = new int[256];
    int *list_b = new int[256];

    int x, y;
    int i, j;

    double rr = 0;
    double gg = 0;
    double bb = 0;
    int count = 0;

    // determine overall color cast
    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        rr += rgba.r;
        gg += rgba.g;
        bb += rgba.b;

        count++;
      }
    }

    rr /= count;
    gg /= count;
    bb /= count;

    for(i = 0; i < 256; i++)
    {
      list_r[i] = 0;
      list_g[i] = 0;
      list_b[i] = 0;
    }

    int size = bmp->cw * bmp->ch;

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        list_r[r]++;
        list_g[g]++;
        list_b[b]++;
      }
    }

    for(j = 255; j >= 0; j--)
    {
      for(i = 0; i < j; i++)
      {
        list_r[j] += list_r[i];
        list_g[j] += list_g[i];
        list_b[j] += list_b[i];
      }
    }

    double scale = 255.0 / size;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        int ra = list_r[r] * scale;
        int ga = list_g[g] * scale;
        int ba = list_b[b] * scale;

        r = ((ra * rr) + (r * (255 - rr))) / 255;
        g = ((ga * gg) + (g * (255 - gg))) / 255;
        b = ((ba * bb) + (b * (255 - bb))) / 255;

        r = std::min(std::max(r, 0), 255);
        g = std::min(std::max(g, 0), 255);
        b = std::min(std::max(b, 0), 255);

        *p++ = makeRgba(r, g, b, rgba.a);
      }

      if(updateProgress(y) < 0)
      {
        delete[] list_r;
        delete[] list_g;
        delete[] list_b;
        return;
      }
    }

    delete[] list_r;
    delete[] list_g;
    delete[] list_b;

    endProgress();
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
    int *list_s = new int[256];

    int x, y;
    int i, j;

    for(i = 0; i < 256; i++)
      list_s[i] = 0;

    int size = bmp->cw * bmp->ch;

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(bmp->getpixel(x, y));

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        int h, s, v;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);

        list_s[s]++;
      }
    }
    for(j = 255; j >= 0; j--)
      for(i = 0; i < j; i++)
        list_s[j] += list_s[i];

    double scale = 255.0 / size;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        const int c = bmp->getpixel(x, y);

        rgba_t rgba = getRgba(c);

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

      if(updateProgress(y) < 0)
      {
        delete[] list_s;
        return;
      }
    }

    delete[] list_s;

    endProgress();
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

namespace RotateHue
{
  Fl_Double_Window *dialog;
  InputInt *angle;
  Fl_Check_Button *preserve;
  Fl_Button *ok;
  Fl_Button *cancel;

  void apply(int amount)
  {
    int x, y;
    int r, g, b;
    int h, s, v;

    int hh = amount * 4.277;

    bool keep_lum = preserve->value();

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);

        rgba_t rgba = getRgba(c);

        int l = getl(c);

        r = rgba.r;
        g = rgba.g;
        b = rgba.b;

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

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void close()
  {
    if(angle->limitValue(-359, 359) < 0)
      return;

    int angle_val = atoi(angle->value());

    if(angle_val < 0)
      angle_val += 360;

    dialog->hide();
    pushUndo();
    apply(angle_val);
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Rotate Hue");
    angle = new InputInt(dialog, 0, y1, 72, 24, "Angle:", 0);
    y1 += 24 + 8;
    angle->maximum_size(4);
    angle->value("60");
    angle->center();
    preserve = new Fl_Check_Button(0, y1, 16, 16, "Preserve Luminance");
    Dialog::center(preserve);
    y1 += 16 + 8;
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
}

namespace Invert
{
  void apply()
  {
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);

        *p++ = Blend::invert(c, 0, 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
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
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        const int c = bmp->getpixel(x, y);

        rgba_t rgba = getRgba(c);

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;
        const int l = getl(c);

        // only change hue
        int h, s, v;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        int sat = s;
        int val = v;

        // correction matrix
        int ra = r;
        int ga = (r * 4 + g * 8 + b * 1) / 13;
        int ba = (r * 2 + g * 4 + b * 8) / 14;

        ra = std::max(std::min(ra, 255), 0);
        ga = std::max(std::min(ga, 255), 0);
        ba = std::max(std::min(ba, 255), 0);

        Blend::rgbToHsv(ra, ga, ba, &h, &s, &v);
        Blend::hsvToRgb(h, sat, val, &ra, &ga, &ba);

        *p++ = Blend::keepLum(makeRgba(ra, ga, ba, rgba.a), l);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
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
  Fl_Double_Window *dialog;
  Fl_Check_Button *normalize;
  Fl_Check_Button *invert;
  Fl_Check_Button *correct;
  Fl_Check_Button *color_only;
  Fl_Button *ok;
  Fl_Button *cancel;

  void apply()
  {
    int x, y;

    float rr = 0;
    float gg = 0;
    float bb = 0;
    int count = 0;

    bool keep_lum = color_only->value();

    // determine overall color cast
    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        const struct rgba_t rgba = getRgba(bmp->getpixel(x, y));

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
    float ra = (256.0f / (256 - rr)) / std::sqrt(256.0f / (rr + 1));
    float ga = (256.0f / (256 - gg)) / std::sqrt(256.0f / (gg + 1));
    float ba = (256.0f / (256 - bb)) / std::sqrt(256.0f / (bb + 1));

    // begin restore
    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        const int c = bmp->getpixel(x, y);
        const struct rgba_t rgba = getRgba(c);
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

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void close()
  {
    dialog->hide();
    pushUndo();

    if(normalize->value())
      Normalize::apply();
    if(invert->value())
      Invert::apply();

    apply();

    if(invert->value())
      Invert::apply();
    if(correct->value())
      CorrectionMatrix::apply();
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Restore");
    normalize = new Fl_Check_Button(0, y1, 16, 16, "Normalize First");
    y1 += 16 + 8;
    normalize->value(1);
    Dialog::center(normalize);
    invert = new Fl_Check_Button(0, y1, 16, 16, "Invert First");
    y1 += 16 + 8;
    Dialog::center(invert);
    correct = new Fl_Check_Button(8, y1, 16, 16, "Use Correction Matrix");
    y1 += 16 + 8;
    Dialog::center(correct);
    color_only = new Fl_Check_Button(8, y1, 16, 16, "Affect Color Only");
    y1 += 16 + 8;
    Dialog::center(color_only);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
}

namespace RemoveDust
{
  Fl_Double_Window *dialog;
  InputInt *amount;
  Fl_Check_Button *invert;
  Fl_Button *ok;
  Fl_Button *cancel;

  void apply(int amount)
  {
    int x, y, i;
    int c[8];
    int r, g, b;
    int test, avg;

    beginProgress();

    for(y = (overscroll + 1); y < bmp->h - (overscroll + 1); y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = (overscroll + 1); x < bmp->w - (overscroll + 1); x++)
      {
        test = bmp->getpixel(x, y);

        c[0] = bmp->getpixel(x + 1, y);
        c[1] = bmp->getpixel(x - 1, y);
        c[2] = bmp->getpixel(x, y + 1);
        c[3] = bmp->getpixel(x, y - 1);
        c[4] = bmp->getpixel(x - 1, y - 1);
        c[5] = bmp->getpixel(x + 1, y - 1);
        c[6] = bmp->getpixel(x - 1, y + 1);
        c[7] = bmp->getpixel(x + 1, y + 1);

        r = g = b = 0;

        for(i = 0; i < 8; i++)
        {
          rgba_t rgba = getRgba(c[i]);
          r += rgba.r;
          g += rgba.g;
          b += rgba.b;
        }

        avg = makeRgba(r / 8, g / 8, b / 8, geta(test));

        if((getl(avg) - getl(test)) > amount)
          *p = avg;

        p++;
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void close()
  {
    if(amount->limitValue(1, 10) < 0)
      return;

    int amount_val = atoi(amount->value());

    dialog->hide();
    pushUndo();

    if(invert->value())
      Invert::apply();

    apply(amount_val);

    if(invert->value())
      Invert::apply();
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Remove Dust");
    amount = new InputInt(dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    amount->value("4");
    amount->center();
    invert = new Fl_Check_Button(0, y1, 16, 16, "Invert First");
    y1 += 16 + 8;
    Dialog::center(invert);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
}

namespace Desaturate
{
  void apply()
  {
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int l = getl(c);

        *p++ = makeRgba(l, l, l, geta(c));
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
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
    int x, y;

    rgba_t rgba_color = getRgba(Project::brush->color);

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c1 = bmp->getpixel(x, y);

        rgba_t rgba = getRgba(c1);

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

        int c2 = makeRgba(r, g, b, rgba.a);

        *p++ = Blend::colorize(c1, c2, 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

namespace ApplyPalette
{
  Fl_Double_Window *dialog;
  Fl_Check_Button *dither;
  Fl_Button *ok;
  Fl_Button *cancel;

  void applyNormal()
  {
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        *p++ = Project::palette->data[(int)Project::palette->lookup(c)];
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void applyDither()
  {
    int x, y;
    int i, j;
    Bitmap *bmp = Project::bmp;

    int e[3], v[3], n[3], last[3];
    int *buf[3], *prev[3];

    for(i = 0; i < 3; i++)
    {
      buf[i] = new int[bmp->w];
      prev[i] = new int[bmp->w];

      for(j = 0; j < bmp->w; j++)
      {
        *(buf[i] + j) = 0;
        *(prev[i] + j) = 0;
      }

      last[i] = 0;
    }

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        rgba_t rgba = getRgba(*p);
        v[0] = Gamma::fix(rgba.r);
        v[1] = Gamma::fix(rgba.g);
        v[2] = Gamma::fix(rgba.b);

        for(i = 0; i < 3; i++)
        {
          n[i] = v[i] + last[i] + prev[i][x];
          n[i] = std::max(std::min(n[i], 65535), 0);
        }

        const int r = Gamma::unfix(n[0]);
        const int g = Gamma::unfix(n[1]);
        const int b = Gamma::unfix(n[2]);

        const int pal_index = (int)Project::palette->lookup(makeRgb(r, g, b));
        const int c = Project::palette->data[pal_index];

        rgba = getRgba(c);
        *p = makeRgb(rgba.r, rgba.g, rgba.b);

        rgba = getRgba(*p);
        v[0] = Gamma::fix(rgba.r);
        v[1] = Gamma::fix(rgba.g);
        v[2] = Gamma::fix(rgba.b);

        p++;

        for(i = 0; i < 3; i++)
        {
          e[i] = n[i] - v[i];
          last[i] = (e[i] * 7)  >> 4;
          buf[i][x - 1] += (e[i] * 3)  >> 4;
          buf[i][x] += (e[i] * 5)  >> 4;
          buf[i][x + 1] += (e[i] * 1)  >> 4;
        }
      }

      for(i = 0; i < 3; i++)
      {
        for(j = overscroll; j < bmp->w - overscroll; j++)
        {
          *(prev[i] + j) = *(buf[i] + j);
          *(buf[i] + j) = 0;
        }
        last[i] = 0;
      }

      if(updateProgress(y) < 0)
      {
        for(i = 0; i < 3; i++)
        {
          delete[] buf[i];
          delete[] prev[i];
        }
        return;
      }
    }

    for(i = 0; i < 3; i++)
    {
      delete[] buf[i];
      delete[] prev[i];
    }

    endProgress();
  }

  void close()
  {
    dialog->hide();
    pushUndo();

    if(dither->value())
      applyDither();
    else
      applyNormal();
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Apply Palette");
    dither = new Fl_Check_Button(0, y1, 16, 16, "Dithering");
    Dialog::center(dither);
    y1 += 16 + 8;
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
}

namespace StainedGlass
{
  Fl_Double_Window *dialog;
  InputInt *detail;
  InputInt *edge;
  Fl_Check_Button *uniform;
  Fl_Check_Button *sat_alpha;
  Fl_Check_Button *draw_edges;
  Fl_Button *ok;
  Fl_Button *cancel;

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
    int x, y;
    int i, j, k;

    int *seedx = new int[size];
    int *seedy = new int[size];
    int *color = new int[size];

    for(i = 0; i < size; i++)
    {
      if(uniform->value())
      {
        seedx[i] = FastRnd::get() % bmp->w; 
        seedy[i] = FastRnd::get() % bmp->h; 
      }
      else
      {
        seedx[i] = FastRnd::get() % bmp->w; 
        seedy[i] = FastRnd::get() % bmp->h; 
  
        int count = 0;

        do
        {
          seedx[i] = FastRnd::get() % bmp->w; 
          seedy[i] = FastRnd::get() % bmp->h; 
          count++;
        }
        while(!isEdge(bmp, seedx[i], seedy[i], div) && count < 10000);
      }

      color[i] = bmp->getpixel(seedx[i], seedy[i]);
    }

    beginProgress();

    // draw segments
    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        // find nearest color
        int nearest = 999999999;
        int use = -1;

        for(i = 0; i < size; i++)
        {
          int dx = x - seedx[i];
          int dy = y - seedy[i];
          int distance = dx * dx + dy * dy;

          if(distance < nearest)
          {
            nearest = distance;
            use = i;
          }
        }

        if(use != -1)
        {
          if(sat_alpha->value())
          {
            rgba_t rgba = getRgba(color[use]);

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

      if(updateProgress(y) < 0)
      {
        delete[] color;
        delete[] seedy;
        delete[] seedx;
        return;
      }
    }

    // draw edges
    if(draw_edges->value())
    {
      Map *map = Project::map;
      map->clear(0);

      for(y = overscroll * 4; y < (bmp->h - overscroll) * 4; y++)
      {
        for(x = overscroll * 4; x < (bmp->w - overscroll) * 4; x++)
        {
          if(isSegmentEdge(bmp, x / 4, y / 4))
            map->setpixelAA(x, y, 255);
        }
      }

      for(y = overscroll; y < bmp->h - overscroll; y++)
      {
        int *p = bmp->row[y] + overscroll;

        for(x = overscroll; x < bmp->w - overscroll; x++)
        {
          const int c = map->getpixel(x, y);

          *p++ = Blend::trans(bmp->getpixel(x, y), makeRgb(0, 0, 0), 255 - c);
        }
      }
    }

    delete[] color;
    delete[] seedy;
    delete[] seedx;

    endProgress();
  }

  void close()
  {
    if(detail->limitValue(1, 50000) < 0)
      return;

    if(edge->limitValue(1, 50) < 0)
      return;

    int detail_val = atoi(detail->value());
    int edge_val = atoi(edge->value());

    dialog->hide();
    pushUndo();
    apply(detail_val, edge_val);
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Stained Glass");
    detail = new InputInt(dialog, 0, y1, 72, 24, "Detail:", 0);
    y1 += 24 + 8;
    detail->value("5000");
    detail->center();
    edge = new InputInt(dialog, 0, y1, 72, 24, "Edge Detect:", 0);
    y1 += 24 + 8;
    edge->value("16");
    edge->center();
    uniform = new Fl_Check_Button(0, y1, 16, 16, "Uniform");
    Dialog::center(uniform);
    y1 += 16 + 8;
    sat_alpha = new Fl_Check_Button(0, y1, 16, 16, "Saturation to Alpha");
    Dialog::center(sat_alpha);
    y1 += 16 + 8;
    draw_edges = new Fl_Check_Button(0, y1, 16, 16, "Draw Edges");
    Dialog::center(draw_edges);
    y1 += 16 + 8;
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
}

namespace Blur
{
  Fl_Double_Window *dialog;
  InputInt *amount;
  Fl_Button *ok;
  Fl_Button *cancel;

  void apply(int size)
  {
    size = (size + 1) * 2 + 1;

    int *kernel = new int[size];
    int x, y;
    int div = 0;

    // bell curve
    const int b = size / 2;

    for(x = 0; x < size; x++)
    {
      kernel[x] = 255 * std::exp(-((double)((x - b) * (x - b)) / ((b * b) / 2)));
      div += kernel[x];
    }

    Bitmap *temp = new Bitmap(bmp->cw, bmp->ch);
    beginProgress();

    // x direction
    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = temp->row[y - overscroll];

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int rr = 0;
        int gg = 0;
        int bb = 0;
        int aa = 0;
        int i;

        for(i = 0; i < size; i++) 
        {
          rgba_t rgba = getRgba(bmp->getpixel(x - size / 2 + i, y));
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

      if(updateProgress(y) < 0)
      {
        delete temp;
        return;
      }
    }

    temp->blit(bmp, 0, 0, bmp->cl, bmp->ct, temp->w, temp->h);
    beginProgress();

    // y direction
    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = temp->row[y - overscroll];

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int rr = 0;
        int gg = 0;
        int bb = 0;
        int aa = 0;
        int i;

        for(i = 0; i < size; i++) 
        {
          rgba_t rgba = getRgba(bmp->getpixel(x, y - size / 2 + i));
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

      if(updateProgress(y) < 0)
      {
        delete temp;
        return;
      }
    }

    temp->blit(bmp, 0, 0, bmp->cl, bmp->ct, temp->w, temp->h);
    delete temp;

    endProgress();
  }

  void close()
  {
    if(amount->limitValue(1, 100) < 0)
      return;

    int amount_val = atoi(amount->value());

    dialog->hide();
    pushUndo();
    apply(amount_val);
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Blur");
    amount = new InputInt(dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    amount->value("1");
    amount->center();
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
}

namespace Sharpen
{
  Fl_Double_Window *dialog;
  InputInt *amount;
  Fl_Button *ok;
  Fl_Button *cancel;

  const int matrix[3][3] =
  {
    { -1, -1, -1 },
    { -1,  9, -1 },
    { -1, -1, -1 }
  };

  void apply(int amount)
  {
    int x, y, i;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      int *p = bmp->row[y] + overscroll;

      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int i, j;
        int lum = 0;

        for(j = 0; j < 3; j++) 
        {
          for(i = 0; i < 3; i++) 
          {
            lum += getl(bmp->getpixel(x + i - 1, y + j - 1)) * matrix[i][j];
          }
        }

        const int c = bmp->getpixel(x, y);

        lum = std::min(std::max(lum, 0), 255);
        *p++ = Blend::trans(c, Blend::keepLum(c, lum), 255 - amount);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void close()
  {
    if(amount->limitValue(1, 255) < 0)
      return;

    int amout_val = atoi(amount->value());

    dialog->hide();
    pushUndo();
    apply(amout_val);
  }

  void quit()
  {
    endProgress();
    dialog->hide();
  }

  void begin()
  {
    dialog->show();
  }

  int *init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Sharpen");
    amount = new InputInt(dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    amount->value("64");
    amount->center();
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();

    return 0;
  }

  static const int *temp = init();
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

