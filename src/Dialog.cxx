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

#ifndef PACKAGE_STRING
#  include "config.h"
#endif

#include <algorithm>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Widget.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "CheckBox.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "File.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
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
  namespace Items
  {
    DialogWindow *dialog;
    Widget *logo;
    Fl_Box *box;
    Fl_Button *ok;
  }

  void begin()
  {
    Items::dialog->show();
  }

  void close()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;
    int ww = 0, hh = 0;
    const char *credits = "Copyright (c) 2015 Joe Davisson.\nAll Rights Reserved.";

    Items::dialog = new DialogWindow(384, 0, "About");
    Items::logo = new Widget(Items::dialog, 32, y1, 320, 96,
                           credits, __logo_large_png, -1, -1, 0);
    if(Project::theme == Project::THEME_LIGHT)
    {
      Items::logo->bitmap->invert();
    }

    Items::logo->align(FL_ALIGN_BOTTOM);
    Items::logo->measure_label(ww, hh);
    y1 += 96 + 8 + hh;
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, y1, 368, 32, PACKAGE_STRING);
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(14);
    y1 += 32;
    Items::dialog->addOkButton(&Items::ok, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace JpegQuality
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *quality;
    Fl_Button *ok;
  }

  void closeCallback(Fl_Widget *, void *)
  {
    // needed to prevent dialog from being closed by window manager
  }

  void begin()
  {
    Items::dialog->show();

    while(true)
    {
      Fl_Widget *action = Fl::readqueue();

      if(!action)
      {
        Fl::wait();
      }
      else if(action == Items::ok)
      {
        if(Items::quality->limitValue(1, 100) == 0)
        {
          Items::dialog->hide();
          break;
        }
      }
    }
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "JPEG Quality");
    Items::dialog->callback(closeCallback);
    Items::quality = new InputInt(Items::dialog, 0, y1, 72, 24, "Quality:", 0);
    Items::quality->value("95");
    Items::quality->center();
    y1 += 24 + 8;
    Items::dialog->addOkButton(&Items::ok, &y1);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace PngOptions
{
  namespace Items
  {
    DialogWindow *dialog;
    CheckBox *use_palette;
    CheckBox *use_alpha;
    InputInt *alpha_levels;
    Fl_Button *ok;
  }

  void closeCallback(Fl_Widget *, void *)
  {
    // needed to prevent dialog from being closed by window manager
  }

  void begin()
  {
    Items::dialog->show();

    while(true)
    {
      Fl_Widget *action = Fl::readqueue();

      if(!action)
      {
        Fl::wait();
      }
      else if(action == Items::ok)
      {
        if(Items::alpha_levels->limitValue(2, 16) == 0)
        {
          Items::dialog->hide();
          break;
        }
      }
    }
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "PNG Options");
    Items::dialog->callback(closeCallback);
    Items::alpha_levels = new InputInt(Items::dialog, 0, y1, 72, 24, "Alpha Levels:", 0);
    Items::alpha_levels->value("2");
    Items::alpha_levels->center();
    y1 += 24 + 8;
    Items::use_palette = new CheckBox(Items::dialog, 0, y1, 16, 16, "Use Current Palette", 0);
    y1 += 16 + 8;
    Items::use_palette->center();
    Items::use_alpha = new CheckBox(Items::dialog, 0, y1, 16, 16, "Save Alpha Channel", 0);
    Items::use_alpha->value(1);
    Items::use_alpha->center();
    y1 += 16 + 8;
    Items::dialog->addOkButton(&Items::ok, &y1);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace NewImage
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *width;
    InputInt *height;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    Items::width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    Items::height->value(s);
    Items::dialog->show();
  }

  void close()
  {
    if(Items::width->limitValue(1, 10000) < 0)
      return;

    if(Items::height->limitValue(1, 10000) < 0)
      return;

    Items::dialog->hide();

    Project::newImage(atoi(Items::width->value()),
                      atoi(Items::height->value()));

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(true);
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "New Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 72, 24, "Width:", 0);
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 72, 24, "Height:", 0);
    y1 += 24 + 8;
    Items::width->center();
    Items::height->center();
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace MakePalette
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *colors;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::palette->max);
    Items::colors->value(s);
    Items::dialog->show();
  }

  void close()
  {
    if(Items::colors->limitValue(1, 256) < 0)
      return;

    Items::dialog->hide();
    Quantize::pca(Project::bmp, Project::palette.get(), atoi(Items::colors->value()));
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Create Palette");
    Items::colors = new InputInt(Items::dialog, 0, 8, 72, 24, "Colors:", 0);
    Items::colors->center();
    y1 += 24 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace Editor
{
  namespace Items
  {
    DialogWindow *dialog;
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
  }

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
      Project::palette->draw(Items::palette);
      Gui::drawPalette();
      Items::palette->do_callback();
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
      Items::hue->bitmap->clear(makeRgb(0, 0, 0));
      Items::sat_val->bitmap->clear(makeRgb(0, 0, 0));

      for(int y = 0; y < 256; y++)
      {
        for(int x = 0; x < 256; x++)
        {
          Blend::hsvToRgb(h, x, y, &r, &g, &b);
          Items::sat_val->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
        }

        Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
        Items::hue->bitmap->hline(0, y, 23, makeRgb(r, g, b), 0);
      }
    }
    else
    {
      // erase previous box if not redrawing entire thing
      Items::sat_val->bitmap->xorRect(oldsvx - 4, oldsvy - 4, oldsvx + 4, oldsvy + 4);
    }

    int x = Items::sat_val->var & 255;
    int y = Items::sat_val->var / 256;

    if(x < 4)
      x = 4;
    if(y < 4)
      y = 4;
    if(x > 251)
      x = 251;
    if(y > 251)
      y = 251;

    Items::sat_val->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);
    oldsvx = x;
    oldsvy = y;

    Items::hue->redraw();
    Items::sat_val->redraw();

    Items::color->bitmap->clear(Project::brush->color);
    Items::color->redraw();
  }

  void setHsvSliders()
  {
    int h, s, v;
    int color = Project::brush->color;
    Blend::rgbToHsv(getr(color), getg(color), getb(color), &h, &s, &v);
    Items::hue->var = h / 6;
    Items::sat_val->var = s + 256 * v;
    Items::hue->redraw();
    Items::sat_val->redraw();
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

        Items::rgb_ramp->value(0);
        Items::rgb_ramp->redraw();
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

        Items::hsv_ramp->value(0);
        Items::hsv_ramp->redraw();
      }

      ramp_state = 0;
      Project::palette->draw(Items::palette);
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
    int h = Items::hue->var * 6;
    int s = Items::sat_val->var & 255;
    int v = Items::sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::updateColor(Project::brush->color);
    setHsv(1);
  }

  void getSatVal()
  {
    int h = Items::hue->var * 6;
    int s = Items::sat_val->var & 255;
    int v = Items::sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::updateColor(Project::brush->color);
    setHsv(0);
  }

  void insertColor()
  {
    storeUndo();
    Project::palette->insertColor(Project::brush->color, Items::palette->var);
    Project::palette->draw(Items::palette);
    Gui::drawPalette();
    Items::palette->do_callback();
  }

  void removeColor()
  {
    storeUndo();
    Project::palette->deleteColor(Items::palette->var);
    Project::palette->draw(Items::palette);
    Gui::drawPalette();
    if(Items::palette->var > Project::palette->max - 1)
      Items::palette->var = Project::palette->max - 1;
    Items::palette->do_callback();
  }

  void replaceColor()
  {
    storeUndo();
    Project::palette->replaceColor(Project::brush->color, Items::palette->var);
    Project::palette->draw(Items::palette);
    Gui::drawPalette();
    Items::palette->do_callback();
  }

  void rgbRamp()
  {
    if(ramp_state == 0)
    {
      Items::rgb_ramp->value(1);
      Items::rgb_ramp->redraw();
      ramp_state = 1;
    }
  }

  void hsvRamp()
  {
    if(ramp_state == 0)
    {
      Items::hsv_ramp->value(1);
      Items::hsv_ramp->redraw();
      ramp_state = 2;
    }
  }

  void begin()
  {
    Project::palette->draw(Items::palette);
    setHsvSliders();
    setHsv(1);
    Items::dialog->show();
    begin_undo = false;
    ramp_begin = 0;
    ramp_state = 0;
  }

  void close()
  {
    Project::palette->fillTable();
    Items::dialog->hide();
  }

  void init()
  {
    Items::dialog = new DialogWindow(608, 312, "Palette Editor");
    Items::hue = new Widget(Items::dialog, 8, 8, 24, 256,
                            "Hue", 24, 1,
                            (Fl_Callback *)getHue);
    Items::sat_val = new Widget(Items::dialog, 40, 8, 256, 256,
                                "Saturation/Value", 1, 1,
                                (Fl_Callback *)getSatVal);
    Items::insert = new Fl_Button(304, 8, 96, 24, "Insert");
    Items::insert->callback((Fl_Callback *)insertColor);
    Items::remove = new Fl_Button(304, 48, 96, 24, "Delete");
    Items::remove->callback((Fl_Callback *)removeColor);
    Items::replace = new Fl_Button(304, 88, 96, 24, "Replace");
    Items::replace->callback((Fl_Callback *)replaceColor);
    Items::undo = new Fl_Button(304, 144, 96, 24, "Undo");
    Items::undo->callback((Fl_Callback *)getUndo);
    Items::rgb_ramp = new Fl_Button(304, 200, 96, 24, "RGB Ramp");
    Items::rgb_ramp->callback((Fl_Callback *)rgbRamp);
    Items::hsv_ramp = new Fl_Button(304, 240, 96, 24, "HSV Ramp");
    Items::hsv_ramp->callback((Fl_Callback *)hsvRamp);
    Items::palette = new Widget(Items::dialog, 408, 8, 192, 192,
                                "Palette", 24, 24,
                                (Fl_Callback *)checkPalette);
    Items::color = new Widget(Items::dialog, 408, 208, 192, 56, "Color", 0, 0, 0);
    new Separator(Items::dialog, 2, 272, 604, 2, "");
    Items::done = new Fl_Button(504, 280, 96, 24, "Done");
    Items::done->callback((Fl_Callback *)close);
    Items::dialog->set_modal();
    Items::dialog->end(); 

    undo_palette = new Palette();
  }
}

namespace Message
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Box *box;
    Fl_Button *ok;
  }

  void begin(const char *title, const char *message)
  {
    Items::dialog->copy_label(title);
    Items::box->copy_label(message);
    Items::dialog->show();
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(384, 0, "Error");
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 64, "");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(14); 
    y1 += 64;
    Items::dialog->addOkButton(&Items::ok, &y1);
    Items::ok->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace Choice
{
  bool yes = false;

  namespace Items
  {
    DialogWindow *dialog;
    Fl_Box *box;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void begin(const char *title, const char *message)
  {
    yes = false;
    Items::dialog->copy_label(title);
    Items::box->copy_label(message);
    Items::dialog->show();
  }

  void close()
  {
    yes = true;
    Items::dialog->hide();
  }

  void quit()
  {
    yes = false;
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(384, 0, "Error");
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, 8, 368, 64, "");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(14); 
    y1 += 64;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->label("Yes");
    Items::cancel->label("No");
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

void Dialog::init()
{
  About::init();
  JpegQuality::init();
  PngOptions::init();
  NewImage::init();
  MakePalette::init();
  Editor::init();
  Message::init();
  Choice::init();
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
  return atoi(JpegQuality::Items::quality->value());
}

void Dialog::pngOptions()
{
  PngOptions::begin();
}

int Dialog::pngUsePalette()
{
  return PngOptions::Items::use_palette->value();
}

int Dialog::pngUseAlpha()
{
  return PngOptions::Items::use_alpha->value();
}

int Dialog::pngAlphaLevels()
{
  return atoi(PngOptions::Items::alpha_levels->value());
}

void Dialog::newImage()
{
  NewImage::begin();
}

void Dialog::makePalette()
{
  MakePalette::begin();
}

void Dialog::editor()
{
  Editor::begin();
}

void Dialog::message(const char *title, const char *message)
{
  Message::begin(title, message);
}

bool Dialog::choice(const char *title, const char *message)
{
  Choice::begin(title, message);
  while(Choice::Items::dialog->shown())
    Fl::wait();
  return Choice::yes;
}

