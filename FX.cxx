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

#include "FX.h"
#include "Bitmap.h"
#include "Palette.h"
#include "Blend.h"
#include "Brush.h"
#include "Dialog.h"
#include "Field.h"
#include "Separator.h"
#include "Gui.h"
#include "View.h"
#include "Undo.h"

extern int *fix_gamma;
extern int *unfix_gamma;

namespace
{
  Fl_Double_Window *rotate_hue;
  Field *rotate_hue_amount;
  Fl_Check_Button *rotate_hue_preserve;
  Fl_Button *rotate_hue_ok;
  Fl_Button *rotate_hue_cancel;

  Fl_Double_Window *restore;
  Fl_Check_Button *restore_normalize;
  Fl_Check_Button *restore_invert;
  Fl_Check_Button *restore_correct;
  Fl_Button *restore_ok;
  Fl_Button *restore_cancel;

  Fl_Double_Window *remove_dust;
  Field *remove_dust_amount;
  Fl_Check_Button *remove_dust_invert;
  Fl_Button *remove_dust_ok;
  Fl_Button *remove_dust_cancel;

  Fl_Double_Window *apply_palette;
  Fl_Check_Button *apply_palette_dither;
  Fl_Button *apply_palette_ok;
  Fl_Button *apply_palette_cancel;

  Bitmap *bmp;
  int overscroll;

  void begin()
  {
    bmp = Bitmap::main;
    overscroll = Bitmap::main->overscroll;

    Undo::push(overscroll, overscroll,
              bmp->w - overscroll * 2, bmp->h - overscroll * 2, 0);
  }
}

void FX::init()
{
  rotate_hue = new Fl_Double_Window(240, 104, "Rotate Hue");
  rotate_hue_amount = new Field(rotate_hue, 108, 8, 72, 24, "Amount:", 0);
  rotate_hue_amount->maximum_size(4);
  rotate_hue_amount->value("60");
  rotate_hue_preserve = new Fl_Check_Button(32, 40, 16, 16,
                                            "Preserve Luminance");
  new Separator(rotate_hue, 2, 62, 236, 2, "");
  rotate_hue_ok = new Fl_Button(96, 72, 64, 24, "OK");
  rotate_hue_ok->callback((Fl_Callback *)hideRotateHue);
  rotate_hue_cancel = new Fl_Button(168, 72, 64, 24, "Cancel");
  rotate_hue_cancel->callback((Fl_Callback *)cancelRotateHue);
  rotate_hue->set_modal();
  rotate_hue->end();

  restore = new Fl_Double_Window(208, 120, "Restore");
  restore_normalize = new Fl_Check_Button(8, 8, 16, 16, "Normalize First");
  restore_normalize->value(1);
  restore_invert = new Fl_Check_Button(8, 32, 16, 16, "Invert First");
  restore_correct = new Fl_Check_Button(8, 56, 16, 16, "Use Correction Matrix");
  new Separator(restore, 2, 80, 204, 2, "");
  restore_ok = new Fl_Button(64, 88, 64, 24, "OK");
  restore_ok->callback((Fl_Callback *)hideRestore);
  restore_cancel = new Fl_Button(136, 88, 64, 24, "Cancel");
  restore_cancel->callback((Fl_Callback *)cancelRestore);
  restore->set_modal();
  restore->end();

  remove_dust = new Fl_Double_Window(240, 104, "Remove Dust");
  remove_dust_amount = new Field(remove_dust, 108, 8, 72, 24, "Amount:", 0);
  remove_dust_amount->value("4");
  remove_dust_invert = new Fl_Check_Button(64, 40, 16, 16, "Invert First");
  new Separator(remove_dust, 2, 62, 236, 2, "");
  remove_dust_ok = new Fl_Button(96, 72, 64, 24, "OK");
  remove_dust_ok->callback((Fl_Callback *)hideRemoveDust);
  remove_dust_cancel = new Fl_Button(168, 72, 64, 24, "Cancel");
  remove_dust_cancel->callback((Fl_Callback *)cancelRemoveDust);
  remove_dust->set_modal();
  remove_dust->end();

  apply_palette = new Fl_Double_Window(200, 72, "Apply Palette");
  apply_palette_dither = new Fl_Check_Button(48, 8, 16, 16, "Dithering");
  new Separator(apply_palette, 2, 32, 196, 2, "");
  apply_palette_ok = new Fl_Button(56, 40, 64, 24, "OK");
  apply_palette_ok->callback((Fl_Callback *)hideApplyPalette);
  apply_palette_cancel = new Fl_Button(128, 40, 64, 24, "Cancel");
  apply_palette_cancel->callback((Fl_Callback *)cancelApplyPalette);
  apply_palette->set_modal();
  apply_palette->end();
}

// normalize
void FX::showNormalize()
{
  begin();
  doNormalize();
}

void FX::doNormalize()
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

  Dialog::showProgress((float)bmp->h / 64);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = (getr(c) - r_low) * r_scale;
      int g = (getg(c) - g_low) * g_scale;
      int b = (getb(c) - b_low) * b_scale;

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::showEqualize()
{
  begin();
  doEqualize();
}

void FX::doEqualize()
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

  Dialog::showProgress((float)bmp->h / 64);

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

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();

  delete[] list_r;
  delete[] list_g;
  delete[] list_b;
}

// value stretch
void FX::showValueStretch()
{
  begin();
  doValueStretch();
}

// equalizes while retaining the overall color cast of an image
void FX::doValueStretch()
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

  Dialog::showProgress((float)bmp->h / 64);

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

      r = MIN(MAX(r, 0), 255);
      g = MIN(MAX(g, 0), 255);
      b = MIN(MAX(b, 0), 255);

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();

  delete[] list_r;
  delete[] list_g;
  delete[] list_b;
}

// saturate
void FX::showSaturate()
{
  begin();
  doSaturate();
}

// equalize saturation
// in some cases can restore color saturation to a faded image
void FX::doSaturate()
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

  Dialog::showProgress((float)bmp->h / 64);

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

      bmp->setpixel(x, y, Blend::forceLuminance(makecol(r, g, b), l), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();

  delete[] list_s;
}

// rotate hue
void FX::showRotateHue()
{
  rotate_hue->show();
}

void FX::hideRotateHue()
{
  char str[8];
  int amount = atoi(rotate_hue_amount->value());

  if(amount < -359)
  {
    snprintf(str, sizeof(str), "%d", -359);
    rotate_hue_amount->value(str);
    return;
  }

  if(amount > 359)
  {
    snprintf(str, sizeof(str), "%d", 359);
    rotate_hue_amount->value(str);
    return;
  }

  if(amount < 0)
    amount += 360;

  begin();
  doRotateHue(amount);
  rotate_hue->hide();
}

void FX::doRotateHue(int amount)
{
  int x, y;
  int r, g, b;
  int h, s, v;

  int hh = amount * 4.277;

  Dialog::showProgress((float)bmp->h / 64);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);

      r = getr(c);
      g = getg(c);
      b = getb(c);
      Blend::rgbToHsv(r, g, b, &h, &s, &v);
      h += hh;
      if(h >= 1536)
        h -= 1536;
      Blend::hsvToRgb(h, s, v, &r, &g, &b);
      c = makecol(r, g, b);

      bmp->setpixel(x, y, c, 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::cancelRotateHue()
{
  Dialog::hideProgress();
  rotate_hue->hide();
}

// invert
void FX::showInvert()
{
  begin();
  doInvert();
}

void FX::doInvert()
{
  int x, y;

  Dialog::showProgress((float)bmp->h / 64);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      bmp->setpixel(x, y, Blend::invert(c, 0, 0), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

// restore
void FX::showRestore()
{
  restore->show();
}

void FX::hideRestore()
{
  begin();
  if(restore_normalize->value())
    doNormalize();
  if(restore_invert->value())
    doInvert();
  doRestore();
  if(restore_invert->value())
    doInvert();
  if(restore_correct->value())
    doCorrect();
  restore->hide();
}

// this algorithm assumes a picture both has a color cast and is faded
// in most cases (with a good scan) can restore nearly all the color
// to a faded photograph
void FX::doRestore()
{
  int x, y;

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

  // adjustment factors
  double ra = (256.0 / (256 - rr)) / sqrt(256.0 / (rr + 1));
  double ga = (256.0 / (256 - gg)) / sqrt(256.0 / (gg + 1));
  double ba = (256.0 / (256 - bb)) / sqrt(256.0 / (bb + 1));

  // begin restore
  Dialog::showProgress((float)bmp->h / 64);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = getr(c);
      int g = getg(c);
      int b = getb(c);

      // apply adjustments
      r = 255 * pow((double)r / 255, ra);
      g = 255 * pow((double)g / 255, ga);
      b = 255 * pow((double)b / 255, ba);

      r = MAX(MIN(r, 255), 0);
      g = MAX(MIN(g, 255), 0);
      b = MAX(MIN(b, 255), 0);

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::cancelRestore()
{
  Dialog::hideProgress();
  restore->hide();
}

// remove dust
void FX::showRemoveDust()
{
  remove_dust->show();
}

void FX::hideRemoveDust()
{
  char str[8];
  int amount = atoi(remove_dust_amount->value());

  if(amount < 1)
  {
    snprintf(str, sizeof(str), "%d", 1);
    remove_dust_amount->value(str);
    return;
  }

  if(amount > 10)
  {
    snprintf(str, sizeof(str), "%d", 10);
    remove_dust_amount->value(str);
    return;
  }

  begin();
  if(remove_dust_invert->value())
    doInvert();
  doRemoveDust(amount);
  if(remove_dust_invert->value())
    doInvert();
  remove_dust->hide();
}

void FX::doRemoveDust(int amount)
{
  int x, y, i;
  int c[8];
  int r, g, b;
  int test, avg;

  Dialog::showProgress((float)bmp->h / 64);

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

      avg = makecol(r / 8, g / 8, b / 8);
      if((getl(avg) - getl(test)) > amount)
        bmp->setpixel(x, y, avg, 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::cancelRemoveDust()
{
  Dialog::hideProgress();
  remove_dust->hide();
}

// colorize
void FX::showColorize()
{
  begin();
  doColorize();
}

void FX::doColorize()
{
  int x, y;

  Dialog::showProgress((float)bmp->h / 64);

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
      r = getr(Brush::main->color);
      g = getg(Brush::main->color);
      b = getb(Brush::main->color);
      Blend::rgbToHsv(r, g, b, &h, &s, &v);
      Blend::hsvToRgb(h, (sat * s) / (sat + s), v, &r, &g, &b);
      int c2 = makecol(r, g, b);
      bmp->setpixel(x, y, Blend::colorize(c1, c2, 0), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::showCorrect()
{
  begin();
  doCorrect();
}

// corrects uneven dye fading in photographs (especially when there is a severe
// color cast, such as when one of the dyes have faded almost completety)
//
// sometimes works better before or after the restore filter depending on
// the image
void FX::doCorrect()
{
  int x, y;

  Dialog::showProgress((float)bmp->h / 64);

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

      ra = MAX(MIN(ra, 255), 0);
      ga = MAX(MIN(ga, 255), 0);
      ba = MAX(MIN(ba, 255), 0);

      Blend::rgbToHsv(ra, ga, ba, &h, &s, &v);
      Blend::hsvToRgb(h, sat, val, &ra, &ga, &ba);

      bmp->setpixel(x, y, Blend::forceLuminance(makecol(ra, ga, ba), l), 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::showApplyPalette()
{
  apply_palette->show();
}

void FX::hideApplyPalette()
{
  begin();

  if(apply_palette_dither->value())
    doApplyPaletteDither();
  else
    doApplyPaletteNormal();

  apply_palette->hide();
}

void FX::doApplyPaletteNormal()
{
  int x, y;

  Dialog::showProgress((float)bmp->h / 64);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      bmp->setpixel(x, y, Palette::main->data[Palette::main->lookup[c & 0xFFFFFF]], 0);
    }

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::doApplyPaletteDither()
{
  int x, y;
  int i, j;
  Bitmap *bmp = Bitmap::main;

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

  Dialog::showProgress((float)bmp->h / 64);

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
        n[i] = MAX(MIN(n[i], 65535), 0);
      }

      const int r = unfix_gamma[n[0]];
      const int g = unfix_gamma[n[1]];
      const int b = unfix_gamma[n[2]];

      int c = Palette::main->data[Palette::main->lookup[makecol(r, g, b) & 0xFFFFFF]];
      bmp->setpixel_solid(x, y, makecol(getr(c), getg(c), getb(c)), 0);

      v[0] = fix_gamma[getr(*p)];
      v[1] = fix_gamma[getg(*p)];
      v[2] = fix_gamma[getb(*p)];

      p++;

      for(i = 0; i < 3; i++)
      {
        e[i] = n[i] - v[i];
        last[i] = (e[i] * 7) / 16;
        buf[i][x - 1] += (e[i] * 3) / 16;
        buf[i][x] += (e[i] * 5) / 16;
        buf[i][x + 1] += (e[i] * 1) / 16;
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

    if(!(y % 64))
    {
      Gui::getView()->draw_main(1);
      Dialog::updateProgress();
    }
  }

  Dialog::hideProgress();
}

void FX::cancelApplyPalette()
{
  Dialog::hideProgress();
  apply_palette->hide();
}

