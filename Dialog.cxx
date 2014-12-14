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

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Dialog.H"
#include "File.H"
#include "Gui.H"
#include "InputInt.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "Quantize.H"
#include "Separator.H"
#include "View.H"
#include "Widget.H"

namespace About
{
  Fl_Double_Window *dialog;
  Widget *logo;
  Fl_Button *ok;

  void begin()
  {
    dialog->show();
  }

  void close()
  {
    dialog->hide();
  }

  void init()
  {
    int y1 = 8;
    int ww = 0, hh = 0;
    const char *credits = "Copyright (c) 2014 Joe Davisson.\nAll Rights Reserved.";

    dialog = new Fl_Double_Window(384, 0, "About");
    logo = new Widget(dialog, 32, y1, 320, 64,
                      credits, File::themePath("logo_large.png"), 0, 0, 0);
    logo->align(FL_ALIGN_BOTTOM);
    logo->measure_label(ww, hh);
    y1 += 64 + 8 + hh;
    Dialog::addOkButton(dialog, &ok, &y1);
    ok->callback((Fl_Callback *)close);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace JpegQuality
{
  Fl_Double_Window *dialog;
  InputInt *option_quality;
  Fl_Button *ok;

  void closeCallback(Fl_Widget *, void *)
  {
    // needed to prevent dialog from being closed by window manager
  }

  void begin()
  {
    dialog->show();

    while(true)
    {
      Fl_Widget *action = Fl::readqueue();

      if(!action)
      {
        Fl::wait();
      }
      else if(action == ok)
      {
        if(option_quality->limitValue(1, 100) == 0)
        {
          dialog->hide();
          break;
        }
      }
    }
  }

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "JPEG Quality");
    dialog->callback(closeCallback);
    option_quality = new InputInt(dialog, 0, y1, 72, 24, "Quality:", 0);
    option_quality->value("95");
    option_quality->center();
    y1 += 24 + 8;
    Dialog::addOkButton(dialog, &ok, &y1);
    dialog->set_modal();
    dialog->end();
  }
}

namespace PngOptions
{
  Fl_Double_Window *dialog;
  Fl_Check_Button *option_use_palette;
  Fl_Check_Button *option_use_alpha;
  InputInt *option_alpha_levels;
  Fl_Button *ok;

  void closeCallback(Fl_Widget *, void *)
  {
    // needed to prevent dialog from being closed by window manager
  }

  void begin()
  {
    dialog->show();

    while(true)
    {
      Fl_Widget *action = Fl::readqueue();

      if(!action)
      {
        Fl::wait();
      }
      else if(action == ok)
      {
        if(option_alpha_levels->limitValue(2, 16) == 0)
        {
          dialog->hide();
          break;
        }
      }
    }
  }

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "PNG Options");
    dialog->callback(closeCallback);
    option_alpha_levels = new InputInt(dialog, 0, y1, 72, 24, "Alpha Levels:", 0);
    option_alpha_levels->value("2");
    option_alpha_levels->center();
    y1 += 24 + 8;
    option_use_palette = new Fl_Check_Button(0, y1, 16, 16, "Use Current Palette");
    y1 += 16 + 8;
    Dialog::center(option_use_palette);
    option_use_alpha = new Fl_Check_Button(0, y1, 16, 16, "Save Alpha Channel");
    option_use_alpha->value(1);
    Dialog::center(option_use_alpha);
    y1 += 16 + 8;
    Dialog::addOkButton(dialog, &ok, &y1);
    dialog->set_modal();
    dialog->end();
  }
}
namespace Progress
{
  Fl_Double_Window *dialog;
  Fl_Progress *bar;
  float value;
  float step;

  void init()
  {
    dialog = new Fl_Double_Window(272, 80, "Progress");
    bar = new Fl_Progress(8, 8, 256, 64);
    bar->minimum(0);
    bar->maximum(100);
    bar->color(0x40404000);
    bar->selection_color(0x88CC8800);
    bar->labelcolor(0xFFFFFF00);
    dialog->set_modal();
    dialog->end();
  }
}

namespace NewImage
{
  Fl_Double_Window *dialog;
  InputInt *option_width;
  InputInt *option_height;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    option_width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    option_height->value(s);
    dialog->show();
  }

  void close()
  {
    if(option_width->limitValue(1, 10000) < 0)
      return;

    if(option_height->limitValue(1, 10000) < 0)
      return;

    dialog->hide();

    Project::newImage(atoi(option_width->value()),
                      atoi(option_height->value()));

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(true);
  }

  void quit()
  {
    dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "New Image");
    option_width = new InputInt(dialog, 0, y1, 72, 24, "Width:", 0);
    y1 += 24 + 8;
    option_height = new InputInt(dialog, 0, y1, 72, 24, "Height:", 0);
    y1 += 24 + 8;
    option_width->center();
    option_height->center();
    option_width->maximum_size(8);
    option_height->maximum_size(8);
    option_width->value("640");
    option_height->value("480");
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace CreatePalette
{
  Fl_Double_Window *dialog;
  InputInt *option_colors;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::palette->max);
    option_colors->value(s);
    dialog->show();
  }

  void close()
  {
    if(option_colors->limitValue(1, 256) < 0)
      return;

    dialog->hide();
    Quantize::pca(Project::bmp, atoi(option_colors->value()));
  }

  void quit()
  {
    dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Create Palette");
    option_colors = new InputInt(dialog, 0, 8, 72, 24, "Colors:", 0);
    option_colors->center();
    y1 += 24 + 8;
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace Editor
{
  Fl_Double_Window *dialog;
  Widget *option_hue;
  Widget *option_sat_val;
  Fl_Button *option_insert;
  Fl_Button *option_remove;
  Fl_Button *option_replace;
  Fl_Button *option_undo;
  Fl_Button *option_rgb_ramp;
  Fl_Button *option_hsv_ramp;
  Widget *option_palette;
  Widget *option_color;
  Fl_Button *done;

  int ramp_begin;
  int ramp_state;
  bool begin_undo;
  int oldsvx, oldsvy;
  Palette *undo_palette;

  void storeUndo()
  {
    Project::palette->copy(undo_palette);
    begin_undo = true;
  }

  void getUndo()
  {
    if(begin_undo)
    {
      begin_undo = false;
      undo_palette->copy(Project::palette.get());
      Project::palette->draw(option_palette);
      Gui::drawPalette();
      option_palette->do_callback();
    }
  }

  void setHsv(bool redraw)
  {
    int r = 0, g = 0, b = 0;
    int h = 0, s = 0, v = 0;
    int c = Project::brush->color;

    Blend::rgbToHsv(getr(c), getg(c), getb(c), &h, &s, &v);

    if(redraw)
    {
      option_hue->bitmap->clear(makeRgb(0, 0, 0));
      option_sat_val->bitmap->clear(makeRgb(0, 0, 0));

      for(int y = 0; y < 256; y++)
      {
        for(int x = 0; x < 256; x++)
        {
          Blend::hsvToRgb(h, x, y, &r, &g, &b);
          option_sat_val->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
        }

        Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
        option_hue->bitmap->hline(0, y, 23, makeRgb(r, g, b), 0);
      }
    }
    else
    {
      // erase previous box if not redrawing entire thing
      option_sat_val->bitmap->xorRect(oldsvx - 4, oldsvy - 4, oldsvx + 4, oldsvy + 4);
    }

    int x = option_sat_val->var & 255;
    int y = option_sat_val->var / 256;

    if(x < 4)
      x = 4;
    if(y < 4)
      y = 4;
    if(x > 251)
      x = 251;
    if(y > 251)
      y = 251;

    option_sat_val->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);
    oldsvx = x;
    oldsvy = y;

    option_hue->redraw();
    option_sat_val->redraw();

    option_color->bitmap->clear(Project::brush->color);
    option_color->redraw();
  }

  void setHsvSliders()
  {
    int h, s, v;
    int color = Project::brush->color;
    Blend::rgbToHsv(getr(color), getg(color), getb(color), &h, &s, &v);
    option_hue->var = h / 6;
    option_sat_val->var = s + 256 * v;
    option_hue->redraw();
    option_sat_val->redraw();
  }

  void checkPalette(Widget *widget, void *var)
  {
    Palette *pal = Project::palette.get();
    int begin, end;

    if(ramp_state > 0)
    {
      storeUndo();
      begin = ramp_begin;
      end = *(int *)var;
      if(begin > end)
        std::swap(begin, end);
      int num = end - begin;

      if(ramp_state == 1)
      {
        // rgb ramp
        int c1 = pal->data[begin];
        int c2 = pal->data[end];
        double stepr = (double)(getr(c2) - getr(c1)) / num;
        double stepg = (double)(getg(c2) - getg(c1)) / num;
        double stepb = (double)(getb(c2) - getb(c1)) / num;
        double r = getr(c1);
        double g = getg(c1);
        double b = getb(c1);

        for(int i = begin; i < end; i++)
        {
          pal->data[i] = makeRgb(r, g, b);
          r += stepr;
          g += stepg;
          b += stepb;
        }

        option_rgb_ramp->value(0);
        option_rgb_ramp->redraw();
      }
      else if(ramp_state == 2)
      {
        // hsv ramp
        int c1 = pal->data[begin];
        int c2 = pal->data[end];
        int h1, s1, v1;
        int h2, s2, v2;

        Blend::rgbToHsv(getr(c1), getg(c1), getb(c1), &h1, &s1, &v1);
        Blend::rgbToHsv(getr(c2), getg(c2), getb(c2), &h2, &s2, &v2);

        int r, g, b;
        double h = h1;
        double s = s1;
        double v = v1;
        const double steph = (double)(h2 - h1) / num;
        const double steps = (double)(s2 - s1) / num;
        const double stepv = (double)(v2 - v1) / num;

        for(int i = begin; i < end; i++)
        {
          Blend::hsvToRgb(h, s, v, &r, &g, &b);
          pal->data[i] = makeRgb(r, g, b);
          h += steph;
          s += steps;
          v += stepv;
        }

        option_hsv_ramp->value(0);
        option_hsv_ramp->redraw();
      }

      ramp_state = 0;
      Project::palette->draw(option_palette);
      Gui::drawPalette();

      return;
    }

    Gui::checkPalette(widget, var);
    ramp_begin = *(int *)var;
    setHsvSliders();
    setHsv(1);
  }

  void getHue()
  {
    int h = option_hue->var * 6;
    int s = option_sat_val->var & 255;
    int v = option_sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::updateColor(Project::brush->color);
    setHsv(1);
  }

  void getSatVal()
  {
    int h = option_hue->var * 6;
    int s = option_sat_val->var & 255;
    int v = option_sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::updateColor(Project::brush->color);
    setHsv(0);
  }

  void insertColor()
  {
    storeUndo();
    Project::palette->insertColor(Project::brush->color, option_palette->var);
    Project::palette->draw(option_palette);
    Gui::drawPalette();
    option_palette->do_callback();
  }

  void removeColor()
  {
    storeUndo();
    Project::palette->deleteColor(option_palette->var);
    Project::palette->draw(option_palette);
    Gui::drawPalette();
    if(option_palette->var > Project::palette->max - 1)
      option_palette->var = Project::palette->max - 1;
    option_palette->do_callback();
  }

  void replaceColor()
  {
    storeUndo();
    Project::palette->replaceColor(Project::brush->color, option_palette->var);
    Project::palette->draw(option_palette);
    Gui::drawPalette();
    option_palette->do_callback();
  }

  void rgbRamp()
  {
    if(ramp_state == 0)
    {
      option_rgb_ramp->value(1);
      option_rgb_ramp->redraw();
      ramp_state = 1;
    }
  }

  void hsvRamp()
  {
    if(ramp_state == 0)
    {
      option_hsv_ramp->value(1);
      option_hsv_ramp->redraw();
      ramp_state = 2;
    }
  }

  void begin()
  {
    Project::palette->draw(option_palette);
    setHsvSliders();
    setHsv(1);
    dialog->show();
    begin_undo = false;
    ramp_begin = 0;
    ramp_state = 0;
  }

  void close()
  {
    Project::palette->fillTable();
    dialog->hide();
  }

  void init()
  {
    dialog = new Fl_Double_Window(608, 312, "Palette Editor");
    option_hue = new Widget(dialog, 8, 8, 24, 256,
                            "Hue", 24, 1,
                            (Fl_Callback *)getHue);
    option_sat_val = new Widget(dialog, 40, 8, 256, 256,
                                "Saturation/Value", 1, 1,
                                (Fl_Callback *)getSatVal);
    option_insert = new Fl_Button(304, 8, 96, 24, "Insert");
    option_insert->callback((Fl_Callback *)insertColor);
    option_remove = new Fl_Button(304, 48, 96, 24, "Delete");
    option_remove->callback((Fl_Callback *)removeColor);
    option_replace = new Fl_Button(304, 88, 96, 24, "Replace");
    option_replace->callback((Fl_Callback *)replaceColor);
    option_undo = new Fl_Button(304, 144, 96, 24, "Undo");
    option_undo->callback((Fl_Callback *)getUndo);
    option_rgb_ramp = new Fl_Button(304, 200, 96, 24, "RGB Ramp");
    option_rgb_ramp->callback((Fl_Callback *)rgbRamp);
    option_hsv_ramp = new Fl_Button(304, 240, 96, 24, "HSV Ramp");
    option_hsv_ramp->callback((Fl_Callback *)hsvRamp);
    option_palette = new Widget(dialog, 408, 8, 192, 192,
                                "Palette", 24, 24,
                                (Fl_Callback *)checkPalette);
    option_color = new Widget(dialog, 408, 208, 192, 56, "Color", 0, 0, 0);
    new Separator(dialog, 2, 272, 604, 2, "");
    done = new Fl_Button(504, 280, 96, 24, "Done");
    done->callback((Fl_Callback *)close);
    dialog->set_modal();
    dialog->end(); 

    undo_palette = new Palette();
  }
}

void Dialog::init()
{
  About::init();
  JpegQuality::init();
  PngOptions::init();
  Progress::init();
  NewImage::init();
  CreatePalette::init();
  Editor::init();
}

void Dialog::center(Fl_Widget *widget)
{
  int ww = 0, hh = 0;

  widget->measure_label(ww, hh);

  widget->resize((widget->parent()->w() / 2) - ((ww + widget->w()) / 2),
                 widget->y(),
                 widget->w(),
                 widget->h());

  widget->redraw();
}

void Dialog::addOkButton(Fl_Group *group, Fl_Button **ok, int *y1)
{
  int w = group->w();

  new Separator(group, 4, *y1, w - 8, 2, "");
  *y1 += 8;
  *ok = new Fl_Button(w - 64 - 8, *y1, 64, 24, "OK");
  group->add(*ok);
  *y1 += 24 + 8;
  group->resize(group->x(), group->y(), group->w(), *y1);
}

void Dialog::addOkCancelButtons(Fl_Group *group,
                                Fl_Button **ok, Fl_Button **cancel, int *y1)
{
  int w = group->w();

  new Separator(group, 4, *y1, w - 8, 2, "");
  *y1 += 8;
  *cancel = new Fl_Button(w - 64 - 8, *y1, 64, 24, "Cancel");
  group->add(*cancel);
  *ok = new Fl_Button((*cancel)->x() - 64 - 8, *y1, 64, 24, "Ok");
  *y1 += 24 + 8;
  group->add(*ok);
  group->resize(group->x(), group->y(), group->w(), *y1);
}

void Dialog::about()
{
  About::begin();
}

void Dialog::jpegQuality()
{
  JpegQuality::begin();
}

int Dialog::jpegQualityValue()
{
  return atoi(JpegQuality::option_quality->value());
}

void Dialog::pngOptions()
{
  PngOptions::begin();
}

int Dialog::pngUsePalette()
{
  return PngOptions::option_use_palette->value();
}

int Dialog::pngUseAlpha()
{
  return PngOptions::option_use_alpha->value();
}

int Dialog::pngAlphaLevels()
{
  return atoi(PngOptions::option_alpha_levels->value());
}

void Dialog::showProgress(float step)
{
  Progress::value = 0;
  Progress::step = 100.0 / step;
  Progress::dialog->show();
}

void Dialog::updateProgress()
{
  Progress::bar->value(Progress::value);
  char percent[16];
  sprintf(percent, "%d%%", (int)Progress::value);
  Progress::bar->label(percent);
  Fl::check();
  Progress::value += Progress::step;
}

void Dialog::hideProgress()
{
  Progress::dialog->hide();
}

void Dialog::newImage()
{
  NewImage::begin();
}

void Dialog::createPalette()
{
  CreatePalette::begin();
}

void Dialog::editor()
{
  Editor::begin();
}

