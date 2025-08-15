/*
Copyright (c) 2024 Joe Davisson.

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

#ifndef PACKAGE_STRING
#  include "config.h"
#endif

#include <algorithm>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Widget.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Button.H"
#include "CheckBox.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "Editor.H"
#include "Group.H"
#include "Gui.H"
#include "Inline.H"
#include "InputInt.H"
#include "InputText.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "Quantize.H"
#include "Separator.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

#define STATUS_HEIGHT 32

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Widget *hue;
    Widget *sat_val;
    Fl_Repeat_Button *insert;
    Fl_Repeat_Button *remove;
    Fl_Button *replace;
    Fl_Button *undo;
    Fl_Button *redo;
    Fl_Button *rgb_ramp;
    Fl_Button *hsv_ramp;
    InputText *hexcolor;
    InputText *hexcolor_web;
    Widget *palette;
    Widget *color;
    Fl_Button *done;
    Group *info;
    Fl_Box *info_text;
    Group *index;
    Fl_Box *index_text;
    Fl_Box *color_text;
  }

  int last_index = 0;
  int replace_state = 0;
  int ramp_begin = 0;
  int ramp_state = 0;
  int oldsvx, oldsvy, oldhy;

  int levels = 256;
  int undo_current = levels - 1;
  int redo_current = levels - 1;

  Palette **undo_stack;
  Palette **redo_stack;
}

void Editor::updateInfo(char *s)
{
  Items::info_text->copy_label(s);
  Items::info_text->redraw();
}

void Editor::updateIndex(int index)
{
  char s[256];

  snprintf(s, sizeof(s), "  Index = %d", index);
  Items::index_text->copy_label(s);
  Items::index_text->redraw();
}

void Editor::setHsvSliders()
{
  int h, s, v;
  int c = Project::brush->color;

  Blend::rgbToHsv(getr(c), getg(c), getb(c), &h, &s, &v);
  Items::hue->var = h / 6 * Items::hue->w();
  Items::sat_val->var = s + 256 * v;
  Items::hue->redraw();
  Items::sat_val->redraw();
}

void Editor::setHsv()
{
  int r = 0, g = 0, b = 0;
  int c = Project::brush->color;
  int w = Items::hue->w();
  int h = Items::hue->var / w * 6;

  Items::hue->bitmap->clear(makeRgb(0, 0, 0));

  for (int y = 0; y < 256; y++)
  {
    int *p = Items::sat_val->bitmap->row[y];

    for (int x = 0; x < 256; x++)
    {
      Blend::hsvToRgb(h, x, y, &r, &g, &b);
      *p++ = makeRgb(r, g, b);
    }

    Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
    Items::hue->bitmap->hline(0, y, Items::hue->w() - 1, makeRgb(r, g, b), 0);
  }

  int svx = Items::sat_val->var & 255;
  int svy = Items::sat_val->var / 256;
  int hy = Items::hue->var / w;

  Items::sat_val->bitmap->rect(0, svy - 1, 255, svy + 1, makeRgb(0, 0, 0), 0);
  Items::sat_val->bitmap->rect(svx - 1, 0, svx + 1, 255, makeRgb(0, 0, 0), 0);
  Items::sat_val->bitmap->xorHline(0, svy, 255);
  Items::sat_val->bitmap->xorVline(0, svx, 255);
  Items::sat_val->bitmap->rect(svx - 9, svy - 9, svx + 9, svy + 9, makeRgb(0, 0, 0), 0);
  Items::sat_val->bitmap->xorRect(svx - 8, svy - 8, svx + 8, svy + 8);
  Items::sat_val->bitmap->rectfill(svx - 7, svy - 7, svx + 7, svy + 7, c, 0);
  Items::sat_val->bitmap->rect(0, 0, 255, 255, makeRgb(0, 0, 0), 0);

  c = Items::hue->bitmap->getpixel(16, hy);

  Items::hue->bitmap->rect(0, hy - 5, w - 1, hy + 5, makeRgb(0, 0, 0), 0);
  Items::hue->bitmap->xorRect(1, hy - 4, w - 2, hy + 4);
  Items::hue->bitmap->rectfill(2, hy - 3 , w - 3, hy + 3, c, 0);
  Items::hue->bitmap->rect(0, 0, w - 1, 255, makeRgb(0, 0, 0), 0);

  oldsvx = svx;
  oldsvy = svy;
  oldhy = hy;

  Items::hue->redraw();
  Items::sat_val->redraw();

  c = Project::brush->color;
  Items::color->bitmap->clear(c);

  rgba_type rgba = getRgba(c);

  r = rgba.r >> 4;
  g = rgba.g >> 4;
  b = rgba.b >> 4;

  r = (r << 4) + r;
  g = (g << 4) + g;
  b = (b << 4) + b;

  c = makeRgba(r, g, b, 255);

  Items::color->bitmap->rect(0, 0,
    Items::color->bitmap->w - 1, Items::color->bitmap->h - 1,
    makeRgb(0, 0, 0), 0);

  Items::color->redraw();
}

void Editor::updateHexColor()
{
  char hex_string[8];

  snprintf(hex_string, sizeof(hex_string),
     "%06x", (unsigned)convertFormat(Project::brush->color, true) & 0xffffff);
  Items::hexcolor->value(hex_string);

  // shortcut hex
  int c = (unsigned)convertFormat(Project::brush->color, true) & 0xffffff;
  rgba_type rgba = getRgba(c);

  snprintf(hex_string, sizeof(hex_string),
     "%01x%01x%01x", rgba.b >> 4, rgba.g >> 4, rgba.r >> 4);
  Items::hexcolor_web->value(hex_string);
}

void Editor::checkHexColor()
{ 
  unsigned int c;
  
  sscanf(Items::hexcolor->value(), "%06x", &c);
  
  if (c > 0xffffff)
    c = 0xffffff;
  
  c |= 0xff000000;
  
  Gui::colorUpdate(convertFormat((int)c, true));
  setHsvSliders();
  setHsv();
  updateHexColor();
}

void Editor::checkHexColorWeb()
{ 
  unsigned int c;
  
  sscanf(Items::hexcolor_web->value(), "%03x", &c);
  
  if (c > 0xfff)
    c = 0xfff;

  int r = (c & 0xfff) >> 8;
  int g = (c & 0xff) >> 4;
  int b = c & 0xf;

  r = (r << 4) + r;
  g = (g << 4) + g;
  b = (b << 4) + b;

  c = makeRgba(r, g, b, 255);
  Project::brush->color = c;
  
  setHsvSliders();
  setHsv();
  updateHexColor();
  Gui::colorUpdate(c);
}

void Editor::insertColor()
{
  if (Project::palette->max >= 256)
    return;

  push();
  Project::palette->insertColor(Project::brush->color, Items::palette->var);
  Project::palette->draw(Items::palette);
  Gui::paletteDraw();
  Items::palette->do_callback();
}

void Editor::removeColor()
{
  if (Project::palette->max <= 1)
    return;

  push();
  Project::palette->deleteColor(Items::palette->var);
  Project::palette->draw(Items::palette);
  Gui::paletteDraw();

  if (Items::palette->var > Project::palette->max - 1)
    Items::palette->var = Project::palette->max - 1;

  Items::palette->do_callback();
}

void Editor::checkReplaceColor(int pos)
{
  push();
  Project::palette->replaceColor(Project::brush->color, pos);
  Project::palette->draw(Items::palette);
  Gui::colorUpdate(Project::brush->color);
  Gui::paletteDraw();
  Items::replace->value(0);
  Items::replace->redraw();
  Items::dialog->cursor(FL_CURSOR_DEFAULT);
  replace_state = 0;
}

void Editor::replaceColor()
{
  if (ramp_state == 0)
  {
    Items::replace->value(1);
    Items::replace->redraw();
    replace_state = 1;
    Items::dialog->cursor(FL_CURSOR_HAND);
  }
}

void Editor::copyColor(int c1, int c2)
{
  push();
  Project::palette->data[c2] = Project::palette->data[c1];
}

void Editor::swapColor(int c1, int c2)
{
  push();
  Project::palette->swapColor(c1, c2);
}

void Editor::checkRampRgb(int end)
{
  push();

  Palette *pal = Project::palette;
  int begin = ramp_begin;

  if (begin > end)
    std::swap(begin, end);

  int num = end - begin;
  int c1 = pal->data[begin];
  int c2 = pal->data[end];
  double stepr = (double)(getr(c2) - getr(c1)) / num;
  double stepg = (double)(getg(c2) - getg(c1)) / num;
  double stepb = (double)(getb(c2) - getb(c1)) / num;
  double r = getr(c1);
  double g = getg(c1);
  double b = getb(c1);

  for (int i = begin; i < end; i++)
  {
    pal->data[i] = makeRgb(r, g, b);
    r += stepr;
    g += stepg;
    b += stepb;
  }

  Items::rgb_ramp->value(0);
  Items::rgb_ramp->redraw();
  Project::palette->draw(Items::palette);
  Gui::paletteDraw();
  ramp_state = 0;
  Items::dialog->cursor(FL_CURSOR_DEFAULT);
}

void Editor::checkRampHsv(int end)
{
  push();

  Palette *pal = Project::palette;
  int begin = ramp_begin;

  if (begin > end)
    std::swap(begin, end);

  int num = end - begin;
  int c1 = pal->data[begin];
  int c2 = pal->data[end];
  int h1, s1, v1;
  int h2, s2, v2;

  Blend::rgbToHsv(getr(c1), getg(c1), getb(c1), &h1, &s1, &v1);
  Blend::rgbToHsv(getr(c2), getg(c2), getb(c2), &h2, &s2, &v2);

  int r, g, b;
  double h = h1;
  double s = s1;
  double v = v1;
  const double steph = (double)(h2 - h1) / num;
  const double steps = (double)(s2 - s1) / num;
  const double stepv = (double)(v2 - v1) / num;

  for (int i = begin; i < end; i++)
  {
    Blend::hsvToRgb(h, s, v, &r, &g, &b);
    pal->data[i] = makeRgb(r, g, b);
    h += steph;
    s += steps;
    v += stepv;
  }

  Items::hsv_ramp->value(0);
  Items::hsv_ramp->redraw();
  Project::palette->draw(Items::palette);
  Gui::paletteDraw();
  ramp_state = 0;
  Items::dialog->cursor(FL_CURSOR_DEFAULT);
}

void Editor::checkPalette()
{
  Palette *pal = Project::palette;
  int pos = Items::palette->var;

  if (pos > pal->max - 1)
  {
    pos = pal->max - 1;
    Items::palette->var = pos;
  }

  if (Fl::event_button1())
  {
    if (ramp_state)
    {
      ramp_begin = last_index;

      switch (ramp_state)
      {
        case 1:
          checkRampRgb(pos);
          break;
        case 2:
          checkRampHsv(pos);
          break;
      }
    }
    else if (replace_state)
    {
      checkReplaceColor(pos);
    }
    else if (Fl::event_ctrl())
    {
      copyColor(last_index, pos);
      Project::brush->color = pal->data[pos];
      updateHexColor();
      setHsvSliders();
      Gui::colorUpdate(Project::brush->color);
      setHsv();
      Project::palette->draw(Items::palette);
    }
    else if (Fl::event_shift())
    {
      swapColor(last_index, pos);
      Project::brush->color = pal->data[pos];
      updateHexColor();
      setHsvSliders();
      Gui::colorUpdate(Project::brush->color);
      setHsv();
      Project::palette->draw(Items::palette);
    }
      else
    {
      Project::brush->color = pal->data[pos];
      updateHexColor();
      setHsvSliders();
      Gui::colorUpdate(Project::brush->color);
      setHsv();
    }
  }

  Gui::paletteIndex(pos);
  updateIndex(pos);
  last_index = pos;

  pal->draw(Items::palette);
}

void Editor::getHue()
{
  int h = (Items::hue->var / Items::hue->w()) * 6;
  int s = Items::sat_val->var & 255;
  int v = Items::sat_val->var / 256;
  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);

  Gui::colorUpdate(Project::brush->color);
  updateHexColor();
  setHsv();
}

void Editor::getSatVal()
{
  int h = (Items::hue->var / Items::hue->w()) * 6;
  int s = Items::sat_val->var & 255;
  int v = Items::sat_val->var / 256;
  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);

  Gui::colorUpdate(Project::brush->color);
  updateHexColor();
  setHsv();
}

void Editor::rgbRamp()
{
  if (ramp_state == 0)
  {
    Items::rgb_ramp->value(1);
    Items::rgb_ramp->redraw();
    ramp_state = 1;
    Items::dialog->cursor(FL_CURSOR_HAND);
  }
}

void Editor::hsvRamp()
{
  if (ramp_state == 0)
  {
    Items::hsv_ramp->value(1);
    Items::hsv_ramp->redraw();
    ramp_state = 2;
    Items::dialog->cursor(FL_CURSOR_HAND);
  }
}

void Editor::begin()
{
  Items::palette->var = Gui::palette_swatches->var;
  last_index = Items::palette->var;
  Project::palette->draw(Items::palette);
  updateHexColor();
  setHsvSliders();
  setHsv();
  Items::dialog->show();
  ramp_begin = 0;
  ramp_state = 0;
  updateInfo((char *)"  Shift to swap, Ctrl to copy, Right-click to move cursor.");
  updateIndex(Items::palette->var);

  while (Items::dialog->shown())
  {
//    if (Fl::event_button1() == 0)
//      button_down = false;

    Fl::wait();
  }
}

void Editor::close()
{
  replace_state = 0;
  ramp_begin = 0;
  ramp_state = 0;
  Items::replace->value(0);
  Items::rgb_ramp->value(0);
  Items::hsv_ramp->value(0);
  Project::palette->fillTable();
  Items::dialog->hide();
}

void Editor::resetUndo()
{
  for (int i = 0; i < levels; i++)
  {
    delete undo_stack[i];
    delete redo_stack[i];

    undo_stack[i] = new Palette();
    redo_stack[i] = new Palette();
  }

  undo_current = levels - 1;
  redo_current = levels - 1;
}

void Editor::doPush()
{
  if (undo_current < 0)
  {
    undo_current = 0;

    Palette *temp_pal = undo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      undo_stack[i] = undo_stack[i - 1];

    undo_stack[0] = temp_pal;
  }

  delete undo_stack[undo_current];
  undo_stack[undo_current] = new Palette();
  Project::palette->copy(undo_stack[undo_current]);
  undo_current--;
}

void Editor::push()
{
  doPush();

  for (int i = 0; i < levels; i++)
  {
    delete redo_stack[i];
    redo_stack[i] = new Palette();
  }

  redo_current = levels - 1;
}

void Editor::pop()
{
  if (undo_current >= levels - 1)
    return;

  pushRedo();

  if (undo_current >= 0)
  {
    delete undo_stack[undo_current];
    undo_stack[undo_current] = new Palette();
  }

  undo_current++;
  undo_stack[undo_current]->copy(Project::palette);

  Gui::paletteDraw();
  Items::palette->do_callback();
}

void Editor::pushRedo()
{
  if (redo_current < 0)
  {
    redo_current = 0;

    Palette *temp_pal = redo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      redo_stack[i] = redo_stack[i - 1];

    redo_stack[0] = temp_pal;
  }

  Project::palette->copy(redo_stack[redo_current]);

  redo_current--;
}

void Editor::popRedo()
{
  if (redo_current >= levels - 1)
    return;

  doPush();

  if (redo_current >= 0)
  {
    delete redo_stack[redo_current];
    redo_stack[redo_current] = new Palette();
  }

  redo_current++;
  redo_stack[redo_current]->copy(Project::palette);

  Gui::paletteDraw();
  Items::palette->do_callback();
}

void Editor::init()
{
  int x1, y1;

  Items::dialog = new DialogWindow(712, 372, "Palette Editor");

  Items::hue = new Widget(Items::dialog, 8, 8, 32, 256,
                          0, 1, 1, (Fl_Callback *)getHue);

  Items::sat_val = new Widget(Items::dialog, 48, 8, 256, 256,
                              0, 1, 1, (Fl_Callback *)getSatVal);
  x1 = 8 + Items::hue->w() + 8 + Items::sat_val->w() + 8;
  y1 = 8;
  Items::insert = new Fl_Repeat_Button(x1, y1, 60, 60, "+");
  Items::insert->callback((Fl_Callback *)insertColor);
  Items::insert->labelsize(32);
  Items::insert->tooltip("Insert");

  x1 += 60 + 8;

  Items::remove = new Fl_Repeat_Button(x1, y1, 60, 60, "-");
  Items::remove->callback((Fl_Callback *)removeColor);
  Items::remove->labelsize(32);
  Items::remove->tooltip("Delete");

  x1 = 8 + Items::hue->w() + 8 + Items::sat_val->w() + 8;
  y1 += 60 + 8;

  Items::replace = new Fl_Button(x1, y1, 128, 42, "Replace");
  Items::replace->callback((Fl_Callback *)replaceColor);
  Items::insert->labelsize(18);
  y1 += 42 + 8;

  Items::undo = new Fl_Button(x1, y1, 60, 42, "Undo");
  Items::undo->callback((Fl_Callback *)pop);
//  y1 += 42 + 8;
    
  Items::redo = new Fl_Button(x1 + 68, y1, 60, 42, "Redo");
  Items::redo->callback((Fl_Callback *)popRedo);
  y1 += 42 + 8;
    
  Items::rgb_ramp = new Fl_Button(x1, y1, 128, 40, "RGB Ramp");
  Items::rgb_ramp->callback((Fl_Callback *)rgbRamp);
  y1 += 40 + 8;

  Items::hsv_ramp = new Fl_Button(x1, y1, 128, 40, "HSV Ramp");
  Items::hsv_ramp->callback((Fl_Callback *)hsvRamp);

  x1 += 128 + 8;
  y1 = 8;

  Items::palette = new Widget(Items::dialog, x1, y1, 256, 256,
                                "", 16, 16, (Fl_Callback *)checkPalette);

  y1 += 256 + 8;

  new Separator(Items::dialog, 0, y1, Items::dialog->w(), Separator::HORIZONTAL, "");

  x1 = 8;
  y1 += 12;

  Items::hexcolor = new InputText(Items::dialog, x1, y1, 128, 32,
                                  "Hexadecimal", (Fl_Callback *)checkHexColor);
  Items::hexcolor->maximum_size(6);
  Items::hexcolor->labelsize(16);
  Items::hexcolor->textsize(18);
  Items::hexcolor->textfont(FL_COURIER);
  Items::hexcolor->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
  Items::hexcolor->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
  x1 += 128 + 8;

  Items::hexcolor_web = new InputText(Items::dialog, x1, y1, 96, 32,
                                      "Shorthand",
                                      (Fl_Callback *)checkHexColorWeb);
  Items::hexcolor_web->maximum_size(3);
  Items::hexcolor_web->labelsize(16);
  Items::hexcolor_web->textsize(18);
  Items::hexcolor_web->textfont(FL_COURIER);
  Items::hexcolor_web->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
  Items::hexcolor_web->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
  x1 += 96 + 8;

  Items::color = new Widget(Items::dialog, x1, y1, 128, 32,
                            "", 0, 0, 0);

  Items::color_text = new Fl_Box(FL_NO_BOX, x1, y1, 128, 32, "Preview");
  Items::color_text->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
  Items::color_text->labelsize(16);

  x1 += 128 + 8;

  Items::done = new Fl_Button(Items::dialog->w() - 96 - 8, y1,
                              96, 40, "Done (E)");
  Items::done->shortcut('e');
  Items::done->callback((Fl_Callback *)close);

  x1 = 0;
  y1 += 40 + 16;
  Items::index = new Group(Items::dialog->w() - 128, y1, 128, STATUS_HEIGHT, "");
  Items::index_text = new Fl_Box(FL_NO_BOX, Items::index->x(), Items::index->y(), Items::index->w(), Items::index->h(), "");
  Items::index_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  Items::info = new Group(x1, y1, Items::dialog->w() - Items::index->w(), STATUS_HEIGHT, "");
  Items::info_text = new Fl_Box(FL_NO_BOX, Items::info->x(), Items::info->y(), Items::info->w(), Items::info->h(), "");
  Items::info_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  Items::dialog->set_modal();
  Items::dialog->end(); 

  undo_stack = new Palette *[levels];
  redo_stack = new Palette *[levels];

  for (int i = 0; i < levels; i++)
  {
    undo_stack[i] = 0;
    redo_stack[i] = 0;
  }

  resetUndo();
}

