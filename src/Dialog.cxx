/*
Copyright (c) 2024 Joe Davisson.

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
    int y1 = 16;
    int ww = 0, hh = 0;
    const char *credits = "\nCopyright (c) 2024 Joe Davisson.\n\nRendera is based in part on the work\nof the FLTK project (http://www.fltk.org).";

    Items::dialog = new DialogWindow(400, 0, "About");

    Items::logo = new Widget(Items::dialog, 400 / 2 - 320 / 2, y1, 320, 96,
                             credits, images_logo_dark_png, -1, -1, 0);

    Items::logo->align(FL_ALIGN_BOTTOM);
    Items::logo->tooltip(0);
    Items::logo->measure_label(ww, hh);
    y1 += 96 + 16 + hh;
    Items::box = new Fl_Box(FL_FLAT_BOX, 0, y1, 400, 32, PACKAGE_STRING);
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    Items::box->labelsize(16);
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

    while (true)
    {
      Fl_Widget *action = Fl::readqueue();

      if (!action)
      {
        Fl::wait();
      }
      else if (action == Items::ok)
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

    while (true)
    {
      Fl_Widget *action = Fl::readqueue();

      if (!action)
      {
        Fl::wait();
      }
      else if (action == Items::ok)
      {
        Items::dialog->hide();
        break;
      }
    }
  }

  void init()
  {
    int y1 = 16;

    Items::dialog = new DialogWindow(400, 0, "Export Java Array");

    Items::option = new Fl_Choice(64, y1, 128, 32, "");
    Items::option->tooltip("Format");
    Items::option->add("C64 Characters");
    Items::option->add("C64 Sprites");
    Items::option->value(0);
    y1 += 32 + 16;

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

    while (true)
    {
      Fl_Widget *action = Fl::readqueue();

      if (!action)
      {
        Fl::wait();
      }
      else if (action == Items::ok)
      {
        Items::dialog->hide();
        break;
      }
    }
  }

  void init()
  {
    int y1 = 16;

    Items::dialog = new DialogWindow(400, 0, "PNG Options");
    Items::dialog->callback(closeCallback);

    Items::alpha_levels = new InputInt(Items::dialog, 0, y1, 128, 32, "Alpha Levels:", 0, 2, 16);
    Items::alpha_levels->value("2");
    Items::alpha_levels->center();
    y1 += 32 + 16;

    Items::use_palette = new CheckBox(Items::dialog, 0, y1, 16, 16, "Use Current Palette", 0);
    y1 += 16 + 16;
    Items::use_palette->center();

    Items::use_alpha = new CheckBox(Items::dialog, 0, y1, 16, 16, "Save Alpha Channel", 0);
    Items::use_alpha->value(1);
    Items::use_alpha->center();
    y1 += 16 + 16;

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
    CheckBox *keep_aspect;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void begin()
  {
    char s[16];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    Items::width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    Items::height->value(s);
    Items::dialog->show();
  }

  void checkWidth()
  {
    if (Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[16];
      int w = atoi(Items::width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      Items::height->value(s);
    }
  }

  void checkHeight()
  {
    if (Items::keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[16];
      int h = atoi(Items::height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      Items::width->value(s);
    }
  }

  void close()
  {
    Items::dialog->hide();

    if (Project::newImage(atoi(Items::width->value()),
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
    int y1 = 16;

    Items::dialog = new DialogWindow(400, 0, "New Image");

    Items::width = new InputInt(Items::dialog, 0, y1, 128, 32, "Width", (Fl_Callback *)checkWidth, 1, 10000);
    Items::width->center();
    Items::width->maximum_size(8);
    y1 += 32 + 16;

    Items::height = new InputInt(Items::dialog, 0, y1, 128, 32, "Height", (Fl_Callback *)checkHeight, 1, 10000);
    Items::height->center();
    Items::height->maximum_size(8);
    y1 += 32 + 16;

    Items::keep_aspect = new CheckBox(Items::dialog, 0, y1, 16, 16, "Keep Aspect", 0);
    Items::keep_aspect->center();
    y1 += 16 + 16;

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
    char s[16];
    snprintf(s, sizeof(s), "%d", Project::palette->max);
    Items::colors->value(s);
    Items::dialog->show();
  }

  void close()
  {
    Items::dialog->hide();
    Quantize::pca(Project::bmp, Project::palette, atoi(Items::colors->value()));
    Gui::paletteDraw();
    Project::palette->fillTable();
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 16;

    Items::dialog = new DialogWindow(400, 0, "Create Palette");

    Items::colors = new InputInt(Items::dialog, 0, y1, 128, 32, "Colors:", 0, 1, 256);
    Items::colors->center();
    y1 += 32 + 16;

    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);

    Items::dialog->set_modal();
    Items::dialog->end(); 
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
    int y1 = 16;

    Items::dialog = new DialogWindow(448, 0, "Error");

    Items::box = new Fl_Box(FL_FLAT_BOX, 64 + 8, 8, Items::dialog->w() - 32, 64, "");
    Items::box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    Items::icon = new Widget(Items::dialog, 8, 8, 64, 64, "", images_dialog_info_png);
    y1 += 64 + 16;

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
  NewImage::init();
  MakePalette::init();
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

void Dialog::message(const char *title, const char *message)
{
  Message::begin(title, message);
}

bool Dialog::choice(const char *title, const char *message)
{
  Choice::begin(title, message);

  while (Choice::Items::dialog->shown())
  {
    Fl::wait();
  }

  return Choice::yes;
}

