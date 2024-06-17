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

#include <algorithm>
#include <cmath>

#include "Bitmap.H"
#include "Blend.H"
#include "FileSP.H"
#include "KDtree.H"
#include "Inline.H"
#include "Palette.H"
#include "Widget.H"

static bool sortByLum(const int c1, const int c2)
{
  return getl(c1) < getl(c2);
}

Palette::Palette()
{
  // palette color data
  data = new int[256];

  // data structure for color lookup
  table = new unsigned char[16777216];

  // use a default palette
  setDefault();
}

Palette::~Palette()
{
  delete[] table;
  delete[] data;
}

// draw palette swatches to a widget, and try to fill the space optimally
void Palette::draw(Widget *widget)
{
  if (max > 256)
    return;

  int w = widget->w();
  int h = widget->h();

  int step = std::sqrt((w * h) / max);

  while ((w % step) != 0)
    step--;

  widget->stepx = step;
  widget->stepy = step;
  widget->bitmap->clear(makeRgb(0, 0, 0));

  for (int y = 0; y < h; y += step)
  {
    for (int x = 0; x < w; x += step)
    {
      widget->bitmap->rect(x, y, x + step, y + step,
                           makeRgb(255, 255, 255), 160);
      widget->bitmap->line(x, y, x + step - 1, y + step - 1,
                           makeRgb(255, 255, 255), 160);
      widget->bitmap->line(w - 1 - x, y, w - 1 - (x + step - 1), y + step - 1,
                           makeRgb(255, 255, 255), 160);
    }
  }

  int i = 0;

  for (int y = 0; y < h / step; y++)
  {
    for (int x = 0; x < w / step; x++)
    {
      if (i >= max)
        break;
      int x1 = x * step;
      int y1 = y * step;

      widget->bitmap->rectfill(x1, y1, x1 + step - 1, y1 + step - 1,
                               data[i], 0);
      i++;
    }
  }

  for (int y = 0; y < h; y += step)
  {
    for (int x = 0; x < w; x += step)
    {
      widget->bitmap->hline(x, y, x + step - 1, makeRgb(0, 0, 0), 128);
      widget->bitmap->vline(y, x, y + step - 1, makeRgb(0, 0, 0), 128);
    }
  }

  int div = widget->w() / step;
  int px = widget->var % div;
  int py = widget->var / div;

  int x = px * step;
  int y = py * step;

  widget->bitmap->rect(0, 0, w - 1, h - 1, makeRgb(0, 0, 0), 0);
  widget->bitmap->rectfill(x, y, x + step - 1, y + step - 1,
                           data[widget->var], 0);
  widget->bitmap->rect(x, y, x + step - 1, y + step - 1, makeRgb(0, 0, 0), 0);
  widget->bitmap->rect(x - 1, y - 1, x + step, y + step, makeRgb(0, 0, 0), 96);
  widget->bitmap->rect(x - 2, y - 2, x + step + 1, y + step + 1,
                       makeRgb(0, 0, 0), 160);
  widget->bitmap->xorRect(x, y, x + step - 1, y + step - 1);

  widget->redraw();
}

// functions to modify palette (be sure to call fillTable afterwards)
void Palette::copy(Palette *dest)
{
  for (int i = 0; i < 256; i++)
    dest->data[i] = data[i];

  dest->max = max;
}

void Palette::insertColor(int color, int index)
{
  if (max >= 256)
    return;

  max++;

  for (int i = max - 1; i > index; i--)
    data[i] = data[i - 1];

  data[index] = color;
}

void Palette::deleteColor(int index)
{
  if (max <= 1)
    return;

  for (int i = index; i < max - 1; i++)
    data[i] = data[i + 1];

  max--;
}

void Palette::replaceColor(int color, int index)
{
  data[index] = color;
}

void Palette::swapColor(int c1, int c2)
{
  int temp = data[c1];

  data[c1] = data[c2];
  data[c2] = temp;
}

// generate color lookup table
void Palette::fillTable()
{
  delete[] table;
  table = new unsigned char[16777216];

  KDtree::node_type test_node;
  KDtree::node_type *root, *found;
  std::vector<KDtree::node_type> colors(max);

  int best_dist;
  const int step = 4;

  for (int i = 0; i < max; i++)
  {
    const int c = data[i];
    colors[i].x[0] = getr(c);
    colors[i].x[1] = getg(c);
    colors[i].x[2] = getb(c);
    colors[i].index = i;
  }

  root = KDtree::build(&colors[0], max, 0);

  for (int b = 0; b <= 256 - step; b += step)
  {
    for (int g = 0; g <= 256 - step; g += step)
    {
      for (int r = 0; r <= 256 - step; r += step)
      {
        test_node.x[0] = r + step / 2;
        test_node.x[1] = g + step / 2;
        test_node.x[2] = b + step / 2;

        found = 0;
        KDtree::nearest(root, &test_node, &found, &best_dist, 0);

        for (int k = 0; k < step; k++)
        {
          const int bk = b + k;

          for (int j = 0; j < step; j++)
          {
            const int gj = g + j;

            for (int i = 0; i < step; i++)
            {
              const int ri = r + i;

              table[makeRgb24(ri, gj, bk)] = found->index;
            }
          }
        }
      }
    }
  }

  for (int i = 0; i < max; i++)
    table[data[i] & 0xffffff] = i;
}

// return the nearest palette entry for an RGB color
int Palette::lookup(const int c)
{
  return table[c & 0xffffff];
}

void Palette::sort()
{
  std::sort(data, data + max, sortByLum);
}

void Palette::normalize()
{
  // search for highest & lowest RGB values
  int r_high = 0;
  int g_high = 0;
  int b_high = 0;
  int r_low = 0xffffff;
  int g_low = 0xffffff;
  int b_low = 0xffffff;

  for (int i = 0; i < max; i++)
  {
    rgba_type rgba = getRgba(data[i]);

    const int r = rgba.r;
    const int g = rgba.g;
    const int b = rgba.b;

    if (r < r_low)
      r_low = r;
    if (r > r_high)
      r_high = r;
    if (g < g_low)
      g_low = g;
    if (g > g_high)
      g_high = g;
    if (b < b_low)
      b_low = b;
    if (b > b_high)
      b_high = b;
  }

  if (!(r_high - r_low))
    r_high++;
  if (!(g_high - g_low))
    g_high++;
  if (!(b_high - b_low))
    b_high++;

  // scale palette
  double r_scale = 255.0 / (r_high - r_low);
  double g_scale = 255.0 / (g_high - g_low);
  double b_scale = 255.0 / (b_high - b_low);

  for (int i = 0; i < max; i++)
  {
    rgba_type rgba = getRgba(data[i]);

    const int r = (rgba.r - r_low) * r_scale;
    const int g = (rgba.g - g_low) * g_scale;
    const int b = (rgba.b - b_low) * b_scale;

    data[i] = makeRgb(r, g, b);
  }

  fillTable();
}

int Palette::load(const char *fn)
{
  max = 0;

  FileSP in(fn, "r");

  int index = 0;

  // read line
  int r, g, b;
  char line[256];
  int ch;
  int len = 0;

  while (true)
  {
    for (int i = 0; i < 255; i++)
    {
      line[i] = '\0';
      ch = fgetc(in.get());
      if (ch == '\n' || ch == EOF)
        break;
      line[i] = ch;
    }

    if (ch == EOF)
      break;

    // replace tabs with spaces
    for (int i = 0; i < len; i++)
      if (line[i] == '\t')
        line[i] = ' ';

    // get first three strings
    if (sscanf(line, "%d %d %d", &r, &g, &b) != 3)
      continue;

    // add to palette
    data[index++] = makeRgb(r, g, b);
    if (index > 256)
      break;
  }

  max = index;
  fillTable();

  return 0;
}

int Palette::save(const char *fn)
{
  FileSP out(fn, "w");

  if (fprintf(out.get(), "GIMP Palette\n") < 0)
    return -1;

  if (fprintf(out.get(), "#\n") < 0)
    return -1;

  for (int i = 0; i < max; i++)
  {
    int c = data[i];

    if (fprintf(out.get(), "%d %d %d\n", getr(c), getg(c), getb(c)) < 0)
      return -1;
  }

  return 0;
}

void Palette::setDefault()
{
  static int hue[] =
  {
    0, 109, 192, 256, 328, 364, 512, 657,
    768, 864, 940, 1024, 1160, 1280, 1425
  }; 

  static int sat[] = { 192, 255, 144, 96 };
  static int val[] = { 128, 255, 255, 255 };

  int index = 0;

  // grays
  data[index++] = makeRgb(255, 255, 255);
  data[index++] = makeRgb(160, 160, 160);
  data[index++] = makeRgb(96, 96, 96);
  data[index++] = makeRgb(0, 0, 0);

  //colors
  for (int h = 0; h < 15; h++)
  {
    for (int v = 3; v >= 0; v--)
    {
      int r, g, b;

      Blend::hsvToRgb(hue[h] / 32 * 32, sat[v], val[v], &r, &g, &b);
      data[index++] = makeRgb(r, g, b);
    }
  }

  max = index;
  fillTable();
}

void Palette::setBlackAndWhite()
{
  data[0] = makeRgb(0, 0, 0);
  data[1] = makeRgb(255, 255, 255);

  max = 2;
  fillTable();
}

void Palette::setGrays()
{
  for (int i = 0; i < 16; i++)
  {
    data[i] = makeRgb(i * 17, i * 17, i * 17);
  }

  max = 16;
  fillTable();
}

void Palette::setTwoBits()
{
  data[0] = makeRgb(0x00, 0x00, 0x00);
  data[1] = makeRgb(0x4a, 0x4a, 0x4a);
  data[2] = makeRgb(0x7b, 0x7b, 0x7b);
  data[3] = makeRgb(0xb2, 0xb2, 0xb2);

  max = 4;
  fillTable();
}

// from colodore.com
void Palette::setC64()
{
  data[0] = makeRgb(0, 0, 0);
  data[1] = makeRgb(255, 255, 255);
  data[2] = makeRgb(129, 51, 56);
  data[3] = makeRgb(117, 206, 200);
  data[4] = makeRgb(142, 60, 151);
  data[5] = makeRgb(86, 172, 77);
  data[6] = makeRgb(46, 44, 155);
  data[7] = makeRgb(237, 241, 113);
  data[8] = makeRgb(142, 80, 41);
  data[9] = makeRgb(85, 56, 0);
  data[10] = makeRgb(196, 108, 113);
  data[11] = makeRgb(74, 74, 74);
  data[12] = makeRgb(123, 123, 123);
  data[13] = makeRgb(169, 255, 159);
  data[14] = makeRgb(112, 109, 235);
  data[15] = makeRgb(178, 178, 178);

  max = 16;
  fillTable();
}

void Palette::setWebSafe()
{
  int index = 0;

  for (int b = 0; b < 6; b++)
  {
    for (int g = 0; g < 6; g++)
    {
      for (int r = 0; r < 6; r++)
      {
        data[index++] = makeRgb(r * 51, g * 51, b * 51);
      }
    }
  }

  max = index;
  fillTable();
}

void Palette::set3LevelRGB()
{
  int index = 0;

  for (int r = 0; r < 3; r++)
  {
    for (int g = 0; g < 3; g++)
    {
      for (int b = 0; b < 3; b++)
      {
        data[index++] = makeRgb(std::min(r * 128, 255),
                                std::min(g * 128, 255),
                                std::min(b * 128, 255));
      }
    }
  }

  max = index;
  fillTable();
}

void Palette::set4LevelRGB()
{
  int index = 0;

  for (int r = 0; r < 4; r++)
  {
    for (int g = 0; g < 4; g++)
    {
      for (int b = 0; b < 4; b++)
      {
        data[index++] = makeRgb(r * 85, g * 85, b * 85);
      }
    }
  }

  max = index;
  fillTable();
}

