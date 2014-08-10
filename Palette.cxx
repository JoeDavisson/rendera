/*
Copyright (c) 2014 Joe Davisson.

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

#include "rendera.h"

Palette *Palette::main;
Palette *Palette::undo;

Palette::Palette()
{
  data = new int[256];
  lookup = new unsigned char[16777216];
  set_default();
  fill_lookup();
}

Palette::~Palette()
{
  delete[] data;
}

void Palette::draw(Widget *widget)
{
  int x, y;
  int w = widget->w();

  int step = 3;

  if(max > 256)
    return;

  if(max <= 4)
    step = 48;
  else if(max <= 16)
    step = 24;
  else if(max <= 36)
    step = 16;
  else if(max <= 64)
    step = 12;
  else if(max <= 144)
    step = 8;
  else if(max <= 256)
    step = 6;

  // for large palette
  if(widget->w() == 192)
    step *= 2;

  widget->stepx = step;
  widget->stepy = step;

  int div = w / step;

  widget->bitmap->clear(makecol(0, 0, 0));

  for(y = 0; y < w; y += step)
  {
    for(x = 0; x < w; x += step)
    {
      widget->bitmap->rect(x, y, x + step, y + step, makecol(160, 160, 160), 0);
      widget->bitmap->line(x, y, x + step - 1, y + step - 1, makecol(160, 160, 160), 0);
      widget->bitmap->line(95 - x, y, 95 - (x + step - 1), y + step - 1, makecol(160, 160, 160), 0);
    }
  }

  int i = 0;
  for(y = 0; y < div; y++)
  {
    for(x = 0; x < div; x++)
    {
      if(i >= max)
        break;
      int x1 = x * step;
      int y1 = y * step;
      widget->bitmap->rectfill(x1, y1, x1 + step - 1, y1 + step - 1, data[i], 0);
      i++;
    }
  }

  widget->redraw();
}

void Palette::copy(Palette *dest)
{
  int i;

  for(i = 0; i < 256; i++)
    dest->data[i] = data[i];

  dest->max = max;
}

void Palette::set_default()
{
  int r, g, b;
  int h, s, v;
  int index = 0;

  int sat[12] = { 255, 255, 255, 255, 255, 255, 192, 160, 128, 96, 64, 32 };
  int val[12] = { 64, 96, 128, 160, 192, 255, 255, 255, 255, 255, 255, 255 };

  for(v = 10; v >= 0; v--)
  {
    for(h = 0; h < 12; h++)
    {
      Blend::hsv_to_rgb(h * 128, sat[v], val[v], &r, &g, &b);

      data[index++] = makecol(r, g, b);
    }
  }

  for(v = 0; v < 12; v++)
  {
    data[index++] = makecol(v * 23.19, v * 23.19, v * 23.19);
  }

  max = index;
}

void Palette::insert_color(int color, int index)
{
  if(max >= 256)
    return;

  max++;

  int i;

  for(i = max - 1; i > index; i--)
    data[i] = data[i - 1];

  data[index] = color;
}

void Palette::delete_color(int index)
{
  if(max <= 1)
    return;

  int i;

  for(i = index; i < max - 1; i++)
    data[i] = data[i + 1];

  max--;
}

void Palette::replace_color(int color, int index)
{
  data[index] = color;
}

void Palette::fill_lookup()
{
  int r, g, b;
  int i, j, k;
  int use, z;
  int step = 4;

  // fill lookup in 4x4x4 blocks to save time
  for(b = 0; b <= 256 - step; b += step)
  {
    for(g = 0; g <= 256 - step; g += step)
    {
      for(r = 0; r <= 256 - step; r += step)
      {
        int c = makecol_notrans(r, g, b);
        int smallest = 0xFFFFFF;
        use = 0;
        for(z = 0; z < max; z++)
        {
          int d = diff24(c, data[z]);
          if(d < smallest)
          {
            smallest = d;
            use = z;
          }
        }

        for(k = b; k < b + step; k++)
        {
          for(j = g; j < g + step; j++)
          {
            for(i = r; i < r + step; i++)
            {
              lookup[makecol_notrans(i, j, k)] = use;
            }
          }
        }
      }
    }
  }


  // then put exact matches back in
  for(z = 0; z < max; z++)
    lookup[data[z] & 0xFFFFFF] = z;
}

// uses GIMP .gpl palette format
void Palette::load(const char *fn)
{
  max = 0;

  FILE *in = fl_fopen(fn, "r");
  if(!in)
  {
    return;
  }

  int index = 0;

  // read line
  int r, g, b;
  char line[256];
  int ch;
  int len = 0;
  int i, j;

  while(1)
  {
    for(i = 0; i < 255; i++)
    {
      line[i] = '\0';
      ch = fgetc(in);
      if(ch == '\n' || ch == EOF)
        break;
      line[i] = ch;
    }

    if(ch == EOF)
      break;

    // replace tabs with spaces
    for(i = 0; i < len; i++)
      if(line[i] == '\t')
        line[i] = ' ';

    // get first three strings
    if(sscanf(line, "%d %d %d", &r, &g, &b) != 3)
      continue;

    // add to palette
    data[index++] = makecol(r, g, b);
    if(index > 256)
      break;
  }

  max = index;
  fill_lookup();
}

