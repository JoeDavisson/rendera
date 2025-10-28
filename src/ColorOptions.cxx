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

#include <cmath>

#include "Blend.H"
#include "Brush.H"
#include "Button.H"
#include "Bitmap.H"
#include "ColorOptions.H"
#include "Editor.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
#include "InputInt.H"
#include "InputText.H"
#include "Palette.H"
#include "Project.H"
#include "RepeatButton.H"
#include "Separator.H"
#include "StaticText.H"
#include "Tool.H"
#include "View.H"
#include "Wheel.H"
#include "Widget.H"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

namespace
{
  void cb_colorChange(Fl_Widget *w, void *data) { ColorOptions *temp = (ColorOptions *)data; temp->colorChange(); }

  void cb_colorHexInput(Fl_Widget *w, void *data) { ColorOptions *temp = (ColorOptions *)data; temp->colorHexInput(); }

  void cb_colorTrans(Fl_Widget *w, void *data) { ColorOptions *temp = (ColorOptions *)data; temp->colorTrans(); }

  void cb_colorTransInput(Fl_Widget *w, void *data) { ColorOptions *temp = (ColorOptions *)data; temp->colorTransInput(); }

  void cb_paletteSwatches(Fl_Widget *w, void *data) { ColorOptions *temp = (ColorOptions *)data; temp->paletteSwatches(); }

  void cb_paletteInput(Fl_Widget *w, void *data) { ColorOptions *temp = (ColorOptions *)data; temp->paletteInput(); }
}

ColorOptions::ColorOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  wheel = new Wheel(this->x() + 8, this->y() + pos, 192, 192, "");
  wheel->callback(cb_colorChange, (void *)this);

  pos += 192 + 8;

  hexcolor = new InputText(this, 56, pos, 136, 32, "Hex:", 0);
  hexcolor->callback(cb_colorHexInput, (void *)this);
  hexcolor->maximum_size(6);
  hexcolor->textfont(FL_COURIER);
  hexcolor->textsize(18);

  pos += 32 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::COLORS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  trans_input = new InputInt(this, 8, pos, 192, 32, "", 0, 0, 255);
  trans_input->callback(cb_colorTransInput, (void *)this);
  trans_input->value("0");
  pos += 32 + 8;

  trans = new Widget(this, 8, pos, 192, 42, "Transparency", 6, 42, 0);
  trans->callback(cb_colorTrans, (void *)this);

  pos += 42 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::COLORS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  blend = new Fl_Choice(8, pos, 192, 32, "");
  blend->tooltip("Blending Mode");
  blend->textsize(10);
  blend->resize(this->x() + 8, this->y() + pos, 192, 32);
  blend->add("Normal");
  blend->add("Gamma Correct");
  blend->add("Lighten");
  blend->add("Darken");
  blend->add("Colorize");
  blend->add("Luminosity");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->value(0);
  blend->callback(cb_colorChange, (void *)this);
  blend->textsize(16);
  pos += 32 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::COLORS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  palette_swatches = new Widget(this, 8, pos, 192, 192, 0, 12, 12, 0);
  palette_swatches->callback(cb_paletteSwatches, (void *)this);
  pos += 192 + 8;

  palette_input = new InputInt(this, 8, pos, 96, 32, "Index:", 0, 0, 255);
  palette_input->callback(cb_paletteInput, (void *)this);
  palette_input->value("0");
  palette_input->center();
  pos += 32 + Gui::SPACING;

  resizable(0);
  end();

  paletteDraw();
}

ColorOptions::~ColorOptions()
{
}

void ColorOptions::colorHexInput()
{
  unsigned int c;

  sscanf(hexcolor->value(), "%06x", &c);

  if (c > 0xffffff)
    c = 0xffffff;

  c |= 0xff000000;

  colorUpdate(convertFormat((int)c, true));
  colorHexUpdate();
//  Editor::update();
}

void ColorOptions::colorHexUpdate()
{
  char hex_string[8];
  snprintf(hex_string, sizeof(hex_string),
       "%06x", (unsigned)convertFormat(Project::brush->color, true) & 0xffffff);
  hexcolor->value(hex_string);
}

void ColorOptions::colorUpdate(int c)
{
  wheel->update(c);
  Project::brush->color = c;
  colorHexUpdate();
  colorTrans();
}

void ColorOptions::transUpdate(int t)
{
  trans->var = t / 8;
  trans->redraw();
  Project::brush->trans = t;
  char s[16];
  snprintf(s, sizeof(s), "%d", Project::brush->trans);
  trans_input->value(s);
  trans_input->redraw();
  colorTrans();
}

void ColorOptions::colorChange()
{
  Project::brush->color = wheel->getColor();
  Project::brush->blend = blend->value();

  colorTrans();
  colorHexUpdate();
//  Editor::update();
//  wheel->update(Project::brush->color);
}

void ColorOptions::colorTransInput()
{
  Project::brush->trans = atoi(trans_input->value());
  trans->var = Project::brush->trans / 8.22;
}

void ColorOptions::colorTrans()
{
  int temp_trans = trans->var * 8;

  if (temp_trans >= 248)
    temp_trans = 255;

  Project::brush->trans = temp_trans;

  char s[16];
  snprintf(s, sizeof(s), "%d", Project::brush->trans);
  trans_input->value(s);
  trans_input->redraw();

  for (int y = 0; y < trans->bitmap->h; y++)
  {
    for (int x = 0; x < trans->bitmap->w; x++)
    {
      const int checker = ((x / 16) & 1) ^ (((y + 3) / 16) & 1) ? 0xff989898 : 0xff686868;
      trans->bitmap->setpixel(x, y, checker);
    }
  }

  const int c = Project::brush->color;
  const float mul = 255.0 / (trans->bitmap->w - 1);

  for (int x = 0; x < trans->bitmap->w; x++)
  {
    trans->bitmap->vline(0, x, trans->bitmap->h - 1, c, x * mul);
  }

  const int stepx = trans->stepx;
  const int pos = trans->var * stepx;

  trans->bitmap->xorRect(pos + 1, 1, pos + stepx - 2, trans->h() - 2);
  trans->bitmap->rect(pos, 0, pos + stepx - 1, trans->h() - 1, makeRgb(0, 0, 0), 0);
  trans->redraw();
  Project::tool->redraw(Gui::view);
}

void ColorOptions::colorBlend()
{
  colorChange();
}

void ColorOptions::paletteDraw()
{ 
  Project::palette->draw(palette_swatches);
  palette_swatches->var = 0;
  palette_swatches->redraw();
  palette_input->value(palette_swatches->var);
}

void ColorOptions::paletteIndex(int var)
{ 
  palette_swatches->var = var;
  palette_input->value(var);
  Project::palette->draw(palette_swatches);
} 

void ColorOptions::paletteInput()
{ 
  Palette *pal = Project::palette;
  int index = atoi(palette_input->value());
  int max = pal->max;

  if (index >= max)
  {
    index = max - 1;
    palette_input->value(index);
  }

  palette_swatches->var = index;
  pal->draw(palette_swatches);
  palette_swatches->do_callback();
} 

int ColorOptions::paletteGetIndex()
{ 
  return palette_swatches->var;
} 

void ColorOptions::paletteSwatches()
{
  Palette *pal = Project::palette;
  int pos = palette_swatches->var;

  if (pos > pal->max - 1)
  {
    pos = pal->max - 1;
    palette_swatches->var = pos;
  }
                         
  int step = palette_swatches->stepx;
  int div = palette_swatches->w() / step;
                        
  int x = pos % div;
  int y = pos / div;

  if (y > (pal->max - 1) / div)
  {
    y = (pal->max - 1) / div;
    pos = x + div * y;
    x = pos % div;
    y = pos / div;
    palette_swatches->var = pos;
  }

  if (pos > pal->max - 1)
  {
    pos = pal->max - 1;
    x = pos % div;
    y = pos / div;
    palette_swatches->var = pos;
  }

  int c = palette_swatches->bitmap->getpixel(x * step + 2, y * step + 2);

  pal->draw(palette_swatches);
  colorUpdate(c);
  palette_input->value(palette_swatches->var);
//  Editor::update();
}

int ColorOptions::paletteSwatchesIndex()
{
  return palette_swatches->var;
}

void ColorOptions::changePalette(Palette *pal)
{
  palette_swatches->var = 0;
  palette_input->value(0);
  pal->draw(palette_swatches);
}

