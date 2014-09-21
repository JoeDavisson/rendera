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
#include "Field.H"
#include "Widget.H"
#include "Separator.H"
#include "Gui.H"
#include "View.H"
#include "Quantize.H"

namespace About
{
  // about
  Fl_Double_Window *dialog;
  Widget *logo;
  Fl_Button *ok;

  void begin()
  {
    dialog->show();
  }

  void end()
  {
    dialog->hide();
  }
}

namespace JpegQuality
{
  Fl_Double_Window *dialog;
  Field *amount;
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
}

namespace Progress
{
  Fl_Double_Window *dialog;
  Fl_Progress *bar;
  float value;
  float step;
}

namespace NewImage
{
  Fl_Double_Window *dialog;
  Field *width;
  Field *height;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Bitmap::main->w - Bitmap::main->overscroll * 2);
    width->value(s);
    snprintf(s, sizeof(s), "%d", Bitmap::main->h - Bitmap::main->overscroll * 2);
    height->value(s);
    dialog->show();
  }

  void end()
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

    int overscroll = Bitmap::main->overscroll;
    delete Bitmap::main;
    Bitmap::main = new Bitmap(w, h, overscroll,
                              make_rgb(255, 255, 255), make_rgb(128, 128, 128));

    delete Map::main;
    Map::main = new Map(Bitmap::main->w, Bitmap::main->h);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(1);
  }

  void quit()
  {
    dialog->hide();
  }
}

namespace CreatePalette
{
  Fl_Double_Window *dialog;
  Field *colors;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Palette::main->max);
    colors->value(s);
    dialog->show();
  }

  void end()
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
    Quantize::pca(Bitmap::main, c);
  }

  void quit()
  {
    dialog->hide();
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

  void storeUndo()
  {
    Palette::main->copy(Palette::undo);
    begin_undo = 1;
  }

  void getUndo()
  {
    if(begin_undo)
    {
      begin_undo = 0;
      Palette::undo->copy(Palette::main);
      Palette::main->draw(palette);
      Gui::drawPalette();
      palette->do_callback();
    }
  }

  void setHsv(bool redraw)
  {
    int x , y;
    int r = 0, g = 0, b = 0;
    int h = 0, s = 0, v = 0;
    int c = Brush::main->color;

    Blend::rgbToHsv(getr(c), getg(c), getb(c), &h, &s, &v);

    if(redraw)
    {
      hue->bitmap->clear(make_rgb(0, 0, 0));
      sat_val->bitmap->clear(make_rgb(0, 0, 0));

      for(y = 0; y < 256; y++)
      {
        for(x = 0; x < 256; x++)
        {
          Blend::hsvToRgb(h, x, y, &r, &g, &b);
          sat_val->bitmap->setpixelSolid(x, y, make_rgb(r, g, b), 0);
        }

        Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
        hue->bitmap->hline(0, y, 23, make_rgb(r, g, b), 0);
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

    color->bitmap->clear(Brush::main->color);
    color->redraw();
  }

  void setHsvSliders()
  {
    int h, s, v;
    int color = Brush::main->color;
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
    Palette *pal = Palette::main;

    if(ramp_started > 0)
    {
      storeUndo();
      begin = ramp_begin;
      end = *(int *)var;
      if(begin > end)
        SWAP(begin, end);
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
          pal->data[i] = make_rgb(r, g, b);
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
          pal->data[i] = make_rgb(r, g, b);
          h += steph;
          s += steps;
          v += stepv;
        }

        hsv_ramp->value(0);
        hsv_ramp->redraw();
      }

      ramp_started = 0;
      Palette::main->draw(palette);
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
    Brush::main->color = make_rgb(r, g, b);

    Gui::updateColor(Brush::main->color);
    setHsv(1);
  }

  void getSatVal()
  {
    int h = hue->var * 6;
    int s = sat_val->var & 255;
    int v = sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Brush::main->color = make_rgb(r, g, b);

    Gui::updateColor(Brush::main->color);
    setHsv(0);
  }

  void insertColor()
  {
    storeUndo();
    Palette::main->insertColor(Brush::main->color, palette->var);
    Palette::main->draw(palette);
    Gui::drawPalette();
    palette->do_callback();
  }

  void removeColor()
  {
    storeUndo();
    Palette::main->deleteColor(palette->var);
    Palette::main->draw(palette);
    Gui::drawPalette();
    if(palette->var > Palette::main->max - 1)
      palette->var = Palette::main->max - 1;
    palette->do_callback();
  }

  void replaceColor()
  {
    storeUndo();
    Palette::main->replaceColor(Brush::main->color, palette->var);
    Palette::main->draw(palette);
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
    Palette::main->draw(palette);
    setHsvSliders();
    setHsv(1);
    dialog->show();
    begin_undo = 0;
    ramp_begin = 0;
    ramp_started = 0;
  }

  void end()
  {
    Palette::main->fillTable();
    dialog->hide();
  }
}

namespace
{
  int file_exists(const char *s)
  {
    FILE *temp = fopen(s, "r");

    if(temp)
    {
      fclose(temp);
      return 1;
    }

    return 0;
  }
}

void Dialog::init()
{
  // about
  About::dialog = new Fl_Double_Window(336, 112, "About");
  About::logo = new Widget(About::dialog, 8, 8, 320, 64, "Logo", "data/logo_large.png", 0, 0, 0);
  About::ok = new Fl_Button(336 / 2 - 32, 80, 64, 24, "OK");
  About::ok->callback((Fl_Callback *)About::end);
  About::dialog->set_modal();
  About::dialog->end(); 

  // JPEG quality
  JpegQuality::dialog = new Fl_Double_Window(200, 80, "JPEG Quality");
  JpegQuality::dialog->callback(JpegQuality::closeCallback);
  JpegQuality::amount = new Field(JpegQuality::dialog, 80, 8, 72, 24, "Quality:", 0);
  JpegQuality::amount->value("95");
  new Separator(JpegQuality::dialog, 2, 40, 196, 2, "");
  JpegQuality::ok = new Fl_Button(128, 48, 64, 24, "OK");
  JpegQuality::dialog->set_modal();
  JpegQuality::dialog->end();

  // progress
  Progress::dialog = new Fl_Double_Window(272, 80, "Progress");
  Progress::bar = new Fl_Progress(8, 8, 256, 64);
  Progress::bar->minimum(0);
  Progress::bar->maximum(100);
  Progress::bar->color(0);
  Progress::bar->selection_color(0x88CC8800);
  Progress::bar->labelcolor(0xFFFFFF00);
  Progress::dialog->set_modal();
  Progress::dialog->end();

  // new image
  NewImage::dialog = new Fl_Double_Window(200, 112, "New Image");
  NewImage::width = new Field(NewImage::dialog, 88, 8, 72, 24, "Width:", 0);
  NewImage::height = new Field(NewImage::dialog, 88, 40, 72, 24, "Height:", 0);
  NewImage::width->maximum_size(8);
  NewImage::height->maximum_size(8);
  NewImage::width->value("640");
  NewImage::height->value("480");
  new Separator(NewImage::dialog, 2, 72, 196, 2, "");
  NewImage::ok = new Fl_Button(56, 80, 64, 24, "OK");
  NewImage::ok->callback((Fl_Callback *)NewImage::end);
  NewImage::cancel = new Fl_Button(128, 80, 64, 24, "Cancel");
  NewImage::cancel->callback((Fl_Callback *)NewImage::quit);
  NewImage::dialog->set_modal();
  NewImage::dialog->end(); 

  // create palette from image
  CreatePalette::dialog = new Fl_Double_Window(200, 80, "Create Palette");
  CreatePalette::colors = new Field(CreatePalette::dialog, 80, 8, 72, 24, "Colors:", 0);
  CreatePalette::ok = new Fl_Button(56, 48, 64, 24, "OK");
  CreatePalette::ok->callback((Fl_Callback *)CreatePalette::end);
  CreatePalette::cancel = new Fl_Button(128, 48, 64, 24, "Cancel");
  CreatePalette::cancel->callback((Fl_Callback *)CreatePalette::quit);
  CreatePalette::dialog->set_modal();
  CreatePalette::dialog->end(); 

  // palette editor
  Editor::dialog = new Fl_Double_Window(608, 312, "Palette Editor");
  Editor::hue = new Widget(Editor::dialog, 8, 8, 24, 256, "Hue", 24, 1, (Fl_Callback *)Editor::getHue);
  Editor::sat_val = new Widget(Editor::dialog, 40, 8, 256, 256, "Saturation/Value", 1, 1, (Fl_Callback *)Editor::getSatVal);
  Editor::insert = new Fl_Button(304, 8, 96, 24, "Insert");
  Editor::insert->callback((Fl_Callback *)Editor::insertColor);
  Editor::remove = new Fl_Button(304, 48, 96, 24, "Delete");
  Editor::remove->callback((Fl_Callback *)Editor::removeColor);
  Editor::replace = new Fl_Button(304, 88, 96, 24, "Replace");
  Editor::replace->callback((Fl_Callback *)Editor::replaceColor);
  Editor::undo = new Fl_Button(304, 144, 96, 24, "Undo");
  Editor::undo->callback((Fl_Callback *)Editor::getUndo);
  Editor::rgb_ramp = new Fl_Button(304, 200, 96, 24, "RGB Ramp");
  Editor::rgb_ramp->callback((Fl_Callback *)Editor::rgbRamp);
  Editor::hsv_ramp = new Fl_Button(304, 240, 96, 24, "HSV Ramp");
  Editor::hsv_ramp->callback((Fl_Callback *)Editor::hsvRamp);
  Editor::palette = new Widget(Editor::dialog, 408, 8, 192, 192, "Palette", 24, 24, (Fl_Callback *)Editor::checkPalette);
  Editor::color = new Widget(Editor::dialog, 408, 208, 192, 56, "Color", 0, 0, 0);
  new Separator(Editor::dialog, 2, 272, 604, 2, "");
  Editor::done = new Fl_Button(504, 280, 96, 24, "Done");
  Editor::done->callback((Fl_Callback *)Editor::end);
  Editor::dialog->set_modal();
  Editor::dialog->end(); 
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

