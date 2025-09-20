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
#include "Bitmap.H"
#include "CheckBox.H"
#include "DialogWindow.H"
#include "FontPreview.H"
#include "Gui.H"
#include "InputInt.H"
#include "Map.H"
#include "OffsetOptions.H"
#include "Project.H"
#include "Selection.H"
#include "Separator.H"
#include "StaticText.H"
#include "Stroke.H"
#include "ToggleButton.H"
#include "Tool.H"
#include "TextOptions.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>

namespace
{
  void cb_changedSize(Fl_Widget *w, void *data) { TextOptions *temp = (TextOptions *)data; temp->changedSize(); }
}

TextOptions::TextOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  text_size = new InputInt(this, 64, pos, 96, 32, "Size:", 0, 4, 500);
  text_size->callback(cb_changedSize, (void *)this);
  text_size->value("48");
  pos += 32 + Gui::SPACING;

  text_angle = new InputInt(this, 64, pos, 96, 32, "Angle:", 0, -359, 359);
  text_angle->callback(cb_changedSize, (void *)this);
  text_angle->value("0");
  pos += 32 + Gui::SPACING;
  
  text_thickness = new InputInt(this, 64, pos, 96, 32, "Thickness:", 0, 0, 31);
//  text_thickness->callback(cb_changedSize, (void *)this);
  text_thickness->value("0");
  pos += 32 + Gui::SPACING;
  
  text_input = new Fl_Input(8, pos, 160, 32, "");
  text_input->textsize(16);
  text_input->value("Text");
  text_input->resize(this->x() + 8, this->y() + pos, 160, 32);
  text_input->callback(cb_changedSize, (void *)this);
  pos += 32 + Gui::SPACING;

  text_smooth = new CheckBox(this, 8, pos, 16, 16, "Antialiased", 0);
  text_smooth->center();
  text_smooth->value(1);
  pos += 16 + Gui::SPACING;

  text_toggle_preview = new Fl_Button(this->x() + 8, this->y() + pos,
                                    160, 32, "Select Font (F)");
  text_toggle_preview->callback((Fl_Callback *)FontPreview::toggle);

  resizable(0);
  end();
}

TextOptions::~TextOptions()
{
}

void TextOptions::changedSize()
{
  int font = FontPreview::getFont();

  FontPreview::update(font - 1);
  text_input->redraw();
  Project::tool->move(Gui::view);
}

const char *TextOptions::getInput()
{
  return text_input->value();
}

int TextOptions::getSize()
{
  return atoi(text_size->value());
}

int TextOptions::getAngle()
{
  return atoi(text_angle->value());
}

int TextOptions::getSmooth()
{
  return text_smooth->value();
}

int TextOptions::getThickness()
{
  return atoi(text_thickness->value());
}

