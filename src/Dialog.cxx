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

#ifndef PACKAGE_STRING
#  include "config.h"
#endif

#include <algorithm>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Widget.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Button.H"
#include "CheckBox.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "File.H"
#include "Group.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
#include "InputInt.H"
#include "InputText.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "Quantize.H"
#include "Separator.H"
#include "Undo.H"
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
    const char *credits = "\nCopyright (c) 2022 Joe Davisson.\n\nRendera is based in part on the work\nof the FLTK project (http://www.fltk.org).";

    Items::dialog = new DialogWindow(384, 0, "About");

    Items::logo = new Widget(Items::dialog, 33, y1, 318, 95,
                             credits, images_logo_dark_png, -1, -1, 0);

    Items::logo->align(FL_ALIGN_BOTTOM);
    Items::logo->measure_label(ww, hh);
    y1 += 96 + 16 + hh;
    Items::box = new Fl_Box(FL_FLAT_BOX, 8, y1, 368, 32, PACKAGE_STRING);
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(14);
    y1 += 24;
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
        Items::dialog->hide();
        break;
      }
    }
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "JPEG Quality");
    Items::dialog->callback(closeCallback);
    Items::quality = new InputInt(Items::dialog, 0, y1, 96, 24, "Quality:", 0, 1, 100);
    Items::quality->value("90");
    Items::quality->center();
    y1 += 24 + 8;
    Items::dialog->addOkButton(&Items::ok, &y1);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

namespace JavaExport
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *option;
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
        Items::dialog->hide();
        break;
      }
    }
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Export Java Array");
    Items::option = new Fl_Choice(64, y1, 128, 24, "");
    Items::option->tooltip("Format");
    Items::option->textsize(10);
    Items::option->add("C64 Characters");
    Items::option->add("C64 Sprites");
    Items::option->value(0);
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
        Items::dialog->hide();
        break;
      }
    }
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "PNG Options");
    Items::dialog->callback(closeCallback);
    Items::alpha_levels = new InputInt(Items::dialog, 0, y1, 96, 24, "Alpha Levels:", 0, 2, 16);
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

namespace TextOptions
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Choice *bits;
    Fl_Choice *align;
    Fl_Button *ok;
    Fl_Button *cancel;
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
        Items::dialog->hide();
        break;
      }
      else if(action == Items::cancel)
      {
        Items::dialog->hide();
        break;
      }
    }
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Text Data Options");
    Items::dialog->callback(closeCallback);
    Items::bits = new Fl_Choice(64, y1, 128, 24, "");
    Items::bits->tooltip("Bit Depth");
    Items::bits->textsize(10);
    Items::bits->add("1-bit (2 colors)");
    Items::bits->add("2-bit (4 colors)");
    Items::bits->add("4-bit (16 colors)");
    Items::bits->add("8-bit (256 colors)");
    Items::bits->value(0);
    y1 += 24 + 8;
    Items::align = new Fl_Choice(64, y1, 128, 24, "");
    Items::align->tooltip("Byte Alignment");
    Items::align->textsize(10);
    Items::align->add("Linear");
    Items::align->add("C64 Characters");
    Items::align->add("C64 Hi-Res");
    Items::align->add("C64 Sprite");
    Items::align->value(0);
    y1 += 24 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
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
    CheckBox *keep_aspect;
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

  void checkWidth()
  {
    if(Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[8];
      int w = atoi(Items::width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      Items::height->value(s);
    }
  }

  void checkHeight()
  {
    if(Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[8];
      int h = atoi(Items::height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      Items::width->value(s);
    }
  }

  void close()
  {
    Items::dialog->hide();

    if(Project::newImage(atoi(Items::width->value()),
                         atoi(Items::height->value())) == -1)
    {
      return;
    }

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->drawMain(true);

    Gui::imagesAddFile("new");
    Project::undo->reset();
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "New Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 96, 24, "Width", (Fl_Callback *)checkWidth, 1, 10000);
    Items::width->center();
    Items::width->maximum_size(8);
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 96, 24, "Height", (Fl_Callback *)checkHeight, 1, 10000);
    Items::height->center();
    Items::height->maximum_size(8);
    y1 += 24 + 8;
    Items::keep_aspect = new CheckBox(Items::dialog, 0, y1, 16, 16, "Keep Aspect", 0);
    Items::keep_aspect->center();
    y1 += 16 + 8;
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
    Items::dialog->hide();
    Quantize::pca(Project::bmp, Project::palette, atoi(Items::colors->value()));
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new DialogWindow(256, 0, "Create Palette");
    Items::colors = new InputInt(Items::dialog, 0, 8, 96, 24, "Colors:", 0, 1, 256);
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
    Widget *lum;
    Widget *sat_val;
    Fl_Repeat_Button *insert;
    Fl_Repeat_Button *remove;
    Fl_Button *replace;
    Fl_Button *undo;
    Fl_Button *rgb_ramp;
    Fl_Button *hsv_ramp;
    InputText *hexcolor;
    InputText *hexcolor_web;
    Widget *palette;
    Fl_Box *palette_border;
    Widget *color;
    Fl_Button *done;
    Group *info;
    Fl_Box *info_text;
    Group *index;
    Fl_Box *index_text;
  }

  int last_index = 0;
  int replace_state = 0;
  int ramp_begin = 0;
  int ramp_state = 0;
  bool begin_undo;
  int oldsvx, oldsvy, oldhy;
  Palette *undo_palette;

  void updateInfo(char *s)
  {
    Items::info_text->copy_label(s);
    Items::info_text->redraw();
  }

  void updateIndex(int index)
  {
    char s[256];

    sprintf(s, "  Index = %d", index);
    Items::index_text->copy_label(s);
    Items::index_text->redraw();
  }

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
      undo_palette->copy(Project::palette);
      Project::palette->draw(Items::palette);
      Gui::paletteDraw();
      Items::palette->do_callback();
    }
  }

  void setHsvSliders()
  {
    int h, s, v;
    int c = Project::brush->color;

    Blend::rgbToHsv(getr(c), getg(c), getb(c), &h, &s, &v);
    Items::hue->var = h / 6 * 24;
    Items::sat_val->var = s + 256 * v;
    Items::hue->redraw();
    Items::sat_val->redraw();
  }

  void setHsv(bool redraw)
  {
    int r = 0, g = 0, b = 0;
    int c = Project::brush->color;
    
    int h = (Items::hue->var / 24) * 6;

    Items::hue->bitmap->clear(makeRgb(0, 0, 0));

    for(int y = 0; y < 256; y++)
    {
      int *p = Items::sat_val->bitmap->row[y];

      for(int x = 0; x < 256; x++)
      {
        Blend::hsvToRgb(h, x, y, &r, &g, &b);
        *p++ = makeRgb(r, g, b);
      }

      Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
      Items::hue->bitmap->hline(0, y, 23, makeRgb(r, g, b), 0);
    }

    int svx = Items::sat_val->var & 255;
    int svy = Items::sat_val->var / 256;
    int hy = Items::hue->var / 24;

    Items::sat_val->bitmap->rect(0, svy - 1, 255, svy + 1, makeRgb(0, 0, 0), 128);
    Items::sat_val->bitmap->rect(svx - 1, 0, svx + 1, 255, makeRgb(0, 0, 0), 128);
    Items::sat_val->bitmap->rect(0, svy - 2, 255, svy + 2, makeRgb(0, 0, 0), 192);
    Items::sat_val->bitmap->rect(svx - 2, 0, svx + 2, 255, makeRgb(0, 0, 0), 192);
    Items::sat_val->bitmap->xorHline(0, svy, 255);
    Items::sat_val->bitmap->xorVline(0, svx, 255);
    Items::sat_val->bitmap->rect(svx - 10, svy - 10, svx + 10, svy + 10, makeRgb(0, 0, 0), 192);
    Items::sat_val->bitmap->rect(svx - 9, svy - 9, svx + 9, svy + 9, makeRgb(0, 0, 0), 128);
    Items::sat_val->bitmap->xorRect(svx - 8, svy - 8, svx + 8, svy + 8);
    Items::sat_val->bitmap->rectfill(svx - 7, svy - 7, svx + 7, svy + 7, c, 0);
    Items::sat_val->bitmap->rect(0, 0, 255, 255, makeRgb(0, 0, 0), 0);

    c = Items::hue->bitmap->getpixel(12, hy);

    Items::hue->bitmap->rect(0, hy - 6, 23, hy + 6, makeRgb(0, 0, 0), 192);
    Items::hue->bitmap->rect(0, hy - 5, 23, hy + 5, makeRgb(0, 0, 0), 128);


    Items::hue->bitmap->xorRect(0, hy - 4, 23, hy + 4);
    Items::hue->bitmap->rectfill(0, hy - 3 , 23, hy + 3, c, 0);
    Items::hue->bitmap->rect(0, 0, 23, 255, makeRgb(0, 0, 0), 0);

    oldsvx = svx;
    oldsvy = svy;
    oldhy = hy;

    Items::hue->redraw();
    Items::sat_val->redraw();

    c = Project::brush->color;
    Items::color->bitmap->clear(c);

    rgba_type rgba = getRgba(c);

    r = rgba.r >> 4;
    g = rgba.g >> 4;
    b = rgba.b >> 4;

    r = (r << 4) + r;
    g = (g << 4) + g;
    b = (b << 4) + b;

    c = makeRgba(r, g, b, 255);

    Items::color->bitmap->rect(0, 0, Items::color->bitmap->w - 1, Items::color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
    Items::color->redraw();
  }

  void updateHexColor()
  {
    char hex_string[8];

    snprintf(hex_string, sizeof(hex_string),
       "%06x", (unsigned)convertFormat(Project::brush->color, true) & 0xFFFFFF);
    Items::hexcolor->value(hex_string);

    // shortcut hex
    int c = (unsigned)convertFormat(Project::brush->color, true) & 0xFFFFFF;
    rgba_type rgba = getRgba(c);

    snprintf(hex_string, sizeof(hex_string),
       "%01x%01x%01x", rgba.b >> 4, rgba.g >> 4, rgba.r >> 4);
    Items::hexcolor_web->value(hex_string);
  }

  void checkHexColor()
  { 
    unsigned int c;
  
    sscanf(Items::hexcolor->value(), "%06x", &c);
  
    if(c > 0xFFFFFF)
      c = 0xFFFFFF;
  
    c |= 0xFF000000;
  
    Gui::colorUpdate(convertFormat((int)c, true));
    setHsvSliders();
    setHsv(true);
    updateHexColor();
  }

  void checkHexColorWeb()
  { 
    unsigned int c;
  
    sscanf(Items::hexcolor_web->value(), "%03x", &c);
  
    if(c > 0xFFF)
      c = 0xFFF;

    int r = (c & 0xFFF) >> 8;
    int g = (c & 0xFF) >> 4;
    int b = c & 0xF;

    r = (r << 4) + r;
    g = (g << 4) + g;
    b = (b << 4) + b;

    c = makeRgba(r, g, b, 255);
    Project::brush->color = c;
  
    setHsvSliders();
    setHsv(true);
    updateHexColor();
    Gui::colorUpdate(c);
  }

  void insertColor()
  {
    storeUndo();
    Project::palette->insertColor(Project::brush->color, Items::palette->var);
    Project::palette->draw(Items::palette);
    Gui::paletteDraw();
    Items::palette->do_callback();
  }

  void removeColor()
  {
    storeUndo();
    Project::palette->deleteColor(Items::palette->var);
    Project::palette->draw(Items::palette);
    Gui::paletteDraw();

    if(Items::palette->var > Project::palette->max - 1)
      Items::palette->var = Project::palette->max - 1;

    Items::palette->do_callback();
  }

  void checkReplaceColor(int pos)
  {
    storeUndo();
    Project::palette->replaceColor(Project::brush->color, pos);
    Project::palette->draw(Items::palette);
    Gui::colorUpdate(Project::brush->color);
    Gui::paletteDraw();
    Items::replace->value(0);
    Items::replace->redraw();
    Items::dialog->cursor(FL_CURSOR_DEFAULT);
    replace_state = 0;
  }

  void replaceColor()
  {
    if(ramp_state == 0)
    {
      Items::replace->value(1);
      Items::replace->redraw();
      replace_state = 1;
      Items::dialog->cursor(FL_CURSOR_HAND);
    }
  }

  void copyColor(int c1, int c2)
  {
    storeUndo();
    Project::palette->data[c2] = Project::palette->data[c1];
  }

  void swapColor(int c1, int c2)
  {
    storeUndo();
    Project::palette->swapColor(c1, c2);
  }

  void checkRampRgb(int end)
  {
    storeUndo();

    Palette *pal = Project::palette;
    int begin = ramp_begin;

    if(begin > end)
      std::swap(begin, end);

    int num = end - begin;
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
    Project::palette->draw(Items::palette);
    Gui::paletteDraw();
    ramp_state = 0;
    Items::dialog->cursor(FL_CURSOR_DEFAULT);
  }

  void checkRampHsv(int end)
  {
    storeUndo();

    Palette *pal = Project::palette;
    int begin = ramp_begin;

    if(begin > end)
      std::swap(begin, end);

    int num = end - begin;
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
    Project::palette->draw(Items::palette);
    Gui::paletteDraw();
    ramp_state = 0;
    Items::dialog->cursor(FL_CURSOR_DEFAULT);
  }

  void checkPalette(Widget *widget, void *var)
  {
    Palette *pal = Project::palette;
    int pos = *(int *)var;

    if(pos > pal->max - 1)
    {
      pos = pal->max - 1;
      widget->var = pos;
    }

    if(Fl::event_button1())
    {
      if(ramp_state)
      {
        ramp_begin = last_index;

        switch(ramp_state)
        {
          case 1:
            checkRampRgb(pos);
            break;
          case 2:
            checkRampHsv(pos);
            break;
        }
      }
      else if(replace_state)
      {
        checkReplaceColor(pos);
      }
      else if(Fl::event_ctrl())
      {
        copyColor(last_index, pos);
        Project::brush->color = pal->data[pos];
        updateHexColor();
        setHsvSliders();
        Gui::colorUpdate(Project::brush->color);
        setHsv(true);
        Project::palette->draw(Items::palette);
      }
      else if(Fl::event_shift())
      {
        swapColor(last_index, pos);
        Project::brush->color = pal->data[pos];
        updateHexColor();
        setHsvSliders();
        Gui::colorUpdate(Project::brush->color);
        setHsv(true);
        Project::palette->draw(Items::palette);
      }
      else
      {
        Project::brush->color = pal->data[pos];
        updateHexColor();
        setHsvSliders();
        Gui::colorUpdate(Project::brush->color);
        setHsv(true);
      }
    }

    Gui::paletteIndex(pos);
    updateIndex(pos);
    last_index = pos;

    pal->draw(Items::palette);
  }

  void getHue()
  {
    int h = (Items::hue->var / 24) * 6;
    int s = Items::sat_val->var & 255;
    int v = Items::sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::colorUpdate(Project::brush->color);
    updateHexColor();
    setHsv(true);
  }

  void getSatVal()
  {
    int h = (Items::hue->var / 24) * 6;
    int s = Items::sat_val->var & 255;
    int v = Items::sat_val->var / 256;
    int r, g, b;

    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    Project::brush->color = makeRgb(r, g, b);

    Gui::colorUpdate(Project::brush->color);
    updateHexColor();
    setHsv(false);
  }

  void rgbRamp(Widget *widget, void *var)
  {
    if(ramp_state == 0)
    {
      Items::rgb_ramp->value(1);
      Items::rgb_ramp->redraw();
      ramp_state = 1;
      Items::dialog->cursor(FL_CURSOR_HAND);
    }
  }

  void hsvRamp()
  {
    if(ramp_state == 0)
    {
      Items::hsv_ramp->value(1);
      Items::hsv_ramp->redraw();
      ramp_state = 2;
      Items::dialog->cursor(FL_CURSOR_HAND);
    }
  }

  void begin()
  {
    Items::palette->var = Gui::getPaletteIndex();
    last_index = Items::palette->var;
    Project::palette->draw(Items::palette);
    updateHexColor();
    setHsvSliders();
    setHsv(1);
    Items::dialog->show();
    begin_undo = false;
    ramp_begin = 0;
    ramp_state = 0;
    updateInfo((char *)"  Shift to swap, Ctrl to copy, Right-click to move cursor.");
    updateIndex(Items::palette->var);
    while(Items::dialog->shown())
      Fl::wait();
  }

  void close()
  {
    replace_state = 0;
    ramp_begin = 0;
    ramp_state = 0;
    Items::replace->value(0);
    Items::rgb_ramp->value(0);
    Items::hsv_ramp->value(0);
    Project::palette->fillTable();
    Items::dialog->hide();
  }

  void init()
  {
    int x1, y1;

    Items::dialog = new DialogWindow(480, 346, "Palette Editor");

    Items::hue = new Widget(Items::dialog, 8, 8, 24, 256, 0, 1, 1, (Fl_Callback *)getHue);

    Items::sat_val = new Widget(Items::dialog, 40, 8, 256, 256,
                                0, 1, 1, (Fl_Callback *)getSatVal);

    y1 = 8;
    Items::insert = new Fl_Repeat_Button(304, y1, 44, 32, "+");
    Items::insert->callback((Fl_Callback *)insertColor);
    Items::insert->labelsize(22);
    Items::insert->tooltip("Insert");

    Items::remove = new Fl_Repeat_Button(358, y1, 44, 32, "-");
    Items::remove->callback((Fl_Callback *)removeColor);
    Items::remove->labelsize(22);
    Items::remove->tooltip("Remove");
    y1 += 32 + 8;

    Items::replace = new Fl_Button(304, y1, 96, 32, "Replace");
    Items::replace->callback((Fl_Callback *)replaceColor);
    Items::insert->labelsize(18);
    y1 += 32 + 8;

    new Separator(Items::dialog, 298, y1, 108, 2, "");
    y1 += 8;

    Items::undo = new Fl_Button(304, y1, 96, 32, "Undo");
    Items::undo->callback((Fl_Callback *)getUndo);
    y1 += 32 + 8;
    
    new Separator(Items::dialog, 298, y1, 108, 2, "");
    y1 += 8;

    Items::rgb_ramp = new Fl_Button(304, y1, 96, 24, "RGB Ramp");
    Items::rgb_ramp->callback((Fl_Callback *)rgbRamp);
    y1 += 24 + 8;

    Items::hsv_ramp = new Fl_Button(304, y1, 96, 24, "HSV Ramp");
    Items::hsv_ramp->callback((Fl_Callback *)hsvRamp);
    y1 += 24 + 8;

    new Separator(Items::dialog, 298, y1, 108, 2, "");
    y1 += 8;

    Items::color = new Widget(Items::dialog, 304, y1, 96, 48, "Paint Color", 0, 0, 0);

    Items::palette = new Widget(Items::dialog, 408, 8, 64, 256,
                                "", 24, 24, (Fl_Callback *)checkPalette);

    new Separator(Items::dialog, 2, 272, Items::dialog->w() - 4, 2, "");

    x1 = 8;
    y1 = 272 + 8;

    Items::hexcolor = new InputText(Items::dialog, x1, y1, 80, 24, "Hexadecimal", (Fl_Callback *)checkHexColor);
    Items::hexcolor->maximum_size(6);
    Items::hexcolor->labelsize(12);
    Items::hexcolor->textsize(14);
    Items::hexcolor->textfont(FL_COURIER);
    Items::hexcolor->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
    Items::hexcolor->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
    x1 += 80 + 8;

    Items::hexcolor_web = new InputText(Items::dialog, x1, y1, 64, 24, "Shorthand", (Fl_Callback *)checkHexColorWeb);
    Items::hexcolor_web->maximum_size(3);
    Items::hexcolor_web->labelsize(12);
    Items::hexcolor_web->textsize(14);
    Items::hexcolor_web->textfont(FL_COURIER);
    Items::hexcolor_web->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
    Items::hexcolor_web->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
    x1 += 64 + 8;

    new Separator(Items::dialog, x1, y1 - 5, 2, 46, "");
    x1 += 8;

    Items::done = new Fl_Button(Items::dialog->w() - 96 - 8, 280, 96, 36, "Done (E)");
    Items::done->shortcut('e');
    Items::done->callback((Fl_Callback *)close);

    y1 += 32 + 10;
    Items::info = new Group(0, y1, 352, 24, "");
    Items::info_text = new Fl_Box(FL_NO_BOX, Items::info->x(), Items::info->y(), Items::info->w(), Items::info->h(), "");
    Items::info_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    Items::info_text->labelsize(12);

    Items::index = new Group(352, y1, 128, 24, "");
    Items::index_text = new Fl_Box(FL_NO_BOX, Items::index->x(), Items::index->y(), Items::index->w(), Items::index->h(), "");
    Items::index_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

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
    Widget *icon;
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

    Items::dialog = new DialogWindow(448, 0, "Error");
    Items::box = new Fl_Box(FL_FLAT_BOX, 64 + 8, 8, Items::dialog->w() - 32, 64, "");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    Items::box->labelsize(14); 
    Items::icon = new Widget(Items::dialog, 8, 8, 64, 64, "", images_dialog_info_png);
    y1 += 64 + 8;
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
    Widget *icon;
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

    Items::dialog = new DialogWindow(448, 0, "Error");
    Items::box = new Fl_Box(FL_FLAT_BOX, 64 + 8, 8, Items::dialog->w() - 32, 64, "");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    Items::box->labelsize(14); 
    Items::icon = new Widget(Items::dialog, 8, 8, 64, 64, "", images_dialog_question_png);
    y1 += 64 + 8;
    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);

    Items::ok->copy_label("Yes");
    Items::ok->callback((Fl_Callback *)close);

    Items::cancel->copy_label("No");
    Items::cancel->callback((Fl_Callback *)quit);
    Items::cancel->shortcut(FL_Escape);

    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

void Dialog::init()
{
  About::init();
  JpegQuality::init();
  JavaExport::init();
  PngOptions::init();
  TextOptions::init();
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

void Dialog::javaExport()
{
  JavaExport::begin();
}

int Dialog::javaExportOption()
{
  return JavaExport::Items::option->value();
}

void Dialog::pngOptions()
{
  PngOptions::begin();
}

void Dialog::textOptions()
{
  TextOptions::begin();
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

