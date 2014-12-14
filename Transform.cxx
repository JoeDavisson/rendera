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
  namespace Items
  {
    Fl_Double_Window *dialog;
    InputInt *width;
    InputInt *height;
    Fl_Check_Button *keep_aspect;
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

  void checkKeepAspect()
  {
    if(Items::keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(Items::width->limitValue(1, 10000) < 0)
      return;

    if(Items::height->limitValue(1, 10000) < 0)
      return;

    int w = atoi(Items::width->value());
    int h = atoi(Items::height->value());

    Items::dialog->hide();
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
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new Fl_Double_Window(256, 0, "Resize Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 72, 24, "Width:", 0);
    Items::width->center();
    Items::width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 72, 24, "Height:", 0);
    Items::height->center();
    Items::height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::keep_aspect = new Fl_Check_Button(0, y1, 16, 16, "Keep Aspect");
    Items::keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Dialog::center(Items::keep_aspect);
    Dialog::addOkCancelButtons(Items::dialog, &Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace Scale
{
  namespace Items
  {
    Fl_Double_Window *dialog;
    InputInt *width;
    InputInt *height;
    Fl_Check_Button *keep_aspect;
    Fl_Check_Button *wrap;
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

  void checkKeepAspect()
  {
    if(Items::keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(Items::width->limitValue(1, 10000) < 0)
      return;

    if(Items::height->limitValue(1, 10000) < 0)
      return;

    int w = atoi(Items::width->value());
    int h = atoi(Items::height->value());

    Items::dialog->hide();
    pushUndo();

    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    bmp->scaleBilinear(temp,
                       overscroll, overscroll,
                       bmp->cw, bmp->ch,
                       overscroll, overscroll, w, h,
                       Items::wrap->value()); 

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
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new Fl_Double_Window(256, 0, "Scale Image");
    Items::width = new InputInt(Items::dialog, 0, y1, 72, 24, "Width:", 0);
    Items::width->center();
    Items::width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    Items::height = new InputInt(Items::dialog, 0, y1, 72, 24, "Height:", 0);
    Items::height->center();
    Items::height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    Items::width->maximum_size(8);
    Items::height->maximum_size(8);
    Items::width->value("640");
    Items::height->value("480");
    Items::keep_aspect = new Fl_Check_Button(0, y1, 16, 16, "Keep Aspect");
    Items::keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Dialog::center(Items::keep_aspect);
    Items::wrap = new Fl_Check_Button(0, y1, 16, 16, "Wrap Edges");
    y1 += 16 + 8;
    Dialog::center(Items::wrap);
    Dialog::addOkCancelButtons(Items::dialog, &Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
  }
}

namespace Rotate
{
  namespace Items
  {
    Fl_Double_Window *dialog;
    InputFloat *angle;
    InputFloat *scale;
    Fl_Check_Button *tile;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "0");
    Items::angle->value(s);
    Items::dialog->show();
  }

  void close()
  {
    if(Items::angle->limitValue(-359.99, 359.99) < 0)
      return;

    if(Items::scale->limitValue(.1, 10.0) < 0)
      return;

    Items::dialog->hide();
    pushUndo();

    int overscroll = Project::bmp->overscroll;
    const float angle = atof(Items::angle->value());
    const float scale = atof(Items::scale->value());

    Bitmap *temp = Project::bmp->rotate(angle, scale, overscroll,
                                        Items::tile->value());

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
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;

    Items::dialog = new Fl_Double_Window(256, 0, "Rotate Image");
    Items::angle = new InputFloat(Items::dialog, 0, y1, 72, 24, "Angle:", 0);
    Items::angle->center();
    y1 += 24 + 8;
    Items::angle->value("0");
    Items::scale = new InputFloat(Items::dialog, 0, y1, 72, 24, "Scale:", 0);
    Items::scale->center();
    y1 += 24 + 8;
    Items::scale->value("1.0");
    Items::tile = new Fl_Check_Button(0, y1, 16, 16, "Tile");
    y1 += 16 + 8;
    Dialog::center(Items::tile);
    Dialog::addOkCancelButtons(Items::dialog, &Items::ok, &Items::cancel, &y1);
    Items::ok->callback((Fl_Callback *)close);
    Items::cancel->callback((Fl_Callback *)quit);
    Items::dialog->set_modal();
    Items::dialog->end(); 
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

