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

#include <vector>

#include "Bitmap.H"
#include "Blend.H"
#include "FileSP.H"
#include "KDtree.H"
#include "Inline.H"
#include "Palette.H"
#include "Widget.H"

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

