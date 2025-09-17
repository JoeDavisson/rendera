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

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>

namespace
{
  Fl_Hold_Browser *text_browse;
  InputInt *text_size;
  InputInt *text_angle;
  Fl_Input *text_input;
  CheckBox *text_smooth;
  DialogWindow *preview_win;
  Fl_Box *preview_text;
  Fl_Button *preview_done;

  void cb_changedSize(Fl_Widget *w, void *data) { TextOptions *temp = (TextOptions *)data; temp->changedSize((InputInt *)w, data); }

  void cb_closeTextPreview(Fl_Widget *w, void *data) { TextOptions *temp = (TextOptions *)data; temp->closeTextPreview(); }
}

TextOptions::TextOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  // add font names
  text_browse = new Fl_Hold_Browser(8, pos, 160, 320);
  text_browse->labelsize(16);
  text_browse->textsize(16);
  text_browse->resize(this->x() + 8, this->y() + pos, 160, 320);

  for (int i = 0; i < Fl::set_fonts("*"); i++)
  {
    const char *name = Fl::get_font_name((Fl_Font)i, 0);
    text_browse->add(name);
  }

  text_browse->value(1);
  text_browse->callback((Fl_Callback *)cb_changedSize);
  pos += 320 + Gui::SPACING;

  // font size
  text_size = new InputInt(this, 64, pos, 96, 32, "Size:",
                           (Fl_Callback *)cb_changedSize, 4, 500);
  text_size->value("48");
  pos += 32 + Gui::SPACING;

  text_angle = new InputInt(this, 64, pos, 96, 32, "Angle:",
                           (Fl_Callback *)cb_changedSize, -359, 359);
  text_angle->value("0");
  pos += 32 + Gui::SPACING;
  
  text_input = new Fl_Input(8, pos, 160, 32, "");
  text_input->textsize(16);
  text_input->value("Text");
  text_input->resize(this->x() + 8, this->y() + pos, 160, 32);
  text_input->callback((Fl_Callback *)cb_changedSize);
  pos += 32 + Gui::SPACING;

  text_smooth = new CheckBox(this, 8, pos, 16, 16, "Antialiased", 0);
  text_smooth->center();
  text_smooth->value(1);

  initTextPreview();
  resizable(0);
  end();
}

TextOptions::~TextOptions()
{
}

void TextOptions::initTextPreview()
{
  int pos = 8;

  preview_win = new DialogWindow(528, 460, "Font Preview");
  preview_text = new Fl_Box(8, 8, 512, 384, "The quick brown\nfox jumps over\nthe lazy dog. ");
  preview_text->box(FL_DOWN_BOX);
  preview_text->align(FL_ALIGN_CENTER);
  preview_text->labelsize(48);

  pos += 384 + 8;

  new Separator(preview_win, 0, pos, 528, Separator::HORIZONTAL, "");
  pos += 12;

  preview_done = new Fl_Button(424, pos, 96, 40, "Done (F)");
  preview_done->shortcut('f');
  preview_done->callback((Fl_Callback *)cb_closeTextPreview);
  preview_win->set_non_modal();
  preview_win->end();
}

void TextOptions::toggleTextPreview()
{
  if (preview_win->shown() == 0)
    preview_win->show();
  else
    preview_win->hide();
}

void TextOptions::closeTextPreview()
{
  preview_win->hide();
}

void TextOptions::changedSize(InputInt *input, void *)
{
  int font = getFont();
  preview_win->show();
  preview_text->labelfont(font - 1);
  preview_text->redraw();
  input->redraw();
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

int TextOptions::getFont()
{
  return text_browse->value();
}

