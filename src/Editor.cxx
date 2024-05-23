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
#include "Common.H"
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

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Widget *hue;
    Widget *sat_val;
    Widget *trans;
    Fl_Repeat_Button *insert;
    Fl_Repeat_Button *remove;
    Fl_Button *replace;
    Fl_Button *undo;
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
  }

  int last_index = 0;
  int replace_state = 0;
  int ramp_begin = 0;
  int ramp_state = 0;
  bool button_down = false;
  bool begin_undo;
  int oldsvx, oldsvy, oldhy;
  Palette *undo_palette;
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

void Editor::storeUndo()
{
  Project::palette->copy(undo_palette);
  begin_undo = true;
}

void Editor::getUndo()
{
  if (begin_undo)
  {
    begin_undo = false;
    undo_palette->copy(Project::palette);
    Project::palette->draw(Items::palette);
    Gui::paletteDraw();
    Items::palette->do_callback();
  }
}

void Editor::setHsvSliders()
{
  int h, s, v;
  int c = Project::brush->color;

  Blend::rgbToHsv(getr(c), getg(c), getb(c), &h, &s, &v);
  Items::hue->var = h / 6 * 24;
  Items::sat_val->var = s + 256 * v;
  Items::hue->redraw();
  Items::sat_val->redraw();
}

void Editor::setHsv(bool redraw)
{
  int r = 0, g = 0, b = 0;
  int c = Project::brush->color;
  int h = (Items::hue->var / 24) * 6;

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
    Items::hue->bitmap->hline(0, y, 23, makeRgb(r, g, b), 0);
  }

  int svx = Items::sat_val->var & 255;
  int svy = Items::sat_val->var / 256;
  int hy = Items::hue->var / 24;

  Items::sat_val->bitmap->rect(0, svy - 1, 255, svy + 1, makeRgb(0, 0, 0), 128);
  Items::sat_val->bitmap->rect(svx - 1, 0, svx + 1, 255, makeRgb(0, 0, 0), 128);
  Items::sat_val->bitmap->rect(0, svy - 2, 255, svy + 2, makeRgb(0, 0, 0), 192);
  Items::sat_val->bitmap->rect(svx - 2, 0, svx + 2, 255, makeRgb(0, 0, 0), 192);
  Items::sat_val->bitmap->xorHline(0, svy, 255);
  Items::sat_val->bitmap->xorVline(0, svx, 255);
  Items::sat_val->bitmap->rect(svx - 10, svy - 10, svx + 10, svy + 10, makeRgb(0, 0, 0), 192);
  Items::sat_val->bitmap->rect(svx - 9, svy - 9, svx + 9, svy + 9, makeRgb(0, 0, 0), 128);
  Items::sat_val->bitmap->xorRect(svx - 8, svy - 8, svx + 8, svy + 8);
  Items::sat_val->bitmap->rectfill(svx - 7, svy - 7, svx + 7, svy + 7, c, 0);
  Items::sat_val->bitmap->rect(0, 0, 255, 255, makeRgb(0, 0, 0), 0);

  c = Items::hue->bitmap->getpixel(12, hy);

  Items::hue->bitmap->rect(0, hy - 6, 23, hy + 6, makeRgb(0, 0, 0), 192);
  Items::hue->bitmap->rect(0, hy - 5, 23, hy + 5, makeRgb(0, 0, 0), 128);
  Items::hue->bitmap->xorRect(0, hy - 4, 23, hy + 4);
  Items::hue->bitmap->rectfill(0, hy - 3 , 23, hy + 3, c, 0);
  Items::hue->bitmap->rect(0, 0, 23, 255, makeRgb(0, 0, 0), 0);

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
  setHsv(true);
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
  setHsv(true);
  updateHexColor();
  Gui::colorUpdate(c);
}

void Editor::insertColor()
{
  storeUndo();
  Project::palette->insertColor(Project::brush->color, Items::palette->var);
  Project::palette->draw(Items::palette);
  Gui::paletteDraw();
  Items::palette->do_callback();
}

void Editor::removeColor()
{
  storeUndo();
  Project::palette->deleteColor(Items::palette->var);
  Project::palette->draw(Items::palette);
  Gui::paletteDraw();

  if (Items::palette->var > Project::palette->max - 1)
    Items::palette->var = Project::palette->max - 1;

  Items::palette->do_callback();
}

void Editor::checkReplaceColor(int pos)
{
  storeUndo();
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
  storeUndo();
  Project::palette->data[c2] = Project::palette->data[c1];
}

void Editor::swapColor(int c1, int c2)
{
  storeUndo();
  Project::palette->swapColor(c1, c2);
}

void Editor::checkRampRgb(int end)
{
  storeUndo();

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
  storeUndo();

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

  if (Fl::event_button1() && button_down == false)
  {
    button_down = true;

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
      setHsv(true);
      Project::palette->draw(Items::palette);
    }
    else if (Fl::event_shift())
    {
      swapColor(last_index, pos);
      Project::brush->color = pal->data[pos];
      updateHexColor();
      setHsvSliders();
      Gui::colorUpdate(Project::brush->color);
      setHsv(true);
      Project::palette->draw(Items::palette);
    }
      else
    {
      Project::brush->color = pal->data[pos];
      updateHexColor();
      setHsvSliders();
      Gui::colorUpdate(Project::brush->color);
      setHsv(true);
    }
  }

  Gui::paletteIndex(pos);
  updateIndex(pos);
  last_index = pos;

  pal->draw(Items::palette);
}

void Editor::getHue()
{
  int h = (Items::hue->var / 24) * 6;
  int s = Items::sat_val->var & 255;
  int v = Items::sat_val->var / 256;
  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);

  Gui::colorUpdate(Project::brush->color);
  updateHexColor();
  setHsv(true);
}

void Editor::getTrans()
{
  Items::trans->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR)); 

  for (int i = 0; i < 256; i++)
    Items::trans->bitmap->vline(0, i, 23, makeRgb(255, 255, 255), i);

  int tx = Items::trans->var & 255;

  Items::trans->bitmap->rect(tx - 6, 0, tx + 6, 23, makeRgb(0, 0, 0), 192);
  Items::trans->bitmap->rect(tx - 5, 0, tx + 5, 23, makeRgb(0, 0, 0), 128);
  Items::trans->bitmap->xorRect(tx - 4, 0, tx + 4, 23);
  Items::trans->bitmap->rect(0, 0, 255, 23, makeRgb(0, 0, 0), 0);
  Gui::transUpdate(tx);
}

void Editor::getSatVal()
{
  int h = (Items::hue->var / 24) * 6;
  int s = Items::sat_val->var & 255;
  int v = Items::sat_val->var / 256;
  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);

  Gui::colorUpdate(Project::brush->color);
  updateHexColor();
  setHsv(false);
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
  Items::palette->var = Gui::getPaletteIndex();
  last_index = Items::palette->var;
  Project::palette->draw(Items::palette);
  updateHexColor();
  setHsvSliders();
  setHsv(1);
  Items::trans->var = Project::brush->trans;
  getTrans();
  Items::dialog->show();
  begin_undo = false;
  ramp_begin = 0;
  ramp_state = 0;
  updateInfo((char *)"  Shift to swap, Ctrl to copy, Right-click to move cursor.");
  updateIndex(Items::palette->var);

  while (Items::dialog->shown())
  {
    if (Fl::event_button1() == 0)
      button_down = false;

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

void Editor::init()
{
  int x1, y1;

  Items::dialog = new DialogWindow(480, 346 + 32, "Palette Editor");

  Items::hue = new Widget(Items::dialog, 8, 8, 24, 256,
                          0, 1, 1, (Fl_Callback *)getHue);

  Items::sat_val = new Widget(Items::dialog, 40, 8, 256, 256,
                              0, 1, 1, (Fl_Callback *)getSatVal);

  Items::trans = new Widget(Items::dialog, 40, 256 + 16, 256, 24,
                            0, 1, 1, (Fl_Callback *)getTrans);

  y1 = 8;
  Items::insert = new Fl_Repeat_Button(304, y1, 44, 32, "+");
  Items::insert->callback((Fl_Callback *)insertColor);
  Items::insert->labelsize(22);
  Items::insert->tooltip("Insert");

  Items::remove = new Fl_Repeat_Button(358, y1, 44, 32, "-");
  Items::remove->callback((Fl_Callback *)removeColor);
  Items::remove->labelsize(22);
  Items::remove->tooltip("Remove");
  y1 += 32 + 8;

  Items::replace = new Fl_Button(304, y1, 96, 32, "Replace");
  Items::replace->callback((Fl_Callback *)replaceColor);
  Items::insert->labelsize(18);
  y1 += 32 + 8;

  new Separator(Items::dialog, 298, y1, 108, 2, "");
  y1 += 8;

  Items::undo = new Fl_Button(304, y1, 96, 32, "Undo");
  Items::undo->callback((Fl_Callback *)getUndo);
  y1 += 32 + 8;
    
  new Separator(Items::dialog, 298, y1, 108, 2, "");
  y1 += 8;

  Items::rgb_ramp = new Fl_Button(304, y1, 96, 24, "RGB Ramp");
  Items::rgb_ramp->callback((Fl_Callback *)rgbRamp);
  y1 += 24 + 8;

  Items::hsv_ramp = new Fl_Button(304, y1, 96, 24, "HSV Ramp");
  Items::hsv_ramp->callback((Fl_Callback *)hsvRamp);
  y1 += 24 + 8;

  new Separator(Items::dialog, 298, y1, 108, 2, "");
  y1 += 8;

  Items::color = new Widget(Items::dialog, 304, y1, 96, 48,
                            "Paint Color", 0, 0, 0);

  Items::palette = new Widget(Items::dialog, 408, 8, 64, 256,
                                "", 24, 24, (Fl_Callback *)checkPalette);

  new Separator(Items::dialog, 2, 272 + 32, Items::dialog->w() - 4, 2, "");

  x1 = 8;
  y1 = 272 + 40;

  Items::hexcolor = new InputText(Items::dialog, x1, y1, 80, 24,
                                  "Hexadecimal", (Fl_Callback *)checkHexColor);
  Items::hexcolor->maximum_size(6);
  Items::hexcolor->labelsize(12);
  Items::hexcolor->textsize(14);
  Items::hexcolor->textfont(FL_COURIER);
  Items::hexcolor->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
  Items::hexcolor->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
  x1 += 80 + 8;

  Items::hexcolor_web = new InputText(Items::dialog, x1, y1, 64, 24,
                                      "Shorthand",
                                      (Fl_Callback *)checkHexColorWeb);
  Items::hexcolor_web->maximum_size(3);
  Items::hexcolor_web->labelsize(12);
  Items::hexcolor_web->textsize(14);
  Items::hexcolor_web->textfont(FL_COURIER);
  Items::hexcolor_web->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
  Items::hexcolor_web->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM);
  x1 += 64 + 8;

  new Separator(Items::dialog, x1, y1 - 5, 2, 46, "");
  x1 += 8;

  Items::done = new Fl_Button(Items::dialog->w() - 96 - 8, y1,
                              96, 36, "Done (E)");
  Items::done->shortcut('e');
  Items::done->callback((Fl_Callback *)close);

  y1 += 32 + 10;
  Items::info = new Group(0, y1, 352, 24, "");
  Items::info_text = new Fl_Box(FL_NO_BOX, Items::info->x(), Items::info->y(), Items::info->w(), Items::info->h(), "");
  Items::info_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  Items::info_text->labelsize(12);

  Items::index = new Group(352, y1, 128, 24, "");
  Items::index_text = new Fl_Box(FL_NO_BOX, Items::index->x(), Items::index->y(), Items::index->w(), Items::index->h(), "");
  Items::index_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  Items::dialog->set_modal();
  Items::dialog->end(); 

  undo_palette = new Palette();
}


