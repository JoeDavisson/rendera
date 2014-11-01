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
#include "Win.H"

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
      Gui::getView()->drawMain(true);
      Dialog::updateProgress();
    }

    return 0;
  }
}

namespace Scale
{
  Win *dialog;
  InputInt *width;
  InputInt *height;
  Fl_Check_Button *keep_aspect;
  Fl_Check_Button *wrap;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Project::bmp->cw);
    width->value(s);
    snprintf(s, sizeof(s), "%d", Project::bmp->ch);
    height->value(s);
    dialog->show();
  }

  void checkWidth()
  {
    if(keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)hh / ww;
      char s[8];
      int w = atoi(width->value());
      int h = w * aspect;
      snprintf(s, sizeof(s), "%d", h);
      height->value(s);
    }
  }

  void checkHeight()
  {
    if(keep_aspect->value())
    {
      int ww = Project::bmp->cw;
      int hh = Project::bmp->ch;
      float aspect = (float)ww / hh;
      char s[8];
      int h = atoi(height->value());
      int w = h * aspect;
      snprintf(s, sizeof(s), "%d", w);
      width->value(s);
    }
  }

  void checkKeepAspect()
  {
    if(keep_aspect->value())
    {
      checkWidth();
    }
  }

  void close()
  {
    if(width->limitValue(1, 10000) < 0)
      return;

    if(height->limitValue(1, 10000) < 0)
      return;

    int w = atoi(width->value());
    int h = atoi(height->value());

    dialog->hide();
    pushUndo();

    Bitmap *bmp = Project::bmp;
    int overscroll = bmp->overscroll;
    Bitmap *temp = new Bitmap(w, h, overscroll);

    bmp->scaleBilinear(temp,
                       overscroll, overscroll,
                       bmp->cw, bmp->ch,
                       overscroll, overscroll, w, h,
                       wrap->value()); 

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

  int *init()
  {
    int y1 = 8;

    dialog = new Win(256, 0, "Scale Image");
    width = new InputInt(dialog, 0, y1, 72, 24, "Width:", 0);
    width->center();
    width->callback((Fl_Callback *)checkWidth);
    y1 += 24 + 8;
    height = new InputInt(dialog, 0, y1, 72, 24, "Height:", 0);
    height->center();
    height->callback((Fl_Callback *)checkHeight);
    y1 += 24 + 8;
    width->maximum_size(8);
    height->maximum_size(8);
    width->value("640");
    height->value("480");
    keep_aspect = new Fl_Check_Button(0, y1, 16, 16, "Keep Aspect");
    keep_aspect->callback((Fl_Callback *)checkKeepAspect);
    y1 += 16 + 8;
    Dialog::center(keep_aspect);
    wrap = new Fl_Check_Button(0, y1, 16, 16, "Wrap Edges");
    y1 += 16 + 8;
    Dialog::center(wrap);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 

    return 0;
  }

  static const int *temp = init();
}

namespace Rotate
{
  Win *dialog;
  InputFloat *angle;
  InputFloat *scale;
  Fl_Check_Button *tile;
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
    if(angle->limitValue(-359.99, 359.99) < 0)
      return;

    if(scale->limitValue(.1, 10.0) < 0)
      return;

    float angle_val = atof(angle->value());
    float scale_val = atof(scale->value());

    dialog->hide();
    pushUndo();

    int overscroll = Project::bmp->overscroll;
    Bitmap *temp = Project::bmp->rotate(angle_val, scale_val, overscroll,
                                        tile->value());

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

  int *init()
  {
    int y1 = 8;

    dialog = new Win(256, 0, "Rotate Image");
    angle = new InputFloat(dialog, 0, y1, 72, 24, "Angle:", 0);
    angle->center();
    y1 += 24 + 8;
    angle->value("0");
    scale = new InputFloat(dialog, 0, y1, 72, 24, "Scale:", 0);
    scale->center();
    y1 += 24 + 8;
    scale->value("1.0");
    tile = new Fl_Check_Button(0, y1, 16, 16, "Tile");
    y1 += 16 + 8;
    Dialog::center(tile);
    Dialog::addOkCancelButtons(dialog, &ok, &cancel, &y1);
    ok->callback((Fl_Callback *)close);
    cancel->callback((Fl_Callback *)quit);
    dialog->set_modal();
    dialog->end(); 

    return 0;
  }

  static const int *temp = init();
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

void Transform::scale()
{
  Scale::begin();
}

void Transform::rotate()
{
  Rotate::begin();
}

