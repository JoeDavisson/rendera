/*
Copyright (c) 2025 Joe Davisson.

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
#include <vector>

#include "Bitmap.H"
#include "Blend.H"
#include "Editor.H"
#include "FileSP.H"
#include "KDtree.H"
#include "Inline.H"
#include "Palette.H"
#include "Widget.H"

static bool sort_value_cb(const int c1, const int c2)
{
  return getl(c1) < getl(c2);
}

Palette::Palette()
{
  // palette color data
  data = new int[256];

  // data structure for color lookup
  table = new unsigned char[16777216];

  data[0] = makeRgb(0, 0, 0);
  data[1] = makeRgb(255, 255, 255);
  max = 2;
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

  while (((w / step) & ((w / step) - 1)) != 0)
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

// generate palette lookup table
void Palette::fillTable()
{
  delete[] table;
  table = new unsigned char[16777216];

  KDtree::node_type test_node;
  KDtree::node_type *root;
  std::vector<KDtree::node_type> colors(max);

  int best_dist;

  for (int i = 0; i < max; i++)
  {
    const int c = data[i];
    colors[i].x[0] = getr(c);
    colors[i].x[1] = getg(c);
    colors[i].x[2] = getb(c);
    colors[i].index = i;
  }

  root = KDtree::build(&colors[0], max, 0);

  for (int b = 0; b < 256; b += 4)
  {
    for (int g = 0; g < 256; g += 4)
    {
      for (int r = 0; r < 256; r += 4)
      {
        test_node.x[0] = r + 2;
        test_node.x[1] = g + 2;
        test_node.x[2] = b + 2;

        KDtree::node_type *found = 0;
        KDtree::nearest(root, &test_node, &found, &best_dist, 0);

        for (int k = 0; k < 4; k++)
        {
          const int bk = b + k;

          for (int j = 0; j < 4; j++)
          {
            const int gj = g + j;

            for (int i = 0; i < 4; i++)
            {
              const int ri = r + i;

              table[makeRgb24(ri, gj, bk)] = found->index;
            }
          }
        }
      }
    }
  }

  // put exact matches back in
  for (int z = 0; z < max; z++)
  {
     int r = getr(data[z]);
     int g = getg(data[z]);
     int b = getb(data[z]);

     r = (r / 4) * 4;
     g = (g / 4) * 4;
     b = (b / 4) * 4;

     for (int k = 0; k < 4; k++)
     {
       const int bk = b + k;

       for (int j = 0; j < 4; j++)
       {
         const int gj = g + j;

         for (int i = 0; i < 4; i++)
         {
           const int ri = r + i;

           table[makeRgb24(ri, gj, bk)] = z;
         }
       }
     }
  }
}

// return the nearest palette entry for an RGB color
int Palette::lookup(const int c)
{
  return table[c & 0xffffff];
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

// sort into grays, low-sat colors, hi-sat colors
void Palette::sortByHue()
{
  Editor::push();

  std::vector<int> bucket(25 * 256, 0);
  std::vector<int> count(25, 0);

  int h, s, v;

  for (int j = 0; j < max; j++)
  {
    const int c = data[j];
    const int r = getr(c);
    const int g = getg(c);
    const int b = getb(c);
    Blend::rgbToHsv(r, g, b, &h, &s, &v);

    if (s == 0)
    {
      bucket[count[0]++] = c;
    }
    else if (s < 128)
    {
      for (int i = 0; i < 12; i++)
      {
        if (h >= i * 128 && h < (i + 1) * 128)
        {
          bucket[(i + 1) * 256 + count[i + 1]++] = c;
        }
      }
    }
      else
    {
      for (int i = 0; i < 12; i++)
      {
        if (h >= i * 128 && h < (i + 1) * 128)
        {
          bucket[(i + 13) * 256 + count[i + 13]++] = c;
        }
      }
    }
  }

  for (int i = 0; i < 25; i++)
  {
    std::sort(&bucket[i * 256], &bucket[i * 256] + count[i], sort_value_cb);
  }

  int index = 0;

  for (int i = 0; i < 25; i++)
  {
    for (int j = 0; j < count[i]; j++)
    {
      data[index++] = bucket[i * 256 + j];
    }
  }

  max = index;
  fillTable();
}

void Palette::sortByValue()
{   
  Editor::push();

  std::sort(data, data + max, sort_value_cb);
  fillTable();
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
 
