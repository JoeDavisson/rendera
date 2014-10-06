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

#include "Transform.H"
#include "Bitmap.H"
#include "Map.H"
#include "InputInt.H"
#include "InputFloat.H"
#include "Separator.H"
#include "Undo.H"
#include "Gui.H"
#include "View.H"
#include "Dialog.H"

extern int *fix_gamma;
extern int *unfix_gamma;

namespace
{
  Bitmap *bmp;
  int overscroll;

  void pushUndo()
  {
    bmp = Bitmap::main;
    overscroll = bmp->overscroll;
    Undo::push(overscroll, overscroll,
               bmp->w - overscroll * 2, bmp->h - overscroll * 2, 1);
  }

  void beginProgress()
  {
    bmp = Bitmap::main;
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

namespace Scale
{
  Fl_Double_Window *dialog;
  InputInt *width;
  InputInt *height;
  Fl_Check_Button *wrap;
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
    pushUndo();

    Bitmap *bmp = Bitmap::main;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    bmp->scaleBilinear(temp,
                       overscroll, overscroll,
                       bmp->w - overscroll * 2, bmp->h - overscroll * 2,
                       overscroll, overscroll, w, h,
                       wrap->value()); 

    delete Bitmap::main;
    Bitmap::main = temp;

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

  void init()
  {
    dialog = new Fl_Double_Window(200, 144, "Scale Image");
    width = new InputInt(dialog, 88, 8, 72, 24, "Width:", 0);
    height = new InputInt(dialog, 88, 40, 72, 24, "Height:", 0);
    width->maximum_size(8);
    height->maximum_size(8);
    width->value("640");
    height->value("480");
    wrap = new Fl_Check_Button(32, 80, 16, 16, "Wrap Edges");
    new Separator(dialog, 2, 104, 196, 2, "");
    ok = new Fl_Button(56, 112, 64, 24, "OK");
    ok->callback((Fl_Callback *)close);
    cancel = new Fl_Button(128, 112, 64, 24, "Cancel");
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace Rotate
{
  Fl_Double_Window *dialog;
  InputFloat *angle;
  InputFloat *scale;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "0");
    angle->value(s);
    dialog->show();
  }

  void close()
  {
    char s[8];

    float angle_val = atof(angle->value());

    if(angle_val < -359.99f)
    {
      snprintf(s, sizeof(s), "%.2f", -359.99f);
      angle->value(s);
      return;
    }

    if(angle_val > 359.99f)
    {
      snprintf(s, sizeof(s), "%.2f", 359.99f);
      angle->value(s);
      return;
    }

    float scale_val = atof(scale->value());

    if(scale_val < .01f)
    {
      snprintf(s, sizeof(s), "%.2f", .01f);
      scale->value(s);
      return;
    }

    if(scale_val > 10.0f)
    {
      snprintf(s, sizeof(s), "%.2f", 10.0f);
      scale->value(s);
      return;
    }

    dialog->hide();
    int overscroll = Bitmap::main->overscroll;
    Bitmap *temp = Bitmap::main->rotate(angle_val, scale_val, overscroll);

    delete Bitmap::main;
    Bitmap::main = temp;

    delete Map::main;
    Map::main = new Map(Bitmap::main->w, Bitmap::main->h);
  }

  void quit()
  {
    dialog->hide();
  }

  void init()
  {
    dialog = new Fl_Double_Window(200, 112, "Rotate Image");
    angle = new InputFloat(dialog, 80, 8, 72, 24, "Angle:", 0);
    angle->value("0");
    scale = new InputFloat(dialog, 80, 40, 72, 24, "Scale:", 0);
    scale->value("1.0");
    new Separator(dialog, 2, 72, 196, 2, "");
    ok = new Fl_Button(56, 80, 64, 24, "OK");
    ok->callback((Fl_Callback *)close);
    cancel = new Fl_Button(128, 80, 64, 24, "Cancel");
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

void Transform::init()
{
  Scale::init();
  Rotate::init();
}

void Transform::scale()
{
  Scale::begin();
}

void Transform::mirror()
{
  pushUndo();
  Bitmap::main->mirror();
  Gui::getView()->drawMain(1);
}

void Transform::flip()
{
  pushUndo();
  Bitmap::main->flip();
  Gui::getView()->drawMain(1);
}

void Transform::rotate()
{
  pushUndo();
  Rotate::begin();
  Gui::getView()->drawMain(1);
}

