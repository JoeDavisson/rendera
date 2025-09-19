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
}

ColorOptions::ColorOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  // satval overlaps the hue color wheel
  hue = new Widget(this, 8, pos, 192, 192, 0, 1, 1, 0);
  hue->callback(cb_colorChange, (void *)this);

  satval = new Widget(this, 8 + 48, pos + 48, 96, 96, 0, 1, 1, 0);
  satval->callback(cb_colorChange, (void *)this);

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
  pos += 192 + Gui::SPACING;

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
  Editor::update();
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
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);

  int h, s, v;

  Blend::rgbToHsv(r, g, b, &h, &s, &v);

  float angle = ((M_PI * 2) / 1536) * h;
  int mx = 96 + 88 * std::cos(angle);
  int my = 96 + 88 * std::sin(angle);
  hue->var = mx + 192 * my;
  satval->var = (int)(s / 2.68) + 96 * (int)(v / 2.68);

  hue->do_callback();

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
  int pos = hue->var;
  int mx = pos % 192;
  int my = pos / 192;

  int dist = 68;
  int inner = dist + 6;
  int outer = dist + 25;
  int center = inner + (outer - inner) / 2;

//  if (widget == hue)
  if (hue->visible_focus())
  {
    const int md = ((mx - 96) * (mx - 96) + (my - 96) * (my - 96));

    if (md < ((inner - 32) * (inner - 32)))
    {
      hue->redraw();
      satval->redraw();
      return;
    }
  }

  float mouse_angle = atan2f(my - 96, mx - 96);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 96) * 2.69;
  int v = (satval->var / 96) * 2.69;

  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  //Blend::wheelToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);
  Project::brush->blend = blend->value();

  // hue circle
  hue->bitmap->clear(Blend::trans(convertFormat(getFltkColor(FL_BACKGROUND_COLOR), true), makeRgb(0, 0, 0), 192));

  for (int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 96 + outer * std::cos(angle);
    int y1 = 96 + outer * std::sin(angle);
    int x2 = 96 + inner * std::cos(angle);
    int y2 = 96 + inner * std::sin(angle);
    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 252);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 252);
  }

  for (int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 96 + (outer - 1) * std::cos(angle);
    int y1 = 96 + (outer - 1) * std::sin(angle);
    int x2 = 96 + (inner + 1) * std::cos(angle);
    int y2 = 96 + (inner + 1) * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 248);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 248);
  }

 for (int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 96 + (outer - 3) * std::cos(angle);
    int y1 = 96 + (outer - 3) * std::sin(angle);
    int x2 = 96 + (inner + 3) * std::cos(angle);
    int y2 = 96 + (inner + 3) * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(r, g, b), 0);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(r, g, b), 0);
  }

  const int x1 = 96 + center * std::cos(mouse_angle);
  const int y1 = 96 + center * std::sin(mouse_angle);

  Blend::hsvToRgb(h, 255, 255, &r, &g, &b);
  //Blend::wheelToRgb(h, 255, 255, &r, &g, &b);
  hue->bitmap->rect(x1 - 10, y1 - 10, x1 + 10, y1 + 10, makeRgb(0, 0, 0), 192);
  hue->bitmap->rect(x1 - 9, y1 - 9, x1 + 9, y1 + 9, makeRgb(0, 0, 0), 96);
  hue->bitmap->xorRect(x1 - 8, y1 - 8, x1 + 8, y1 + 8);
  hue->bitmap->rectfill(x1 - 7, y1 - 7, x1 + 7, y1 + 7, makeRgb(r, g, b), 0);

  // saturation/value
  hue->bitmap->rect(48 - 1, 48 - 1, 48 + 95 + 1, 48 + 95 + 1, makeRgb(0, 0, 0), 192);

  for (int y = 0; y < 96; y++)
  {
    for (int x = 0; x < 96; x++)
    {
      Blend::hsvToRgb(h, x * 2.69, y * 2.69, &r, &g, &b);
      //Blend::wheelToRgb(h, x * 4.05, y * 4.05, &r, &g, &b);
      satval->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
    }
  }

  int x = (satval->var % 96);
  int y = (satval->var / 96);

  if (x < 9)
    x = 9;
  if (y < 9)
    y = 9;
  if (x > 86)
    x = 86;
  if (y > 86)
    y = 86;

  satval->bitmap->rect(x - 10, y - 10, x + 10, y + 10, makeRgb(0, 0, 0), 192);
  satval->bitmap->rect(x - 9, y - 9, x + 9, y + 9, makeRgb(0, 0, 0), 96);
  satval->bitmap->xorRect(x - 8, y - 8, x + 8, y + 8);

  colorTrans();
  colorHexUpdate();
  hue->redraw();
  satval->redraw();
  Editor::update();
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
}

void ColorOptions::paletteIndex(int var)
{ 
  palette_swatches->var = var;
  Project::palette->draw(palette_swatches);
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
  Editor::update();
}

int ColorOptions::paletteSwatchesIndex()
{
  return palette_swatches->var;
}

void ColorOptions::changePalette(Palette *pal)
{
  palette_swatches->var = 0;
  pal->draw(palette_swatches);
}

