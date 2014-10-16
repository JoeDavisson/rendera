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

#include "Dialog.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Palette.H"
#include "Blend.H"
#include "Map.H"
#include "InputInt.H"
#include "Widget.H"
#include "Separator.H"
#include "Gui.H"
#include "View.H"
#include "Quantize.H"
#include "Project.H"

namespace
{
  void center(Fl_Widget *widget)
  {
    int ww, hh;

    widget->measure_label(ww, hh);

    widget->resize(widget->parent()->w() / 2 - (ww + widget->w()) / 2 + ww,
                   widget->y(),
                   widget->w(),
                   widget->h());
  }

  void addOkButton(Fl_Group *group, Fl_Button **ok, int *y1)
  {
    int w = group->w();

    new Separator(group, 4, *y1, w - 8, 2, "");
    *y1 += 8;
    *ok = new Fl_Button(w - 64 - 8, *y1, 64, 24, "OK");
    group->add(*ok);
    *y1 += 24 + 8;
    group->resize(group->x(), group->y(), group->w(), *y1);
  }

  void addOkCancelButtons(Fl_Group *group, Fl_Button **ok, Fl_Button **cancel, int *y1)
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
}

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

    dialog = new Fl_Double_Window(384, 0, "About");
    logo = new Widget(dialog, 0, y1, 320, 64, "", "data/logo_large.png", 0, 0, 0);
    center(logo);
    y1 += 64 + 8;
    addOkButton(dialog, &ok, &y1);
    ok->callback((Fl_Callback *)close);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace JpegQuality
{
  Fl_Double_Window *dialog;
  InputInt *amount;
  Fl_Button *ok;

  void closeCallback(Fl_Widget *, void *)
  {
    // needed to prevent dialog from being closed by window manager
  }

  void begin()
  {
    dialog->show();

    while(1)
    {
      Fl_Widget *action = Fl::readqueue();

      if(!action)
      {
        Fl::wait();
      }
      else if(action == ok)
      {
        char s[8];
        int q = atoi(amount->value());

        if(q < 1)
        {
          snprintf(s, sizeof(s), "%d", 1);
          amount->value(s);
        }
        else if(q > 100)
        {
          snprintf(s, sizeof(s), "%d", 100);
          amount->value(s);
        }
        else
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
    amount = new InputInt(dialog, 0, y1, 72, 24, "Quality:", 0);
    amount->value("95");
    center(amount);
    y1 += 24 + 8;
    addOkButton(dialog, &ok, &y1);
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
  InputInt *width;
  InputInt *height;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->w - Project::overscroll * 2);
    width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->h - Project::overscroll * 2);
    height->value(s);
    dialog->show();
  }

  void close()
  {
    char s[8];

    int w = atoi(width->value());
    int h = atoi(height->value());

    if(w < 1)
    {
      snprintf(s, sizeof(s), "%d", 1);
      width->value(s);
      return;
    }

    if(h < 1)
    {
      snprintf(s, sizeof(s), "%d", 1);
      height->value(s);
      return;
    }

    if(w > 10000)
    {
      snprintf(s, sizeof(s), "%d", 10000);
      width->value(s);
      return;
    }

    if(h > 10000)
    {
      snprintf(s, sizeof(s), "%d", 10000);
      height->value(s);
      return;
    }

    dialog->hide();

    Project::newImage(w, h);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(1);
  }

  void quit()
  {
    dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "New Image");
    width = new InputInt(dialog, 0, y1, 72, 24, "Width:", 0);
    y1 += 24 + 8;
    height = new InputInt(dialog, 0, y1, 72, 24, "Height:", 0);
    y1 += 24 + 8;
    center(width);
    center(height);
    width->maximum_size(8);
    height->maximum_size(8);
    width->value("640");
    height->value("480");
    addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace CreatePalette
{
  Fl_Double_Window *dialog;
  InputInt *colors;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::palette->max);
    colors->value(s);
    dialog->show();
  }

  void close()
  {
    char s[8];

    int c = atoi(colors->value());

    if(c < 1)
    {
      snprintf(s, sizeof(s), "%d", 1);
      colors->value(s);
      return;
    }

    if(c > 256)
    {
      snprintf(s, sizeof(s), "%d", 256);
      colors->value(s);
      return;
    }

    dialog->hide();
    Quantize::pca(Project::bmp, c);
  }

  void quit()
  {
    dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    dialog = new Fl_Double_Window(256, 0, "Create Palette");
    colors = new InputInt(dialog, 0, 8, 72, 24, "Colors:", 0);
    center(colors);
    y1 += 24 + 8;
    addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace Editor
{
  Fl_Double_Window *dialog;
  Widget *hue;
  Widget *sat_val;
  Fl_Button *insert;
  Fl_Button *remove;
  Fl_Button *replace;
  Fl_Button *undo;
  Fl_Button *rgb_ramp;
  Fl_Button *hsv_ramp;
  Widget *palette;
  Widget *color;
  Fl_Button *done;

  int ramp_begin;
  int ramp_started;
  int begin_undo;
  int oldsvx, oldsvy;
  Palette *undo_palette;

  void storeUndo()
  {
    Project::palette->copy(undo_palette);
    begin_undo = 1;
  }

  void getUndo()
  {
    if(begin_undo)
    {
      begin_undo = 0;
      undo_palette->copy(Project::palette);
      Project::palette->draw(palette);
      Gui::drawPalette();
      palette->do_callback();
    }
  }

  void setHsv(bool redraw)
  {
    int x , y;
    int r = 0, g = 0, b = 0;
    int h = 0, s = 0, v = 0;
    int c = Project::brush->color;

    Blend::rgbToHsv(getr(c), getg(c), getb(c), &h, &s, &v);

    if(redraw)
    {
      hue->bitmap->clear(makeRgb(0, 0, 0));
      sat_val->bitmap->clear(makeRgb(0, 0, 0));

      for(y = 0; y < 256; y++)
      {
        for(x = 0; x < 256; x++)
        {
          Blend::hsvToRgb(h, x, y, &r, &g, &b);
          sat_val->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
        }

        Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
        hue->bitmap->hline(0, y, 23, makeRgb(r, g, b), 0);
      }
    }
    else
    {
      // erase previous box if not redrawing entire thing
      sat_val->bitmap->xorRect(oldsvx - 4, oldsvy - 4, oldsvx + 4, oldsvy + 4);
    }

    x = sat_val->var & 255;
    y = sat_val->var / 256;

    if(x < 4)
      x = 4;
    if(y < 4)
      y = 4;
    if(x > 251)
      x = 251;
    if(y > 251)
      y = 251;

    sat_val->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);
    oldsvx = x;
    oldsvy = y;

    hue->redraw();
    sat_val->redraw();

    color->bitmap->clear(Project::brush->color);
    color->redraw();
  }

  void setHsvSliders()
  {
    int h, s, v;
    int color = Project::brush->color;
    Blend::rgbToHsv(getr(color), getg(color), getb(color), &h, &s, &v);
    hue->var = h / 6;
    sat_val->var = s + 256 * v;
    hue->redraw();
    sat_val->redraw();
  }

  void checkPalette(Widget *widget, void *var)
  {
    int i;
    int begin, end;
    Palette *pal = Project::palette;

    if(ramp_started > 0)
    {
      storeUndo();
      begin = ramp_begin;
      end = *(int *)var;
      if(begin > end)
        std::swap(begin, end);
      int num = end - begin;

      if(ramp_started == 1)
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

        for(i = begin; i < end; i++)
        {
          pal->data[i] = makeRgb(r, g, b);
          r += stepr;
          g += stepg;
          b += stepb;
        }

        rgb_ramp->value(0);
        rgb_ramp->redraw();
      }
      else if(ramp_started == 2)
      {
        // hsv ramp
        int c1 = pal->data[begin];
        int c2 = pal->data[end];
        int h1, s1, v1;
        int h2, s2, v2;
        Blend::rgbToHsv(getr(c1), getg(c1), getb(c1), &h1, &s1, &v1);
        Blend::rgbToHsv(getr(c2), getg(c2), getb(c2), &h2, &s2, &v2);
        double steph = (double)(h2 - h1) / num;
        double steps = (double)(s2 - s1) / num;
        double stepv = (double)(v2 - v1) / num;
        int r, g, b;
        double h = h1;
        double s = s1;
        double v = v1;

        for(i = begin; i < end; i++)
        {
          Blend::hsvToRgb(h, s, v, &r, &g, &b);
          pal->data[i] = makeRgb(r, g, b);
          h += steph;
          s += steps;
          v += stepv;
        }

        hsv_ramp->value(0);
        hsv_ramp->redraw();
      }

      ramp_started = 0;
      Project::palette->draw(palette);
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
    int h = hue->var * 6;
    int s = sat_val->var & 255;
    int v = sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::updateColor(Project::brush->color);
    setHsv(1);
  }

  void getSatVal()
  {
    int h = hue->var * 6;
    int s = sat_val->var & 255;
    int v = sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::updateColor(Project::brush->color);
    setHsv(0);
  }

  void insertColor()
  {
    storeUndo();
    Project::palette->insertColor(Project::brush->color, palette->var);
    Project::palette->draw(palette);
    Gui::drawPalette();
    palette->do_callback();
  }

  void removeColor()
  {
    storeUndo();
    Project::palette->deleteColor(palette->var);
    Project::palette->draw(palette);
    Gui::drawPalette();
    if(palette->var > Project::palette->max - 1)
      palette->var = Project::palette->max - 1;
    palette->do_callback();
  }

  void replaceColor()
  {
    storeUndo();
    Project::palette->replaceColor(Project::brush->color, palette->var);
    Project::palette->draw(palette);
    Gui::drawPalette();
    palette->do_callback();
  }

  void rgbRamp()
  {
    if(!ramp_started)
    {
      rgb_ramp->value(1);
      rgb_ramp->redraw();
      ramp_started = 1;
    }
  }

  void hsvRamp()
  {
    if(!ramp_started)
    {
      hsv_ramp->value(1);
      hsv_ramp->redraw();
      ramp_started = 2;
    }
  }

  void begin()
  {
    Project::palette->draw(palette);
    setHsvSliders();
    setHsv(1);
    dialog->show();
    begin_undo = 0;
    ramp_begin = 0;
    ramp_started = 0;
  }

  void close()
  {
    Project::palette->fillTable();
    dialog->hide();
  }

  void init()
  {
    dialog = new Fl_Double_Window(608, 312, "Palette Editor");
    hue = new Widget(dialog, 8, 8, 24, 256, "Hue", 24, 1, (Fl_Callback *)getHue);
    sat_val = new Widget(dialog, 40, 8, 256, 256, "Saturation/Value", 1, 1, (Fl_Callback *)getSatVal);
    insert = new Fl_Button(304, 8, 96, 24, "Insert");
    insert->callback((Fl_Callback *)insertColor);
    remove = new Fl_Button(304, 48, 96, 24, "Delete");
    remove->callback((Fl_Callback *)removeColor);
    replace = new Fl_Button(304, 88, 96, 24, "Replace");
    replace->callback((Fl_Callback *)replaceColor);
    undo = new Fl_Button(304, 144, 96, 24, "Undo");
    undo->callback((Fl_Callback *)getUndo);
    rgb_ramp = new Fl_Button(304, 200, 96, 24, "RGB Ramp");
    rgb_ramp->callback((Fl_Callback *)rgbRamp);
    hsv_ramp = new Fl_Button(304, 240, 96, 24, "HSV Ramp");
    hsv_ramp->callback((Fl_Callback *)hsvRamp);
    palette = new Widget(dialog, 408, 8, 192, 192, "Palette", 24, 24, (Fl_Callback *)checkPalette);
    color = new Widget(dialog, 408, 208, 192, 56, "Color", 0, 0, 0);
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
  Progress::init();
  NewImage::init();
  CreatePalette::init();
  Editor::init();
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
  int quality = atoi(JpegQuality::amount->value());

  if(quality < 1)
    quality = 1;
  if(quality > 100)
    quality = 100;

  return quality;
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

