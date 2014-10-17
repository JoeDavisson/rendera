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

#include "FX.H"
#include "Bitmap.H"
#include "Palette.H"
#include "Blend.H"
#include "Brush.H"
#include "Dialog.H"
#include "InputInt.H"
#include "Separator.H"
#include "Gui.H"
#include "View.H"
#include "Undo.H"
#include "Octree.H"
#include "Project.H"

extern int *fix_gamma;
extern int *unfix_gamma;

namespace
{
  Bitmap *bmp;
  int overscroll;

  void pushUndo()
  {
    bmp = Project::bmp;
    overscroll = bmp->overscroll;
    Undo::push(overscroll, overscroll,
               bmp->w - overscroll * 2, bmp->h - overscroll * 2, 0);
  }

  void beginProgress()
  {
    bmp = Project::bmp;
    Dialog::showProgress(bmp->h / 64);
  }

  void endProgress()
  {
    Dialog::hideProgress();
    Gui::getView()->drawMain(1);
  }

  int updateProgress(int y)
  {
    // user cancelled operation
    if(Fl::get_key(FL_Escape))
    {
      endProgress();
      return -1;
    }

    // only redraw every 64 rasters
    if(!(y % 64))
    {
      Gui::getView()->drawMain(1);
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
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);

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
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = (getr(c) - r_low) * r_scale;
        int g = (getg(c) - g_low) * g_scale;
        int b = (getb(c) - b_low) * b_scale;

        bmp->setpixel(x, y, makeRgba(r, g, b, geta(c)), 0);
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

    int size = (bmp->w - overscroll * 2) * (bmp->h - overscroll * 2);

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);

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
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);

        r = list_r[r] * scale;
        g = list_g[g] * scale;
        b = list_b[b] * scale;

        bmp->setpixel(x, y, makeRgba(r, g, b, geta(c)), 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();

    delete[] list_r;
    delete[] list_g;
    delete[] list_b;
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

// equalizes while retaining the overall color cast of an image
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
        int c = bmp->getpixel(x, y);

        rr += getr(c);
        gg += getg(c);
        bb += getb(c);

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

    int size = (bmp->w - overscroll * 2) * (bmp->h - overscroll * 2);

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);

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
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);

        int ra = list_r[r] * scale;
        int ga = list_g[g] * scale;
        int ba = list_b[b] * scale;

        r = ((ra * rr) + (r * (255 - rr))) / 255;
        g = ((ga * gg) + (g * (255 - gg))) / 255;
        b = ((ba * bb) + (b * (255 - bb))) / 255;

        r = std::min(std::max(r, 0), 255);
        g = std::min(std::max(g, 0), 255);
        b = std::min(std::max(b, 0), 255);

        bmp->setpixel(x, y, makeRgba(r, g, b, geta(c)), 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();

    delete[] list_r;
    delete[] list_g;
    delete[] list_b;
  }

  void begin()
  {
    pushUndo();
    apply();
  }
}

// in some cases can restore color saturation to a faded image
namespace Saturate
{
  void apply()
  {
    int *list_s = new int[256];

    int x, y;
    int i, j;

    for(i = 0; i < 256; i++)
      list_s[i] = 0;

    int size = (bmp->w - overscroll * 2) * (bmp->h - overscroll * 2);

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);
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
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);
        int l = getl(c);
        int h, s, v;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        int temp = s;
        s = list_s[s] * scale;
        if(s < temp)
          s = temp;
        Blend::hsvToRgb(h, s, v, &r, &g, &b);

        bmp->setpixel(x, y, Blend::keepLum(makeRgba(r, g, b, geta(c)), l), 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();

    delete[] list_s;
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
  InputInt *amount;
  Fl_Check_Button *preserve;
  Fl_Button *ok;
  Fl_Button *cancel;

  void apply(int amount)
  {
    int x, y;
    int r, g, b;
    int h, s, v;

    int hh = amount * 4.277;

    int keep_lum = 0;
    if(preserve->value())
      keep_lum = 1;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int l = getl(c);

        r = getr(c);
        g = getg(c);
        b = getb(c);
        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        h += hh;
        if(h >= 1536)
          h -= 1536;
        Blend::hsvToRgb(h, s, v, &r, &g, &b);
        c = makeRgba(r, g, b, geta(c));

        if(keep_lum)
          bmp->setpixel(x, y, Blend::keepLum(c, l), 0);
        else
          bmp->setpixel(x, y, c, 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void close()
  {
    char str[8];
    int a = atoi(amount->value());

    if(a < -359)
    {
      snprintf(str, sizeof(str), "%d", -359);
      amount->value(str);
      return;
    }

    if(a > 359)
    {
      snprintf(str, sizeof(str), "%d", 359);
      amount->value(str);
      return;
    }

    if(a < 0)
      a += 360;

    dialog->hide();

    pushUndo();
    apply(a);
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

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Rotate Hue");
    amount = new InputInt(dialog, 0, y1, 72, 24, "Amount:", 0);
    y1 += 24 + 8;
    amount->maximum_size(4);
    amount->value("60");
    amount->center();
    preserve = new Fl_Check_Button(0, y1, 16, 16, "Preserve Luminance");
    Dialog::center(preserve);
    y1 += 16 + 8;
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();
  }
}

namespace Invert
{
  void apply()
  {
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        bmp->setpixel(x, y, Blend::invert(c, 0, 0), 0);
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

// corrects uneven dye fading in photographs (especially when there is a severe
// color cast, such as when one of the dyes have faded almost completety)
//
// sometimes works better before or after the restore filter depending on
// the image
namespace CorrectionMatrix
{
  void apply()
  {
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int r = getr(c);
        int g = getg(c);
        int b = getb(c);
        int l = getl(c);

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

        bmp->setpixel(x, y,
          Blend::keepLum(makeRgba(ra, ga, ba, geta(c)), l), 0);
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

// this algorithm assumes a picture both has a color cast and is faded
// in most cases (with a good scan) can restore nearly all the color
// to a faded photograph
namespace Restore
{
  Fl_Double_Window *dialog;
  Fl_Check_Button *normalize;
  Fl_Check_Button *invert;
  Fl_Check_Button *correct;
  Fl_Button *ok;
  Fl_Button *cancel;

  void apply()
  {
    int x, y;

    float rr = 0;
    float gg = 0;
    float bb = 0;
    int count = 0;

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
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        const struct rgba_t rgba = getRgba(bmp->getpixel(x, y));
        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        // apply adjustments
        r = 255 * powf((float)r / 255, ra);
        g = 255 * powf((float)g / 255, ga);
        b = 255 * powf((float)b / 255, ba);

        r = std::max(std::min(r, 255), 0);
        g = std::max(std::min(g, 255), 0);
        b = std::max(std::min(b, 255), 0);

        bmp->setpixel(x, y, makeRgba(r, g, b, rgba.a), 0);
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

  void init()
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
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();
  }
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
          r += getr(c[i]);
          g += getg(c[i]);
          b += getb(c[i]);
        }

        avg = makeRgba(r / 8, g / 8, b / 8, geta(test));
        if((getl(avg) - getl(test)) > amount)
          bmp->setpixel(x, y, avg, 0);
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();
  }

  void close()
  {
    char str[8];
    int a = atoi(amount->value());

    if(a < 1)
    {
      snprintf(str, sizeof(str), "%d", 1);
      amount->value(str);
      return;
    }

    if(a > 10)
    {
      snprintf(str, sizeof(str), "%d", 10);
      amount->value(str);
      return;
    }

    pushUndo();

    if(invert->value())
      Invert::apply();

    apply(a);

    if(invert->value())
      Invert::apply();

    dialog->hide();
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

  void init()
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
  }
}

namespace Desaturate
{
  void apply()
  {
    int x, y;

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        int l = getl(c);
        bmp->setpixel(x, y, makeRgba(l, l, l, geta(c)), 0);
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

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c1 = bmp->getpixel(x, y);
        int r = getr(c1);
        int g = getg(c1);
        int b = getb(c1);
        int h, s, v;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        int sat = s;
        if(sat < 64)
          sat = 64;
        r = getr(Project::brush->color);
        g = getg(Project::brush->color);
        b = getb(Project::brush->color);
        Blend::rgbToHsv(r, g, b, &h, &s, &v);
        Blend::hsvToRgb(h, (sat * s) / (sat + s), v, &r, &g, &b);
        int c2 = makeRgba(r, g, b, geta(c1));
        bmp->setpixel(x, y, Blend::colorize(c1, c2, 0), 0);
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
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        int c = bmp->getpixel(x, y);
        bmp->setpixel(x, y,
          Project::palette->data[(int)Project::palette->lookup(c)], 0);
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
        v[0] = fix_gamma[getr(*p)];
        v[1] = fix_gamma[getg(*p)];
        v[2] = fix_gamma[getb(*p)];

        for(i = 0; i < 3; i++)
        {
          n[i] = v[i] + last[i] + prev[i][x];
          n[i] = std::max(std::min(n[i], 65535), 0);
        }

        const int r = unfix_gamma[n[0]];
        const int g = unfix_gamma[n[1]];
        const int b = unfix_gamma[n[2]];

        const int pal_index = (int)Project::palette->lookup(makeRgb(r, g, b));
        const int c = Project::palette->data[pal_index];

        struct rgba_t rgba = getRgba(c);

        *p = makeRgb(rgba.r, rgba.g, rgba.b);

        v[0] = fix_gamma[getr(*p)];
        v[1] = fix_gamma[getg(*p)];
        v[2] = fix_gamma[getb(*p)];

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
        return;
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

  void init()
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
  }
}

namespace StainedGlass
{
  Fl_Double_Window *dialog;
  InputInt *detail;
  InputInt *edge;
  Fl_Check_Button *uniform;
  Fl_Button *ok;
  Fl_Button *cancel;

  static inline int isEdge(Bitmap *temp, const int &x, const int &y,
                           const int &div)
  {
    const int lum0 = getl(temp->getpixel(x, y)) / div;
    const int lum1 = getl(temp->getpixel(x + 1, y)) / div;
    const int lum2 = getl(temp->getpixel(x, y + 1)) / div;
    const int lum3 = getl(temp->getpixel(x + 1, y + 1)) / div;

    if((lum0 == lum1) && (lum0 == lum2) && (lum0 == lum3))
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
        seedx[i] = rnd32() % bmp->w; 
        seedy[i] = rnd32() % bmp->h; 
      }
      else
      {
        do
        {
          seedx[i] = rnd32() % bmp->w; 
          seedy[i] = rnd32() % bmp->h; 
        }
        while(!isEdge(bmp, seedx[i], seedy[i], div));
      }

      color[i] = bmp->getpixel(seedx[i], seedy[i]);
    }

    beginProgress();

    for(y = overscroll; y < bmp->h - overscroll; y++)
    {
      for(x = overscroll; x < bmp->w - overscroll; x++)
      {
        // find nearest color
        int nearest = 999999;
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
          bmp->setpixel(x, y, color[use], 0);
        }
      }

      if(updateProgress(y) < 0)
        return;
    }

    endProgress();

    delete[] color;
    delete[] seedy;
    delete[] seedx;
  }

  void close()
  {
    char str[8];
    int a = atoi(detail->value());

    if(a < 1)
    {
      snprintf(str, sizeof(str), "%d", 1);
      detail->value(str);
      return;
    }

    if(a > 16384)
    {
      snprintf(str, sizeof(str), "%d", 16384);
      detail->value(str);
      return;
    }

    int b = atoi(edge->value());

    if(b < 1)
    {
      snprintf(str, sizeof(str), "%d", 1);
      edge->value(str);
      return;
    }

    if(b > 64)
    {
      snprintf(str, sizeof(str), "%d", 64);
      edge->value(str);
      return;
    }
    pushUndo();

    apply(a, b);

    dialog->hide();
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

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Stained Glass");
    detail = new InputInt(dialog, 0, y1, 72, 24, "Detail:", 0);
    y1 += 24 + 8;
    detail->value("1000");
    detail->center();
    edge = new InputInt(dialog, 0, y1, 72, 24, "Edge Detect:", 0);
    y1 += 24 + 8;
    edge->value("16");
    edge->center();
    uniform = new Fl_Check_Button(0, y1, 16, 16, "Uniform");
    y1 += 16 + 8;
    Dialog::center(uniform);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end();
  }
}

void FX::init()
{
  RotateHue::init();
  Restore::init();
  RemoveDust::init();
  ApplyPalette::init();
  StainedGlass::init();
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

