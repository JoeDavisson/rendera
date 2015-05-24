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

#include <FL/Fl_Box.H>
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
#include "Quantize.H"
#include "Separator.H"
#include "Undo.H"
#include "View.H"

namespace
{
  Bitmap *bmp;

  void pushUndo()
  {
    bmp = Project::bmp;
//    Undo::push(bmp->cl, bmp->ct, bmp->cw, bmp->ch);
    Undo::push();
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        rgba_type rgba = getRgba(*p);

        const int r = (rgba.r - r_low) * r_scale;
        const int g = (rgba.g - g_low) * g_scale;
        const int b = (rgba.b - b_low) * b_scale;

        *p = makeRgba(r, g, b, rgba.a);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        rgba_type rgba = getRgba(*p);

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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        rgba_type rgba = getRgba(*p);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        r = list_r[r] * scale;
        g = list_g[g] * scale;
        b = list_b[b] * scale;

        *p = makeRgba(r, g, b, rgba.a);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        rgba_type rgba = getRgba(*p);

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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        rgba_type rgba = getRgba(*p);

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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
          continue;

        const int temp = s;
        s = list_s[s] * scale;

        if(s < temp)
          s = temp;

        Blend::hsvToRgb(h, s, v, &r, &g, &b);
        *p = Blend::trans(*p, Blend::keepLum(makeRgba(r, g, b, rgba.a), l), 255 - s);
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
    CheckBox *preserve_lum;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void apply(int amount)
  {
    const int hh = amount * 4.277;
    const bool keep_lum = Items::preserve_lum->value();

    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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

        if(h >= 1536)
          h -= 1536;

        Blend::hsvToRgb(h, s, v, &r, &g, &b);
        c = makeRgba(r, g, b, rgba.a);

        if(keep_lum)
          *p = Blend::keepLum(c, l);
        else
          *p = c;
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
    Items::angle = new InputInt(Items::dialog, 0, y1, 96, 24, "Angle:", 0, -359, 359);
    y1 += 24 + 8;
    Items::angle->maximum_size(4);
    Items::angle->value("60");
    Items::angle->center();
    Items::preserve_lum = new CheckBox(Items::dialog, 0, y1, 16, 16, "Preserve Luminance", 0);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        *p = Blend::invert(*p, 0, 0);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        const rgba_type rgba = getRgba(*p);
        *p = makeRgba(rgba.r, rgba.g, rgba.b, 255 - rgba.a);
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
namespace AutoCorrect
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        const rgba_type rgba = getRgba(*p);

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
    const double ra = (256.0f / (256 - rr)) / std::sqrt(256.0f / (rr + 1));
    const double ga = (256.0f / (256 - gg)) / std::sqrt(256.0f / (gg + 1));
    const double ba = (256.0f / (256 - bb)) / std::sqrt(256.0f / (bb + 1));

    // begin restore
    Gui::showProgress(bmp->h);

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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

    Items::dialog = new DialogWindow(384, 0, "Auto-Correct");
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 32, "Automatically restores color/contrast to a bad image.");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(12);
    y1 += 32;
    Items::normalize = new CheckBox(Items::dialog, 0, y1, 16, 16, "Normalize First", 0);
    y1 += 16 + 8;
    Items::normalize->value(1);
    Items::normalize->center();
    Items::invert = new CheckBox(Items::dialog, 0, y1, 16, 16, "Invert First", 0);
    Items::invert->center();
    y1 += 16 + 8;
    Items::preserve_lum = new CheckBox(Items::dialog, 8, y1, 16, 16, "Preserve Luminance", 0);
    y1 += 16 + 8;
    Items::preserve_lum->center();
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

// implementation of Geoff Daniell's photo restoration algorithm
// see original python source in redist directory
namespace Restore
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Box *box;
    InputInt *contrast;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  // for storing floating-point adjustment factors etc
  class Triplet
  {
  public:
    Triplet(float x, float y, float z)
    {
      set(x, y, z);
    }

    ~Triplet()
    {
    }

    void set(float x, float y, float z)
    {
      value[0] = x;
      value[1] = y;
      value[2] = z;
    }

    void copy(Triplet src)
    {
      value[0] = src.value[0];
      value[1] = src.value[1];
      value[2] = src.value[2];
    }

    float value[3];
  };

  // this stores one channel from a palette into the destination buffer
  void getSlice(const Palette *pal, const int channel, int *dest)
  {
    switch(channel)
    {
      case 0:
        for(int i = 0; i < 256; i++)
          dest[i] = getr(pal->data[i]);
        break;
      case 1:
        for(int i = 0; i < 256; i++)
          dest[i] = getg(pal->data[i]);
        break;
      case 2:
        for(int i = 0; i < 256; i++)
          dest[i] = getb(pal->data[i]);
        break;
    }
  }

  // scale/gamma
  inline int levels_value(const int &value,
                          const int &in_min, const int &in_max,
                          const float &gamma,
                          const int &out_min, const int &out_max)
  {
    float v = (float)(value - in_min) / ((in_max - in_min) + 1);
    v = powf(v, 1.0f / gamma);
    v = v * (out_max - out_min) + out_min;

    return clamp((int)v, 255);
  }

  // this emulates the levels function in GIMP
  void levels(Bitmap *temp,
              const int channel,
              const int in_min, const int in_max,
              const float gamma,
              const int out_min, const int out_max)
  {
    for(int y = temp->ct; y < temp->cb; y++)
    {
      int *p = temp->row[y] + temp->cl;
      for(int x = temp->cl; x < temp->cr; x++, p++)
      {
        const rgba_type rgba = getRgba(*p);
        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;

        switch(channel)
        {
          case 0:
            r = levels_value(r, in_min, in_max, gamma, out_min, out_max);
            break;
          case 1:
            g = levels_value(g, in_min, in_max, gamma, out_min, out_max);
            break;
          case 2:
            b = levels_value(b, in_min, in_max, gamma, out_min, out_max);
            break;
        }

        *p = makeRgb(r, g, b);
      }
    }
  }

  // find m through successive approximation
  float percentile(const int *c, const float &f)
  {
    int n = 0;

    for(int i = 0; i < 256; i++)
    {
      if(c[i] > 0)
        n++;
    }

    const int nf = (int)(n * f);
    int m = -1;
    int k = 0;

    while(k < nf)
    {
      m++;
      k = 0;

      for(int i = 0; i < 256; i++)
      {
        if(c[i] > 0 && c[i] <= m)
          k++;
      }
    }

    return (float)m;
  }

  struct color_range_type
  {
    float hi[3], lo[3];
  };

  struct color_range_type colormap(Bitmap *src,
                                   Triplet alpha, Triplet m,
                                   float top, float bot)
  {
    // duplicate image
    Bitmap small_copy(src->w, src->h);
    src->blit(&small_copy, 0, 0, 0, 0, src->w, src->h);

    // restore
    for(int i = 0; i < 3; i++)
      levels(&small_copy, i, 0, m.value[i], alpha.value[i], 0, 255);

    // get new colormap
    Palette pal;
    Quantize::fast(&small_copy, &pal);

    // return color range
    int slice[256];
    color_range_type color_range;

    for(int i = 0; i < 3; i++)
    {
      getSlice(&pal, i, slice);
      color_range.hi[i] = percentile(slice, top);
      color_range.lo[i] = percentile(slice, bot);
    }

    return color_range;
  }

  void apply(float contrast)
  {
    // make small copy
    Bitmap small_image(bmp->w / 10, bmp->h / 10);
    bmp->scale(&small_image);

    // get initial percentiles
    float top = 1.0f;
    float bot = 0.1f;

    struct color_range_type color_range;
    color_range = colormap(&small_image,
                           Triplet(1.0f, 1.0f, 1.0f),
                           Triplet(255, 255, 255),
                           top,
                           bot);

    // get initial m and alpha
    Triplet init_alpha(0, 0, 0);
    Triplet init_m(0, 0, 0);

    for(int i = 0; i < 3; i++)
    {
      float hi = color_range.hi[i];
      float lo = color_range.lo[i];

      init_m.value[i] = expf((logf(top) * logf(lo) - logf(bot) * logf(hi))
                            / (logf(top) - logf(bot)));
      init_alpha.value[i] = (logf(lo) - logf(init_m.value[i])) / logf(bot);
    }

    // iterate to get correct m and alpha
    Triplet alpha(0, 0, 0);
    Triplet m(0, 0, 0);
    alpha.copy(init_alpha);
    m.copy(init_m);
    Triplet d_alpha(1.0f, 1.0f, 1.0f);
    Triplet d_m(0, 0, 0);

    int iter = 0;

    while(std::max(std::abs(d_alpha.value[0]),
                   std::max(std::abs(d_alpha.value[1]),
                            std::abs(d_alpha.value[2]))) > 0.02f)
    {
      iter++;
      if(iter == 10)
        break;

      for(int i = 0; i < 3; i++)
      {
        if(alpha.value[i] < 0.1f || alpha.value[i] > 10.0f)
        {
          Dialog::message("Error", "The image has deteriorated too far to restore.");
          return;
        }
      }

      color_range = colormap(&small_image, alpha, m, top, bot);
        
      for(int i = 0; i < 3; i++)
      {
        float hi = color_range.hi[i];
        float lo = color_range.lo[i];

        d_alpha.value[i] = alpha.value[i] * alpha.value[i]
                           * (lo / (255.0f * bot) - 1) / logf(bot);
        d_m.value[i] = alpha.value[i] * (hi - 255.0f * top);

        if(std::abs(d_alpha.value[i]) > 0.2f * alpha.value[i])
        {
          d_alpha.value[i] = 0.2f * alpha.value[i] * d_alpha.value[i]
                                    / std::abs(d_alpha.value[i]);
        }

        alpha.value[i] += d_alpha.value[i];
        m.value[i] += d_m.value[i];
      }
    }

    // if loop failed to converge use initial values
    if(iter == 10)
    {
      Dialog::message("Error", "The image has deteriorated badly, the restoration\n is probably poor but may be an improvement on the original.");
      alpha.copy(init_alpha);
      m.copy(init_m);
    }

    // create restored image
    for(int i = 0; i < 3; i++)
      levels(&small_image, i, 0, m.value[i], alpha.value[i], 0, 255);

    // get color map of restored image
    Palette pal;
    Quantize::pca(&small_image, &pal, 256);

    float newc[3][256];

    for(int i = 0; i < 3; i++)
    {
      int temp[256];

      getSlice(&pal, i, temp);

      for(int j = 0; j < 256; j++)
        newc[i][j] = (float)temp[j];
    }

    // average color
    float av[256];

    for(int i = 0; i < 256; i++)
      av[i] = (newc[0][i] + newc[1][i] + newc[2][i]) / (3 * 255);

    // tweak restoration
    Triplet lambda(1.0f, 1.0f, 1.0f);
    Triplet sigma(1.0f, 1.0f, 1.0f);

    bool ok = true;

    for(int i = 0; i < 3; i++)
    {
      float lambda_c = 1.0f;
      float d_lambda_c = 1.0f;
      float l[256]; 

      for(int j = 0; j < 256; j++)
        l[j] = logf((newc[i][j] + 1.0f) / 255);

      iter = 0;
      float sig = 0;

      while(std::abs(d_lambda_c) > 1e-4f)
      {
        iter++;
        if(iter >= 20)
          break;

        float pc[256];
        float ppl = 0;
        float ppll = 0;
        float pp = 0;
        float ap = 0;
        float apl = 0;
        float apll = 0;

        for(int j = 0; j < 256; j++)
        {
          pc[j] = powf(newc[i][j] / 255, lambda_c);
          ppl += pc[j] * pc[j] * l[j];
          ppll += pc[j] * pc[j] * l[j] * l[j];
          pp += pc[j] * pc[j];
          ap += av[j] * pc[j];
          apl += av[j] * pc[j] * l[j];
          apll += av[j] * pc[j] * l[j] * l[j];
        }

        sig = ap / pp;
        d_lambda_c = -(apl - sig * ppl) / (apll - 2 * sig * ppll);
        lambda_c += d_lambda_c;
      }   

      lambda.value[i] = lambda_c;
      sigma.value[i] = sig;

      if(iter == 20)
        ok = false;
    }

    // if loop fails to converge use default values
    if(!ok)
    {
      lambda.set(1.0f, 1.0f, 1.0f);
      sigma.set(1.0f, 1.0f, 1.0f);
    }

    float smin = std::min(sigma.value[0],
                          std::min(sigma.value[1], sigma.value[2]));

    // adjust parameters
    for(int i = 0; i < 3; i++)
    {
      alpha.value[i] /= lambda.value[i];
      sigma.value[i] /= smin;
      m.value[i] /= powf(sigma.value[i], alpha.value[i]);
    }

    // implement degree of restoration and restore
    contrast /= 100;

    for(int i = 0; i < 3; i++)
    {
      alpha.value[i] = 1.0f - contrast * (1.0f - alpha.value[i]);
      m.value[i] = 255.0f - contrast * (255.0f - m.value[i]);
      levels(bmp, i, 0, m.value[i], alpha.value[i], 0, 255);
    }

    // correct side absorptions
    Gui::hideProgress();
  }

  void close()
  {
    Items::dialog->hide();
    pushUndo();

    apply(atoi(Items::contrast->value()));
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
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 48, "Restores faded color photographs and slides.\n (Algorithm by Geoff Daniell.)");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(12);
    y1 += 48;
    Items::contrast = new InputInt(Items::dialog, 0, y1, 96, 24, "Contrast:", 0, 1, 150);
    y1 += 24 + 8;
    Items::contrast->maximum_size(4);
    Items::contrast->value("100");
    Items::contrast->center();
//    y1 += 16 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end();
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        rgba_type rgba = getRgba(*p);

        int r = rgba.r;
        int g = rgba.g;
        int b = rgba.b;
        int l = getl(*p);

        // correction matrix
        int ra = r;
        int ga = (r * 4 + g * 8 + b * 1) / 13;
        int ba = (r * 2 + g * 4 + b * 8) / 14;

        ra = clamp(ra, 255);
        ga = clamp(ga, 255);
        ba = clamp(ba, 255);

        *p = Blend::keepLum(makeRgba(ra, ga, ba, rgba.a), l);
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
      for(int x = bmp->cl + 1; x <= bmp->cr - 1; x++, p++)
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
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 32, "Removes dust from scanned images.");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(12);
    y1 += 32;
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount:", 0, 1, 10);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
      {
        const int l = getl(*p);

        *p = makeRgba(l, l, l, geta(*p));
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
        for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
        for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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

    Items::dialog = new DialogWindow(256, 0, "Apply Palette");
    Items::mode = new Fl_Choice(96, y1, 128, 24, "Dither:");
    Items::mode->tooltip("Dither");
    Items::mode->textsize(10);
    Items::mode->add("Threshold");
    Items::mode->add("Floyd-Steinberg");
    Items::mode->add("Jarvis, Judice and Ninke");
    Items::mode->add("Stucki");
    Items::mode->add("Atkinson");
    Items::mode->add("Sierra");
    Items::mode->value(0);
    y1 += 24 + 8;
    Items::gamma = new CheckBox(Items::dialog, 0, y1, 16, 16, "Gamma Correction", 0);
    Items::gamma->center();
    y1 += 16 + 8;
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
        for(int x = bmp->cl; x <= bmp->cr; x++, p++)
        {
          const int c = map->getpixel(x, y);

          *p = Blend::trans(*p, makeRgb(0, 0, 0), 255 - c);
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
    Items::detail = new InputInt(Items::dialog, 0, y1, 96, 24, "Detail:", 0, 1, 50000);
    y1 += 24 + 8;
    Items::detail->value("5000");
    Items::detail->center();
    Items::edge = new InputInt(Items::dialog, 0, y1, 96, 24, "Edge Detect:", 0, 1, 50);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
      }

      if(Gui::updateProgress(y) < 0)
        return;
    }

    Gui::showProgress(bmp->h);

    // y direction
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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

    Items::dialog = new DialogWindow(256, 0, "Gaussian Blur");
    Items::radius = new InputInt(Items::dialog, 0, y1, 96, 24, "Radius:", 0, 1, 100);
    y1 += 24 + 8;
    Items::radius->value("1");
    Items::radius->center();
    Items::blend = new InputInt(Items::dialog, 0, y1, 96, 24, "Blend:", 0, 0, 100);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount:", 0, 1, 100);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
    Items::radius = new InputInt(Items::dialog, 0, y1, 96, 24, "Radius:", 0, 1, 100);
    y1 += 24 + 8;
    Items::radius->value("1");
    Items::radius->center();
    Items::amount = new InputFloat(Items::dialog, 0, y1, 96, 24, "Amount:", 0, 0.1, 10.0);
    y1 += 24 + 8;
    Items::amount->value("1.5");
    Items::amount->center();
    Items::threshold = new InputInt(Items::dialog, 0, y1, 72, 24, "Threshold:", 0, 0, 255);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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
          lum = clamp(lum, 255);

          const int c = bmp->getpixel(x, y);

          *p = Blend::trans(c, Blend::keepLum(c, lum), 255 - amount * 2.55);
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

          r = clamp(r, 255);
          g = clamp(g, 255);
          b = clamp(b, 255);

          const int c = bmp->getpixel(x, y);

          *p = Blend::trans(c, makeRgba(r, g, b, geta(c)), 255 - amount * 2.55);
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
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount:", 0, 1, 100);
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
      for(int x = bmp->cl; x <= bmp->cr; x++, p++)
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

    Items::dialog = new DialogWindow(256, 0, "Artistic");
    Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount:", 0, 1, 10);
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

        Math::forwardFFT(&real_row[0], &imag_row[0], w);

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

        Math::forwardFFT(&real_col[0], &imag_col[0], h);

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

        Math::inverseFFT(&real_row[0], &imag_row[0], w);

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

        Math::inverseFFT(&real_col[0], &imag_col[0], h);

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

void FX::init()
{
  RotateHue::init();
  AutoCorrect::init();
  Restore::init();
  RemoveDust::init();
  ApplyPalette::init();
  StainedGlass::init();
  GaussianBlur::init();
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

void FX::autoCorrect()
{
  AutoCorrect::begin();
}

void FX::correctionMatrix()
{
  CorrectionMatrix::begin();
}

void FX::restore()
{
  Restore::begin();
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

void FX::gaussianBlur()
{
  GaussianBlur::begin();
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

void FX::forwardFFT()
{
  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  if(Math::isPowerOfTwo(w) && Math::isPowerOfTwo(h))
  {
    ForwardFFT::begin();
  }
  else
  {
    Dialog::message("Error", "Image dimensions must be powers of two.");
  }
}

void FX::inverseFFT()
{
  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  if(Math::isPowerOfTwo(w) && Math::isPowerOfTwo(h))
  {
    InverseFFT::begin();
  }
  else
  {
    Dialog::message("Error", "Image dimensions must be powers of two.");
  }
}

