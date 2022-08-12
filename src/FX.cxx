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
    Project::undo->push();
  }

  void drawPreview(Bitmap *dest)
  {
    Bitmap *src = Project::bmp;
    int overscroll = Project::overscroll;

    if(src->w >= src->h)
    {
      float aspect = (float)(src->h - overscroll * 2) / (src->w - overscroll * 2);
      int height = (float)dest->w * aspect;

      dest->clear(getFltkColor(FL_BACKGROUND2_COLOR));
      src->fastStretch(dest, overscroll, overscroll,
                      src->w - overscroll * 2, src->h - overscroll * 2,
                      0, (dest->h - height) / 2, dest->w, height, false);
      dest->rect(0, (dest->h - height) / 2, dest->w, ((dest->h - height) / 2) + height, makeRgb(0, 0, 0), 0);
    }
    else
    {
      float aspect = (float)(src->w - overscroll * 2) / (src->h - overscroll * 2);
      int width = (float)dest->h * aspect;

      dest->clear(getFltkColor(FL_BACKGROUND2_COLOR));
      src->fastStretch(dest, overscroll, overscroll,
                      src->w - overscroll * 2, src->h - overscroll * 2,
                      (dest->w - width) / 2, 0, width, dest->w, false);
      dest->rect((dest->w - width) / 2, 0, ((dest->w - width) / 2) + width, dest->w, makeRgb(0, 0, 0), 0);
    }

    dest->rect(0, 0, dest->w - 1, dest->h - 1, makeRgb(0, 0, 0), 128);
  }
}

namespace Test
{
  void apply()
  {
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;
        const int l = getl(makeRgb(r, g, b));

        *p = makeRgb((r | l) & 255, (g | l) & 255, (b | l) & 255);

        p++;
      }
    }
  }

  void begin()
  {
    pushUndo();
    apply();
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
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

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

        p++;
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
        rgba_type rgba = getRgba(*p);

        const int r = (rgba.r - r_low) * r_scale;
        const int g = (rgba.g - g_low) * g_scale;
        const int b = (rgba.b - b_low) * b_scale;

        *p = makeRgba(r, g, b, rgba.a);
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
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        list_r[r]++;
        list_g[g]++;
        list_b[b]++;
        p++;
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
        rgba_type rgba = getRgba(*p);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        r = list_r[r] * scale;
        g = list_g[g] * scale;
        b = list_b[b] * scale;

        *p = makeRgba(r, g, b, rgba.a);
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
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        list_r[r]++;
        list_g[g]++;
        list_b[b]++;

        p++;
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
        rgba_type rgba = getRgba(*p);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        const int ra = list_r[r] * scale;
        const int ga = list_g[g] * scale;
        const int ba = list_b[b] * scale;

        r = ((ra * rr) + (r * (255 - rr))) / 255;
        g = ((ga * gg) + (g * (255 - gg))) / 255;
        b = ((ba * bb) + (b * (255 - bb))) / 255;

        r = clamp(r, 255);
        g = clamp(g, 255);
        b = clamp(b, 255);

        *p = makeRgba(r, g, b, rgba.a);
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

namespace Saturate
{
  void apply()
  {
    std::vector<int> list_s(256, 0);

    const int size = bmp->cw * bmp->ch;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

        const int r = rgba.r;
        const int g = rgba.g;
        const int b = rgba.b;

        int h, s, v;
        Blend::rgbToHsv(r, g, b, &h, &s, &v);

        list_s[s]++;
        p++;
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
        rgba_type rgba = getRgba(*p);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        const int l = getl(*p);
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

        const int c1 = *p;
        *p = Blend::colorize(c1, Blend::keepLum(makeRgba(r, g, b, rgba.a), l), 255 - s);
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
    Widget *preview;
    Widget *hue;
    Fl_Repeat_Button *inc_hue;
    Fl_Repeat_Button *dec_hue;
    CheckBox *preserve_lum;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void render(Bitmap *dest, bool show_progress)
  {
    const int hh = (((Items::hue->var + 180) % 360) * 6) * .712;
    const bool keep_lum = Items::preserve_lum->value();

    drawPreview(Items::preview->bitmap);

    if(show_progress)
      Gui::showProgress(bmp->h);

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

  void setHue()
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
    sprintf(degree, "%d\xB0",
             (int)(hx - 180));

    Items::hue->copy_label(degree);
    render(Items::preview->bitmap, false);
    Items::preview->redraw();
  }

  void incHue()
  {
    Items::hue->var++;
//    Items::hue->var %= 360;
    if(Items::hue->var > 359)
      Items::hue->var = 359;
    setHue();
  }

  void decHue()
  {
    Items::hue->var--;
//    Items::hue->var %= 360;

    if(Items::hue->var < 0)
      Items::hue->var = 0;
    setHue();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();
    render(Project::bmp, true);
  }

  void quit()
  {
    Gui::hideProgress();
    Items::dialog->hide();
  }

  void begin()
  {
    bmp = Project::bmp;
    Items::hue->var = 180;
    drawPreview(Items::preview->bitmap);
    Items::preview->redraw();
    Items::hue->do_callback();
    Items::dialog->show();
  }

  void init()
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
        *p = Blend::invert(*p, 0, 0);
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
        const rgba_type rgba = getRgba(*p);
        *p = makeRgba(rgba.r, rgba.g, rgba.b, 255 - rgba.a);
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

// tries to automatically fix color/contrast problems
namespace Restore
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Box *box;
    CheckBox *normalize;
    CheckBox *invert;
    CheckBox *preserve_lum;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply()
  {
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
}

// Attempes to correct side-absorptions in color photographs.
// Sometimes works better before or after the restore filter depending on
// the image.
namespace SideAbsorptions
{
  void apply()
  {
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;
        int h, s, v, old_s;

        Blend::rgbToHsv(r, g, b, &h, &old_s, &v);

        int ra = r;
        int ga = (r * 4 + g * 8 + b * 1) / 13;
        int ba = (r * 2 + g * 4 + b * 8) / 14;

        ra = clamp(ra, 255);
        ga = clamp(ga, 255);
        ba = clamp(ba, 255);

        Blend::rgbToHsv(ra, ga, ba, &h, &s, &v);
        Blend::hsvToRgb(h, old_s, v, &ra, &ga, &ba);

        *p = makeRgba(ra, ga, ba, rgba.a);
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

namespace RemoveDust
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Box *box;
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
        const int test = *p;
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

    Items::dialog = new DialogWindow(384, 0, "Remove Dust");
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 32, "Cleans up scanned images.");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
//    Items::box->labelsize(12);
    y1 += 32;
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount (1-10)", 0, 1, 10);
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
        const int l = getl(*p);

        *p = makeRgba(l, l, l, geta(*p));
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
        rgba_type rgba = getRgba(*p);

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

        *p = Blend::colorize(*p, makeRgba(r, g, b, rgba.a), 0);
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

namespace PaletteColors
{
  void apply()
  {
    Palette *pal = Project::palette.get();

    Gui::showProgress(bmp->h);

    const float inc = 1.0 / (bmp->cw * bmp->ch);

    float freq[256];

    for(int i = 0; i < pal->max; i++)
      freq[i] = 0;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p);

        rgba.r = (rgba.r >> 3);
        rgba.g = (rgba.g >> 3);
        rgba.b = (rgba.b >> 3);

        rgba.r = (rgba.r << 3) + rgba.r;
        rgba.g = (rgba.g << 3) + rgba.g;
        rgba.b = (rgba.b << 3) + rgba.b;

        int c = makeRgb(rgba.r, rgba.g, rgba.b);

        freq[pal->lookup(c)] += inc;
        p++;
      }
    }

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int lowest = 999999;
        int use1 = 0;
        int use2 = 0;

        for(int i = 0; i < pal->max; i++)
        {
          int d = diff24(*p, pal->data[i]);

          if(d < lowest)
          {
            lowest = d;
            use1 = i;
          }
        }

        lowest = 999999;

        for(int i = 0; i < pal->max; i++)
        {
          if(i == use1)
            continue;

          int d = diff24(*p, pal->data[i]);

          if(d < lowest)
          {
            lowest = d;
            use2 = i;
          }
        }

        int c1 = pal->data[use1];
        int c2 = pal->data[use2];

        rgba_type rgba1 = getRgba(c1);
        rgba_type rgba2 = getRgba(c2);

        const float mul = 1.0f / (freq[use1] + freq[use2]);

        rgba1.r = (freq[use1] * rgba1.r + freq[use2] * rgba2.r) * mul; 
        rgba1.g = (freq[use1] * rgba1.g + freq[use2] * rgba2.g) * mul; 
        rgba1.b = (freq[use1] * rgba1.b + freq[use2] * rgba2.b) * mul; 

        int l = getl(*p);

        *p = Blend::keepLum(makeRgb(rgba1.r, rgba1.g, rgba1.b), l);
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

namespace DitherImage
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *mode;
    CheckBox *gamma;
    CheckBox *lum_only;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  enum
  {
    THRESHOLD,
    FLOYD,
    JARVIS,
    STUCKI,
    ATKINSON,
    SIERRA
  };
 
  namespace Threshold
  {
    int matrix[3][5] =
    {
      { 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0 },
    };

    const int div = 1;
  }

  namespace Floyd
  {
    int matrix[3][5] =
    {
      { 0, 0, 0, 7, 0 },
      { 0, 3, 5, 1, 0 },
      { 0, 0, 0, 0, 0 },
    };

    const int div = 16;
  }

  namespace Jarvis
  {
    int matrix[3][5] =
    {
      { 0, 0, 0, 7, 5 },
      { 3, 5, 7, 5, 3 },
      { 1, 3, 5, 3, 1 }
    };

    const int div = 48;
  }

  namespace Stucki
  {
    int matrix[3][5] =
    {
      { 0, 0, 0, 8, 4 },
      { 2, 4, 8, 4, 2 },
      { 1, 2, 4, 2, 1 }
    };

    const int div = 42;
  }

  namespace Atkinson
  {
    int matrix[3][5] =
    {
      { 0, 0, 0, 1, 1 },
      { 0, 1, 1, 1, 0 },
      { 0, 0, 1, 0, 0 }
    };

    const int div = 8;
  }

  namespace Sierra
  {
    int matrix[3][5] =
    {
      { 0, 0, 0, 5, 3 },
      { 2, 4, 5, 4, 2 },
      { 0, 2, 3, 2, 0 }
    };

    const int div = 32;
  }

  void apply(int mode)
  {
    int (*matrix)[5] = Threshold::matrix;
    int w = 5, h = 3;
    int div = 1;

    switch(mode)
    {
      case THRESHOLD:
        matrix = Threshold::matrix;
        div = Threshold::div;
        break;
      case FLOYD:
        matrix = Floyd::matrix;
        div = Floyd::div;
        break;
      case JARVIS:
        matrix = Jarvis::matrix;
        div = Jarvis::div;
        break;
      case STUCKI:
        matrix = Stucki::matrix;
        div = Stucki::div;
        break;
      case ATKINSON:
        matrix = Atkinson::matrix;
        div = Atkinson::div;
        break;
      case SIERRA:
        matrix = Sierra::matrix;
        div = Sierra::div;
        break;
      default:
        matrix = Threshold::matrix;
        div = Threshold::div;
        break;
    }

    Bitmap *bmp = Project::bmp;
    const bool fix_gamma = Items::gamma->value();
    const bool lum_only = Items::lum_only->value();

    Gui::showProgress(bmp->h);

    if(lum_only)
    {
      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        int *p = bmp->row[y] + bmp->cl;

        for(int x = bmp->cl; x <= bmp->cr; x++)
        {
          const int alpha = geta(*p);
          const int old_l = getl(*p);

          const int pal_index =
            (int)Project::palette->lookup(Blend::keepLum(*p, old_l));
          const int c = Project::palette->data[pal_index];

          struct rgba_type rgba = getRgba(c);
          *p = makeRgba(rgba.r, rgba.g, rgba.b, alpha);

          const int new_l = getl(*p);
          int el;

          if(fix_gamma)
          {
            el = Gamma::fix(old_l) - Gamma::fix(new_l);
          }
          else
          {
            el = old_l - new_l;
          }

          for(int j = 0; j < h; j++)
          {
            for(int i = 0; i < w; i++)
            {
              if(matrix[j][i] > 0)
              {
                int c = bmp->getpixel(x - w / 2 + i, y + j);
                int l = getl(c);

                if(fix_gamma)
                {
                  l = Gamma::fix(l); 
                }

                l += (el * matrix[j][i]) / div;

                if(fix_gamma)
                {
                  l = Gamma::unfix(clamp(l, 65535));
                }
                else
                {
                  l = clamp(l, 255);
                }

                rgba_type rgba = getRgba(Blend::keepLum(c, l));

                bmp->setpixelSolid(x - w / 2 + i, y + j,
                                 makeRgba(rgba.r, rgba.g, rgba.b, rgba.a), 0);
              }  
            }
          }

          p++;
        }

        if(Gui::updateProgress(y) < 0)
          return;
      }
    }
    else
    {
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
          *p = makeRgba(rgba.r, rgba.g, rgba.b, alpha);

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
                  r = Gamma::unfix(clamp(r, 65535));
                  g = Gamma::unfix(clamp(g, 65535));
                  b = Gamma::unfix(clamp(b, 65535));
                }
                else
                {
                  r = clamp(r, 255);
                  g = clamp(g, 255);
                  b = clamp(b, 255);
                }

                bmp->setpixelSolid(x - w / 2 + i, y + j,
                                 makeRgba(r, g, b, rgba.a), 0);
              }  
            }

          }

          p++;
        }

        if(Gui::updateProgress(y) < 0)
          return;
      }
    }

    Gui::hideProgress();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();
    apply(Items::mode->value());
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
    Items::dialog->redraw();
  }

  void init()
  {
    int y1 = 8;
    int ww = 0;
    int hh = 0;

    Items::dialog = new DialogWindow(256, 0, "Apply Palette to Image");
    Items::mode = new Fl_Choice(0, y1, 128, 24, "Dither:");
    Items::mode->tooltip("Dither");
    Items::mode->textsize(10);
    Items::mode->add("No Dithering");
    Items::mode->add("Floyd-Steinberg");
    Items::mode->add("Jarvis-Judice-Ninke");
    Items::mode->add("Stucki");
    Items::mode->add("Atkinson");
    Items::mode->add("Sierra");
    Items::mode->value(0);
    Items::mode->measure_label(ww, hh);
    Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
    y1 += 24 + 8;
    Items::gamma = new CheckBox(Items::dialog, 0, y1, 16, 16, "Gamma Correction", 0);
    Items::gamma->center();
    y1 += 16 + 8;
    Items::lum_only = new CheckBox(Items::dialog, 0, y1, 16, 16, "Luminosity Based", 0);
    Items::lum_only->center();
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

  inline int isEdge(Bitmap *b, const int x, const int y, const int div)
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

  inline int isSegmentEdge(Bitmap *b, const int x, const int y)
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
        seedx[i] = ExtraMath::rnd() % bmp->w; 
        seedy[i] = ExtraMath::rnd() % bmp->h; 
      }
      else
      {
        seedx[i] = ExtraMath::rnd() % bmp->w; 
        seedy[i] = ExtraMath::rnd() % bmp->h; 
  
        int count = 0;

        do
        {
          seedx[i] = ExtraMath::rnd() % bmp->w; 
          seedy[i] = ExtraMath::rnd() % bmp->h; 
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
//      for(int y = bmp->ct * 4; y <= bmp->cb * 4; y++)
//      {
//        for(int x = bmp->cl * 4; x <= bmp->cr * 4; x++)
//        {
      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        for(int x = bmp->cl; x <= bmp->cr; x++)
        {
          if(isSegmentEdge(bmp, x, y))
            map->setpixel(x, y, 1);
//            map->setpixelAA(x, y, 255);
        }
      }

      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        int *p = bmp->row[y] + bmp->cl;

        for(int x = bmp->cl; x <= bmp->cr; x++)
        {
          const int c = map->getpixel(x, y);

//          *p = Blend::trans(*p, makeRgb(0, 0, 0), 255 - c);
          if(c)
            *p = Blend::trans(*p, makeRgb(0, 0, 0), 160);
//            *p = makeRgb(0, 0, 0);

          p++;
        }
      }
    }

    Gui::hideProgress();
  }

  void close()
  {
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
    Items::detail = new InputInt(Items::dialog, 0, y1, 96, 24, "Detail (1-50000)", 0, 1, 50000);
    y1 += 24 + 8;
    Items::detail->value("5000");
    Items::detail->center();
    Items::edge = new InputInt(Items::dialog, 0, y1, 96, 24, "Edge Detect (1-50)", 0, 1, 50);
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

namespace GaussianBlur
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *radius;
    InputInt *blend;
    Fl_Choice *mode;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int radius, int blend)
  {
    radius = (radius + 1) * 2;

    int mode = Items::mode->value();

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
          const int mul = kernel[i];
          rgba_type rgba = getRgba(bmp->getpixel(x - radius / 2 + i, y));

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
          const int mul = kernel[i];
          rgba_type rgba = getRgba(temp.getpixel(x - bmp->cl,
                                                 y - radius / 2 + i - bmp->ct));

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

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::hideProgress();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::radius->value()), 255 - atoi(Items::blend->value()) * 2.55);
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
    int ww = 0;
    int hh = 0;

    Items::dialog = new DialogWindow(256, 0, "Gaussian Blur");
    Items::radius = new InputInt(Items::dialog, 0, y1, 96, 24, "Radius (1-100)", 0, 1, 100);
    y1 += 24 + 8;
    Items::radius->value("1");
    Items::radius->center();
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
}

namespace Bloom
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *radius;
    InputInt *blend;
    InputInt *threshold;
//    Fl_Choice *mode;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int radius, int threshold, int blend)
  {
    radius = (radius + 1) * 2;

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
          const int mul = kernel[i];
          rgba_type rgba = getRgba(bmp->getpixel(x - radius / 2 + i, y));

          if(getlUnpacked(rgba.r, rgba.g, rgba.b) > threshold)
          {
            rr += Gamma::fix(rgba.r) * mul;
            gg += Gamma::fix(rgba.g) * mul;
            bb += Gamma::fix(rgba.b) * mul;
            aa += rgba.a * mul;
          }
        }

        rr = Gamma::unfix(rr / div);
        gg = Gamma::unfix(gg / div);
        bb = Gamma::unfix(bb / div);
        aa /= div;

        *p = makeRgba(rr, gg, bb, aa);
        p++;
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
          const int mul = kernel[i];
          rgba_type rgba = getRgba(temp.getpixel(x - bmp->cl,
                                                 y - radius / 2 + i - bmp->ct));

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

        *p = Blend::lighten(*p, c3, blend);
        p++;
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
    apply(atoi(Items::radius->value()), atoi(Items::threshold->value()), 255 - atoi(Items::blend->value()) * 2.55);
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

    Items::dialog = new DialogWindow(256, 0, "Bloom");
    Items::radius = new InputInt(Items::dialog, 0, y1, 96, 24, "Radius (0-100)", 0, 1, 100);
    Items::radius->value("16");
    Items::radius->center();
    y1 += 24 + 8;
    Items::threshold = new InputInt(Items::dialog, 0, y1, 96, 24, "Threshold (0-255)", 0, 0, 255);
    Items::threshold->value("128");
    Items::threshold->center();
    y1 += 24 + 8;
    Items::blend = new InputInt(Items::dialog, 0, y1, 96, 24, "Blend %", 0, 0, 100);
    Items::blend->value("25");
    Items::blend->center();
    y1 += 24 + 8;
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

        lum = clamp(lum, 255);
        *p = Blend::trans(c, Blend::keepLum(c, lum), 255 - amount * 2.55);
        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

    Gui::hideProgress();
  }

  void close()
  {
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
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount %", 0, 0, 100);
    y1 += 24 + 8;
    Items::amount->value("25");
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

  void close()
  {
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
}

namespace ConvolutionMatrix
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *mode;
    InputInt *amount;
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
    EMBOSS,
    EMBOSS_REVERSE,
//    TEST
  };
 
  void apply(int amount, int mode/*, bool lum_only*/)
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
      case EMBOSS_REVERSE:
        copyMatrix(FilterMatrix::emboss_reverse, matrix);
        div = 1;
        break;
//      case TEST:
//        copyMatrix(FilterMatrix::test, matrix);
//        div = 1;
//        break;
    }

    Bitmap temp(bmp->cw, bmp->ch);
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = temp.row[y - bmp->cl];

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int r = 0;
        int g = 0;
        int b = 0;

        for(int j = 0; j < 3; j++) 
        {
          for(int i = 0; i < 3; i++) 
          {
            const rgba_type rgba = getRgba(bmp->getpixel(x + i - 1, y + j - 1));

            r += rgba.r * matrix[i][j];
            g += rgba.g * matrix[i][j];
            b += rgba.b * matrix[i][j];
          }
        }

        r /= div;
        g /= div;
        b /= div;

        r = clamp(r, 255);
        g = clamp(g, 255);
        b = clamp(b, 255);

        const int c = bmp->getpixel(x, y);

        *p = Blend::trans(c, makeRgba(r, g, b, geta(c)), 255 - amount * 2.55);
        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

    Gui::hideProgress();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();
    apply(atoi(Items::amount->value()),
               Items::mode->value());
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
    int ww = 0;
    int hh = 0;

    Items::dialog = new DialogWindow(256, 0, "Box Filters");
    Items::mode = new Fl_Choice(0, y1, 128, 24, "Filter:");
    Items::mode->textsize(10);
    Items::mode->add("Box Blur");
    Items::mode->add("Gaussian Blur");
    Items::mode->add("Sharpen");
    Items::mode->add("Edge Detect");
    Items::mode->add("Emboss");
    Items::mode->add("Emboss (Inverse)");
    Items::mode->value(0);
    Items::mode->measure_label(ww, hh);
    Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
    y1 += 24 + 8;
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount %", 0, 0, 100);
    Items::amount->value("50");
    Items::amount->center();
    y1 += 24 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Sobel
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
    int div = 1;

    Bitmap temp(bmp->cw, bmp->ch);
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = temp.row[y - bmp->cl];

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int r1 = 0;
        int g1 = 0;
        int b1 = 0;
        int r2 = 0;
        int g2 = 0;
        int b2 = 0;

        for(int j = 0; j < 3; j++) 
        {
          for(int i = 0; i < 3; i++) 
          {
            const rgba_type rgba = getRgba(bmp->getpixel(x + i - 1, y + j - 1));

            r1 += rgba.r * FilterMatrix::sobel1[i][j];
            r2 += rgba.r * FilterMatrix::sobel2[i][j];
            g1 += rgba.g * FilterMatrix::sobel1[i][j];
            g2 += rgba.g * FilterMatrix::sobel2[i][j];
            b1 += rgba.b * FilterMatrix::sobel1[i][j];
            b2 += rgba.b * FilterMatrix::sobel2[i][j];
          }
        }

        int r = std::sqrt(r1 * r1 + r2 * r2);
        int g = std::sqrt(g1 * g1 + g2 * g2);
        int b = std::sqrt(b1 * b1 + b2 * b2);

        r /= div;
        g /= div;
        b /= div;

        r = clamp(r, 255);
        g = clamp(g, 255);
        b = clamp(b, 255);

        const int c = bmp->getpixel(x, y);

        *p = Blend::trans(c, makeRgba(r, g, b, geta(c)), 255 - amount * 2.55);
        p++;
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    temp.blit(bmp, 0, 0, bmp->cl, bmp->ct, temp.w, temp.h);

    Gui::hideProgress();
  }

  void close()
  {
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

    Items::dialog = new DialogWindow(256, 0, "Sobel Edge Detection");
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount %", 0, 0, 100);
    Items::amount->value("100");
    Items::amount->center();
    y1 += 24 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace Painting
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

        for(int j = -amount; j <= amount; j++) 
        {
          for(int i = -amount; i <= amount; i++) 
          {
            const int c3 = *p;
            const int c1 = bmp->getpixel(x + i, y + j);
            const int c2 = bmp->getpixel(x - i, y - j);

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

    Items::dialog = new DialogWindow(256, 0, "Painting");
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount (1-10)", 0, 1, 10);
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

namespace Randomize
{
  void apply()
  {
for(int j = 0; j <= 1; j++)
{
    // horizontal
    for(int y = bmp->ct; y <= bmp->cb; y += 1)
    {
      for(int x = bmp->cl + 1 + j; x <= bmp->cr; x += 2)
      {
        if((ExtraMath::rnd() & 1) == 1)
        {
          const int temp = bmp->getpixel(x, y);

          bmp->setpixel(x, y, bmp->getpixel(x - 1, y), 128);
          bmp->setpixel(x - 1, y, temp, 128);
        }
      }
    }

    // vertical
    for(int x = bmp->cl; x <= bmp->cr; x += 1)
    {
      for(int y = bmp->ct + 1 + j; y <= bmp->cb; y += 2)
      {
        if((ExtraMath::rnd() & 1) == 1)
        {
          const int temp = bmp->getpixel(x, y);

          bmp->setpixel(x, y, bmp->getpixel(x, y - 1), 128);
          bmp->setpixel(x, y - 1, temp, 128);
        }
      }
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
  DitherImage::init();
  StainedGlass::init();
  GaussianBlur::init();
  Bloom::init();
  Sharpen::init();
  UnsharpMask::init();
  ConvolutionMatrix::init();
  Sobel::init();
  Painting::init();
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

void FX::sideAbsorptions()
{
  SideAbsorptions::begin();
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

void FX::paletteColors()
{
  PaletteColors::begin();
}

void FX::ditherImage()
{
  DitherImage::begin();
}

void FX::stainedGlass()
{
  StainedGlass::begin();
}

void FX::gaussianBlur()
{
  GaussianBlur::begin();
}

void FX::bloom()
{
  Bloom::begin();
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

void FX::sobel()
{
  Sobel::begin();
}

void FX::painting()
{
  Painting::begin();
}

void FX::randomize()
{
  Randomize::begin();
}

