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
#include "Clone.H"
#include "FillOptions.H"
#include "Gui.H"
#include "Images.H"
#include "Map.H"
#include "OffsetOptions.H"
#include "PaintOptions.H"
#include "PickerOptions.H"
#include "Project.H"
#include "Selection.H"
#include "SelectionOptions.H"
#include "Separator.H"
#include "StaticText.H"
#include "Stroke.H"
#include "TextOptions.H"
#include "ToggleButton.H"
#include "Tool.H"
#include "ToolOptions.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Group.H>

namespace
{
  Widget *tool;
  ToggleButton *clone;
  ToggleButton *origin;
  ToggleButton *constrain;

  void cb_change(Fl_Widget *w, void *data) { ToolOptions *temp = (ToolOptions *)data; temp->change((Widget *)w, data); }

  void cb_cloneEnable(Fl_Widget *w, void *data) { ToolOptions *temp = (ToolOptions *)data; temp->cloneEnable((Widget *)w, data); }

  void cb_constrainEnable(Fl_Widget *w, void *data) { ToolOptions *temp = (ToolOptions *)data; temp->constrainEnable((Widget *)w, data); }

  void cb_originEnable(Fl_Widget *w, void *data) { ToolOptions *temp = (ToolOptions *)data; temp->originEnable((Widget *)w, data); }
}

ToolOptions::ToolOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  tool = new Widget(this, 8, pos, 48, 6 * 48,
                    "Tools", images_tools_png, 48, 48,
                    (Fl_Callback *)cb_change);

  pos += 6 * 48 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::TOOLS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  clone = new ToggleButton(this, 8, pos, 48, 48,
                           "Clone (Ctrl+Click to set target)",
                           images_clone_png,
                           (Fl_Callback *)cb_cloneEnable);

  pos += 48 + 8;

  origin = new ToggleButton(this, 8, pos, 48, 48,
                            "Start From Center", images_origin_png,
                            (Fl_Callback *)cb_originEnable);

  pos += 48 + 8;

  constrain = new ToggleButton(this, 8, pos, 48, 48,
                              "Lock Proportions",
                              images_constrain_png,
                              (Fl_Callback *)cb_constrainEnable);

  resizable(0);
  end();
}

ToolOptions::~ToolOptions()
{
}

void ToolOptions::init()
{
  tool->do_callback();
}

void ToolOptions::change(int t)
{
  tool->var = t;
  tool->do_callback();
}

void ToolOptions::change(Widget *, void *var)
{
  int tool = *(int *)var;

  if (tool != Tool::PAINT)
    Gui::paint->hide();
  if (tool != Tool::PICKER)
    Gui::picker->hide();
  if (tool != Tool::SELECT)
    Gui::selection->hide();
  if (tool != Tool::OFFSET)
    Gui::offset->hide();
  if (tool != Tool::TEXT)
    Gui::text->hide();
  if (tool != Tool::FILL)
    Gui::fill->hide();

  Project::map->clear(0);
  Gui::view->drawMain(true);

  switch (tool)
  {
    case Tool::PAINT:
      Project::setTool(Tool::PAINT);
      Project::tool->reset();
      Gui::paint->updateBrush();
      Gui::paint->show();
      Gui::statusInfo((char *)"Middle-click to navigate. Mouse wheel zooms. Esc to cancel rendering.");
      
      break;
    case Tool::PICKER:
      Project::setTool(Tool::PICKER);
      Project::tool->reset();
      Gui::picker->show();
      Gui::statusInfo((char *)"Click to select a color from the image.");
      break;
    case Tool::SELECT:
      Project::setTool(Tool::SELECT);
      Project::tool->redraw(Gui::view);
      Gui::selection->show();
      Gui::statusInfo((char *)"Draw a box, then click inside box to move, outside to change size.");
      break;
    case Tool::OFFSET:
      Project::setTool(Tool::OFFSET);
      Project::tool->reset();
      Gui::offset->show();
      Gui::statusInfo((char *)"Click and drag to change image offset.");
      break;
    case Tool::TEXT:
      Project::setTool(Tool::TEXT);
      Project::tool->reset();
      Gui::text->show();
      Gui::statusInfo((char *)"Click to stamp text onto the image.");
      break;
    case Tool::FILL:
      Project::setTool(Tool::FILL);
      Project::tool->reset();
      Gui::fill->show();
      Gui::statusInfo((char *)"Click to fill an area with the selected color. Blending modes ignored. Esc to cancel.");
      break;
  }
}

void ToolOptions::cloneEnable(Widget *, void *var)
{
  Clone::active = *(int *)var;
}

void ToolOptions::originEnable(Widget *, void *var)
{
  Project::stroke->origin = *(int *)var;
}

void ToolOptions::constrainEnable(Widget *, void *var)
{
  Project::stroke->constrain = *(int *)var;
}

int ToolOptions::getTool()
{
  return tool->var;
}

