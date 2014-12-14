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

#include "Bitmap.H"
#include "Dialog.H"
#include "Gui.H"
#include "InputFloat.H"
#include "InputInt.H"
#include "Map.H"
#include "Project.H"
#include "Separator.H"
#include "Transform.H"
#include "Undo.H"
#include "View.H"

namespace
{
  Bitmap *bmp;
  int overscroll;

  void pushUndo()
  {
    bmp = Project::bmp;
    overscroll = bmp->overscroll;
    Undo::push();
  }

  void beginProgress()
  {
    bmp = Project::bmp;
    Dialog::showProgress(bmp->h / 64);
  }

  void endProgress()
  {
    Dialog::hideProgress();
    Gui::getView()->drawMain(true);
  }

  int updateProgress(const int y)
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
      Gui::getView()->drawMain(true);
      Dialog::updateProgress();
    }

    return 0;
  }
}

namespace Resize
{
  Fl_Double_Window *dialog;
  InputInt *option_width;
  InputInt *option_height;
  Fl_Check_Button *option_keep_aspect;
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

  void checkWidth()
  {
    if(option_keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[8];
      int w = atoi(option_width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      option_height->value(s);
    }
  }

  void checkHeight()
  {
    if(option_keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[8];
      int h = atoi(option_height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      option_width->value(s);
    }
  }

  void checkKeepAspect()
  {
    if(option_keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(option_width->limitValue(1, 10000) < 0)
      return;

    if(option_height->limitValue(1, 10000) < 0)
      return;

    int w = atoi(option_width->value());
    int h = atoi(option_height->value());

    dialog->hide();
    pushUndo();

    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    bmp->blit(temp, overscroll, overscroll, overscroll, overscroll,
                    bmp->cw, bmp->ch);

    delete Project::bmp;
    Project::bmp = temp;

    delete Project::map;
    Project::map = new Map(Project::bmp->w, Project::bmp->h);

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

    dialog = new Fl_Double_Window(256, 0, "Resize Image");
    option_width = new InputInt(dialog, 0, y1, 72, 24, "Width:", 0);
    option_width->center();
    option_width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    option_height = new InputInt(dialog, 0, y1, 72, 24, "Height:", 0);
    option_height->center();
    option_height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    option_width->maximum_size(8);
    option_height->maximum_size(8);
    option_width->value("640");
    option_height->value("480");
    option_keep_aspect = new Fl_Check_Button(0, y1, 16, 16, "Keep Aspect");
    option_keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Dialog::center(option_keep_aspect);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace Scale
{
  Fl_Double_Window *dialog;
  InputInt *option_width;
  InputInt *option_height;
  Fl_Check_Button *option_keep_aspect;
  Fl_Check_Button *option_wrap;
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

  void checkWidth()
  {
    if(option_keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[8];
      int w = atoi(option_width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      option_height->value(s);
    }
  }

  void checkHeight()
  {
    if(option_keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[8];
      int h = atoi(option_height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      option_width->value(s);
    }
  }

  void checkKeepAspect()
  {
    if(option_keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(option_width->limitValue(1, 10000) < 0)
      return;

    if(option_height->limitValue(1, 10000) < 0)
      return;

    int w = atoi(option_width->value());
    int h = atoi(option_height->value());

    dialog->hide();
    pushUndo();

    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    bmp->scaleBilinear(temp,
                       overscroll, overscroll,
                       bmp->cw, bmp->ch,
                       overscroll, overscroll, w, h,
                       option_wrap->value()); 

    delete Project::bmp;
    Project::bmp = temp;

    delete Project::map;
    Project::map = new Map(Project::bmp->w, Project::bmp->h);

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

    dialog = new Fl_Double_Window(256, 0, "Scale Image");
    option_width = new InputInt(dialog, 0, y1, 72, 24, "Width:", 0);
    option_width->center();
    option_width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    option_height = new InputInt(dialog, 0, y1, 72, 24, "Height:", 0);
    option_height->center();
    option_height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    option_width->maximum_size(8);
    option_height->maximum_size(8);
    option_width->value("640");
    option_height->value("480");
    option_keep_aspect = new Fl_Check_Button(0, y1, 16, 16, "Keep Aspect");
    option_keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Dialog::center(option_keep_aspect);
    option_wrap = new Fl_Check_Button(0, y1, 16, 16, "Wrap Edges");
    y1 += 16 + 8;
    Dialog::center(option_wrap);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

namespace Rotate
{
  Fl_Double_Window *dialog;
  InputFloat *option_angle;
  InputFloat *option_scale;
  Fl_Check_Button *option_tile;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "0");
    option_angle->value(s);
    dialog->show();
  }

  void close()
  {
    if(option_angle->limitValue(-359.99, 359.99) < 0)
      return;

    if(option_scale->limitValue(.1, 10.0) < 0)
      return;

    dialog->hide();
    pushUndo();

    int overscroll = Project::bmp->overscroll;
    const float angle = atof(option_angle->value());
    const float scale = atof(option_scale->value());

    Bitmap *temp = Project::bmp->rotate(angle, scale, overscroll,
                                        option_tile->value());

    delete Project::bmp;
    Project::bmp = temp;

    delete Project::map;
    Project::map = new Map(Project::bmp->w, Project::bmp->h);

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

    dialog = new Fl_Double_Window(256, 0, "Rotate Image");
    option_angle = new InputFloat(dialog, 0, y1, 72, 24, "Angle:", 0);
    option_angle->center();
    y1 += 24 + 8;
    option_angle->value("0");
    option_scale = new InputFloat(dialog, 0, y1, 72, 24, "Scale:", 0);
    option_scale->center();
    y1 += 24 + 8;
    option_scale->value("1.0");
    option_tile = new Fl_Check_Button(0, y1, 16, 16, "Tile");
    y1 += 16 + 8;
    Dialog::center(option_tile);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 
  }
}

void Transform::init()
{
  Resize::init();
  Scale::init();
  Rotate::init();
}

void Transform::mirror()
{
  pushUndo();
  Project::bmp->mirror();
  Gui::getView()->drawMain(true);
}

void Transform::flip()
{
  pushUndo();
  Project::bmp->flip();
  Gui::getView()->drawMain(true);
}

void Transform::resize()
{
  Resize::begin();
}

void Transform::scale()
{
  Scale::begin();
}

void Transform::rotate()
{
  Rotate::begin();
}

