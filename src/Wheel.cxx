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

#include <Blend.H>
#include <Bitmap.H>
#include <Group.H>
#include <Inline.H>
#include <Wheel.H>
#include <Widget.H>

#include <FL/Fl_Group.H>

namespace
{
  void cb_change_with_cb(Fl_Widget *w, void *data) { Wheel *temp = (Wheel *)data; temp->changeWithCB(); }

  void cb_change_hue(Fl_Widget *w, void *data) { Wheel *temp = (Wheel *)data; temp->changeHue(); }
}

Wheel::Wheel(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  // satval overlaps the hue color wheel
  hue = new Widget(this, 0, 0, 192, 192, 0, 1, 1, 0);
  hue->callback(cb_change_hue, (void *)this);

  satval = new Widget(this, 48, 48, 96, 96, 0, 1, 1, 0);
  satval->callback(cb_change_with_cb, (void *)this);

  resizable(0);
  end();
}

Wheel::~Wheel()
{
}

void Wheel::changeHue()
{
  int pos = hue->var;
  int mx = pos % 192;
  int my = pos / 192; 
  
  int dist = 68;
  int inner = dist + 6;
//  int outer = dist + 25;
//  int center = inner + (outer - inner) / 2;

  const int md = ((mx - 96) * (mx - 96) + (my - 96) * (my - 96));

  if (md < ((inner - 4) * (inner - 4)))
  {
    hue->redraw();
    satval->redraw();
    this->redraw();
    return;
  }

  changeWithCB();
  this->redraw();
}

void Wheel::change()
{ 
  int pos = hue->var;
  int mx = pos % 192;
  int my = pos / 192; 
  
  int dist = 68;
  int inner = dist + 6;
  int outer = dist + 25;
  int center = inner + (outer - inner) / 2;

/*
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
*/

  float mouse_angle = atan2f(my - 96, mx - 96);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 96) * 2.69;
  int v = (satval->var / 96) * 2.69;

  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  color = makeRgb(r, g, b);

  // hue circle
  hue->bitmap->clear(Blend::trans(convertFormat(getFltkColor(FL_BACKGROUND_COLOR), true), makeRgb(0, 0, 0), 192));

  for (int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);

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

  hue->redraw();
  satval->redraw();
}

void Wheel::changeWithCB()
{
  change();

  // run callback for this group set by parent
  if (callback())
    do_callback();
}

int Wheel::getColor()
{
  return color;
}

void Wheel::setColor(const int c)
{
  color = c;
}

void Wheel::update(const int c)
{
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);

  int h, s, v;

  Blend::rgbToHsv(r, g, b, &h, &s, &v);

  float angle = ((M_PI * 2) / 1536) * h;
  int mx = 96 + 88 * std::cos(angle);
  int my = 96 + 88 * std::sin(angle);

  color = c;
  hue->var = mx + 192 * my;
  satval->var = (int)(s / 2.68) + 96 * (int)(v / 2.68);
  change();
}

