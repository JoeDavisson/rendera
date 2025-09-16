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
#include "FillOptions.H"
#include "Gui.H"
#include "InputInt.H"
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
#include "ToggleButton.H"
#include "Tool.H"
#include "TextOptions.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>

namespace
{
  Fl_Hold_Browser *text_browse;
  InputInt *text_size;
  InputInt *text_angle;
  Fl_Input *text_input;
  CheckBox *text_smooth;

  void cb_textChangedSize(Fl_Widget *w, void *data) { TextOptions *temp = (TextOptions *)data; temp->textChangedSize((InputInt *)w, data); }
}

TextOptions::TextOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  // add font names
  text_browse = new Fl_Hold_Browser(8, pos, 160, 384);
  text_browse->labelsize(16);
  text_browse->textsize(16);
  text_browse->resize(this->x() + 8, this->y() + pos, 160, 384);

  for (int i = 0; i < Fl::set_fonts("*"); i++)
  {
    const char *name = Fl::get_font_name((Fl_Font)i, 0);
    text_browse->add(name);
  }

  text_browse->value(1);
  text_browse->callback((Fl_Callback *)cb_textChangedSize);
  pos += 384 + Gui::SPACING;

  // font size
  text_size = new InputInt(this, 64, pos, 96, 32, "Size:",
                           (Fl_Callback *)cb_textChangedSize, 4, 500);
  text_size->value("48");
  pos += 32 + Gui::SPACING;

  text_angle = new InputInt(this, 64, pos, 96, 32, "Angle:",
                           (Fl_Callback *)cb_textChangedSize, -359, 359);
  text_angle->value("0");
  pos += 32 + Gui::SPACING;
  
  text_input = new Fl_Input(8, pos, 160, 32, "");
  text_input->textsize(16);
  text_input->value("Text");
  text_input->resize(this->x() + 8, this->y() + pos, 160, 32);
  text_input->callback((Fl_Callback *)cb_textChangedSize);
  pos += 32 + Gui::SPACING;

  text_smooth = new CheckBox(this, 8, pos, 16, 16, "Antialiased", 0);
  text_smooth->center();
  text_smooth->value(1);

  resizable(0);
  end();
}

TextOptions::~TextOptions()
{
}

void TextOptions::textChangedSize(InputInt *input, void *)
{
  input->redraw();
  Project::tool->move(Gui::view);
}

const char *TextOptions::textGetInput()
{
  return text_input->value();
}

int TextOptions::textGetSize()
{
  return atoi(text_size->value());
}

int TextOptions::textGetAngle()
{
  return atoi(text_angle->value());
}

int TextOptions::textGetSmooth()
{
  return text_smooth->value();
}

int TextOptions::textGetFont()
{
  return text_browse->value();
}

