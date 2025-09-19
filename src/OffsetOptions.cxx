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

#include "Button.H"
#include "Bitmap.H"
#include "Gui.H"
#include "Images.H"
#include "OffsetOptions.H"
#include "Project.H"
#include "RepeatButton.H"
#include "Separator.H"
#include "StaticText.H"
#include "Tool.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Group.H>

namespace
{
  void cb_left(Fl_Widget *w, void *data) { OffsetOptions *temp = (OffsetOptions *)data; temp->left(); }

  void cb_right(Fl_Widget *w, void *data) { OffsetOptions *temp = (OffsetOptions *)data; temp->right(); }

  void cb_up(Fl_Widget *w, void *data) { OffsetOptions *temp = (OffsetOptions *)data; temp->up(); }

  void cb_down(Fl_Widget *w, void *data) { OffsetOptions *temp = (OffsetOptions *)data; temp->down(); }
}

OffsetOptions::OffsetOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  new StaticText(this, 32 + 8, pos, 32, 24, "x:");
  offset_x = new StaticText(this, 32 + 24, pos, 72, 24, 0);
  pos += 24;

  new StaticText(this, 32 + 8, pos, 32, 24, "y:");
  offset_y = new StaticText(this, 32 + 24, pos, 72, 24, 0);
  pos += 24 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  offset_left = new RepeatButton(this, 16, pos + 26, 40, 40, "",
                                 images_left_png, 0);
  offset_left->callback(cb_left, (void *)this);

  offset_up = new RepeatButton(this, 68, pos, 40, 40, "",
                               images_up_png, 0);
  offset_up->callback(cb_up, (void *)this);

  offset_right = new RepeatButton(this, 120, pos + 26, 40, 40, "",
                                  images_right_png, 0);
  offset_right->callback(cb_right, (void *)this);

  offset_down = new RepeatButton(this, 68, pos + 52, 40, 40, "",
                                 images_down_png, 0);
  offset_down->callback(cb_down, (void *)this);

  pos += 92;

  new StaticText(this, 8, pos, 160, 32, "Nudge");

  values(0, 0);

  resizable(0);
  end();
}

OffsetOptions::~OffsetOptions()
{
}

void OffsetOptions::values(int x, int y)
{
  char s[256];

  snprintf(s, sizeof(s), "%d", x);
  offset_x->copy_label(s);
  offset_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  offset_y->copy_label(s);
  offset_y->redraw();
}

void OffsetOptions::left()
{
  View *view = Gui::view;

  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgx = view->gridsnap ? -view->gridx : -1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void OffsetOptions::right()
{
  View *view = Gui::view;

  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgx = view->gridsnap ? view->gridx : 1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void OffsetOptions::up()
{
  View *view = Gui::view;

  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgy = view->gridsnap ? -view->gridy : -1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void OffsetOptions::down()
{
  View *view = Gui::view;

  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgy = view->gridsnap ? view->gridy : 1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

