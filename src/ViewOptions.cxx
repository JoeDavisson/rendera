/*
Copyright (c) 2025 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
state WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <cstdlib>

#include "Button.H"
#include "CheckBox.H"
#include "Group.H"
#include "InputInt.H"
#include "Project.H"
#include "StaticText.H"
#include "Gui.H"
#include "Images.H"
#include "View.H"
#include "Separator.H"
#include "ToggleButton.H"
#include "Tool.H"
#include "ViewOptions.H"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

namespace
{
  void cb_zoomIn(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->zoomIn(); }

  void cb_zoomOut(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->zoomOut(); }
 
  void cb_zoomOne(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->zoomOne(); }

  void cb_gridEnable(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->gridEnable(); }

  void cb_gridSnap(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->gridSnap(); }

  void cb_gridX(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->gridX(); }

  void cb_gridY(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->gridY(); }

  void cb_aspectMode(Fl_Widget *w, void *data) { ViewOptions *temp = (ViewOptions *)data; temp->aspectMode(); }
}

ViewOptions::ViewOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Gui::SPACING;

  zoom_one = new Button(this, pos, 8, 40, 40,
                        "Actual Size (1)", images_zoom_one_png, 0);
  zoom_one->callback(cb_zoomOne, (void *)this);

  pos += 40 + Gui::SPACING;

  zoom_in = new Button(this, pos, 8, 40, 40,
                       "Zoom In (+)", images_zoom_in_png, 0);
  zoom_in->callback(cb_zoomIn, (void *)this);

  pos += 40 + Gui::SPACING;

  zoom_out = new Button(this, pos, 8, 40, 40,
                        "Zoom Out (-)", images_zoom_out_png, 0);
  zoom_out->callback(cb_zoomOut, (void *)this);

  pos += 40 + Gui::SPACING;

  zoom = new StaticText(this, pos, 8, 64, 40, "");
  zoom->labelsize(20);
  pos += 64 + Gui::SPACING;

  new Separator(this, pos, 0, Gui::TOP_HEIGHT, Separator::VERTICAL, "");
  pos += 4 + Gui::SPACING;

  grid = new ToggleButton(this, pos, 8, 40, 40,
                          "Show Grid", images_grid_png, 0);
  grid->callback(cb_gridEnable, (void *)this);

  pos += 40 + Gui::SPACING;

  gridsnap = new ToggleButton(this, pos, 8, 40, 40,
                          "Snap to Grid", images_gridsnap_png, 0);
  gridsnap->callback(cb_gridSnap, (void *)this);

  pos += 96; 

  gridx = new InputInt(this, pos, 8, 104, 40, "X:", 0, 1, 256);
  gridx->callback(cb_gridX, (void *)this);

  gridx->labelsize(18);
  gridx->textsize(18);
  gridx->value(8);
  pos += 104 + 48;

  gridy = new InputInt(this, pos, 8, 104, 40, "Y:", 0, 1, 256);
  gridy->callback(cb_gridY, (void *)this);

  gridy->labelsize(18);
  gridy->textsize(18);
  gridy->value(8);
  pos += 104 + Gui::SPACING;

  new Separator(this, pos, 0, Gui::TOP_HEIGHT, Separator::VERTICAL, "");
  pos += 4 + Gui::SPACING;

  aspect = new Fl_Choice(pos, 8, 160, 40, "");
  aspect->tooltip("Aspect Ratio\n(simulates non-square displays)");
  aspect->textsize(10);
  aspect->resize(this->x() + pos, this->y() + 8, 160, 40);
  aspect->add("Normal (1:1)");
  aspect->add("Wide (2:1)");
  aspect->add("Tall (1:2)");
  aspect->value(0);
  aspect->callback(cb_aspectMode, (void *)this);
  aspect->textsize(16);
  pos += 160 + Gui::SPACING;

  new Separator(this, pos, 0, Gui::TOP_HEIGHT, Separator::VERTICAL, "");
  pos += 4 + Gui::SPACING;

  resizable(0);
  end();
}

ViewOptions::~ViewOptions()
{
}

void ViewOptions::zoomIn()
{ 
  View *view = Gui::view;

  view->zoomIn(view->w() / 2, view->h() / 2);
  zoomLevel();
}            
                        
void ViewOptions::zoomOut()
{         
  View *view = Gui::view;

  view->zoomOut(view->w() / 2, view->h() / 2);
  zoomLevel();
} 
              
void ViewOptions::zoomOne()
{ 
  View *view = Gui::view;

  view->zoomOne();
  zoomLevel();
}            
                   
void ViewOptions::zoomLevel()
{             
  char s[256];
                          
  View *view = Gui::view;

  if (view->zoom < 1)
    snprintf(s, sizeof(s), "1/%1.0fx", 1.0 / view->zoom);
  else
    snprintf(s, sizeof(s), "%2.1fx", view->zoom);

  zoom->copy_label(s);
  zoom->redraw();
}

void ViewOptions::gridEnable()
{
  View *view = Gui::view;

  view->grid = grid->var;
  view->drawMain(true);
  view->redraw();

  if (Project::tool->isActive())
  {
    Project::tool->redraw(view);
  }
}

void ViewOptions::gridSnap()
{
  View *view = Gui::view;

  view->gridsnap = gridsnap->var;
}

void ViewOptions::gridX()
{
  View *view = Gui::view;

  view->gridx = gridx->value();
  view->drawMain(true);
}

void ViewOptions::gridY()
{
  View *view = Gui::view;

  view->gridy = gridy->value();
  view->drawMain(true);
}

void ViewOptions::aspectMode()
{
  View *view = Gui::view;

  view->changeAspect(aspect->value());
}

