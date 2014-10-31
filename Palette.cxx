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

#include "Bitmap.H"
#include "Blend.H"
#include "Octree.H"
#include "Palette.H"
#include "Widget.H"

Palette::Palette()
{
  data = new int[256];
  table = new Octree();
  setDefault();
}

Palette::~Palette()
{
  delete table;
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
  else if(max <= 9)
    step = 32;
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

  widget->bitmap->clear(makeRgb(0, 0, 0));

  for(y = 0; y < w; y += step)
  {
    for(x = 0; x < w; x += step)
    {
      widget->bitmap->rect(x, y, x + step, y + step,
                           makeRgb(64, 64, 64), 0);
      widget->bitmap->line(x, y, x + step - 1, y + step - 1,
                           makeRgb(64, 64, 64), 0);
      widget->bitmap->line(w - 1 - x, y, w - 1 - (x + step - 1), y + step - 1,
                           makeRgb(64, 64, 64), 0);
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
      widget->bitmap->hline(x1, y1, x1 + step - 1, makeRgb(0, 0, 0), 128);
      widget->bitmap->vline(y1 + 1, x1, y1 + step - 1, makeRgb(0, 0, 0), 128);
      i++;
    }
  }

  widget->redraw();
}

void Palette::copy(SP<Palette> dest)
{
  int i;

  for(i = 0; i < 256; i++)
    dest->data[i] = data[i];

  dest->max = max;
}

void Palette::insertColor(int color, int index)
{
  if(max >= 256)
    return;

  max++;

  int i;

  for(i = max - 1; i > index; i--)
    data[i] = data[i - 1];

  data[index] = color;
}

void Palette::deleteColor(int index)
{
  if(max <= 1)
    return;

  int i;

  for(i = index; i < max - 1; i++)
    data[i] = data[i + 1];

  max--;
}

void Palette::replaceColor(int color, int index)
{
  data[index] = color;
}

// create palette lookup table
void Palette::fillTable()
{
  delete table;
  table = new Octree();

  int r, g, b;
  int use, i;
  int step = 4;

  // each 4x4 block of the color cube gets a palette entry
  // close enough and avoids a huge octree
  for(b = 0; b <= 256 - step; b += step)
  {
    for(g = 0; g <= 256 - step; g += step)
    {
      for(r = 0; r <= 256 - step; r += step)
      {
        int c = makeRgb24(r, g, b);
        int smallest = 0xFFFFFF;
        use = 0;

        for(i = 0; i < max; i++)
        {
          int d = diff24(c, data[i]);
          if(d < smallest)
          {
            smallest = d;
            use = i;
          }
        }

        table->writePath(r, g, b, use);
      }
    }
  }

  // put exact matches back in
  for(i = 0; i < max; i++)
    table->writePath(getr(data[i]), getg(data[i]), getb(data[i]), i);
}

// return palette index which corresponds to color
int Palette::lookup(const int &c)
{
  struct rgba_t rgba = getRgba(c);

  return table->read(rgba.r, rgba.g, rgba.b);
}

// sort palette
void Palette::sort()
{
  Common::sortBy(data, data + max, getl);
}

// uses GIMP .gpl palette format
int Palette::load(const char *fn)
{
  max = 0;

  FILE *in = fopen(fn, "r");
  if(!in)
    return -1;

  int index = 0;

  // read line
  int r, g, b;
  char line[256];
  int ch;
  int len = 0;
  int i;

  while(true)
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
    data[index++] = makeRgb(r, g, b);
    if(index > 256)
      break;
  }

  fclose(in);
  max = index;
  fillTable();

  return 0;
}

// uses GIMP .gpl palette format
int Palette::save(const char *fn)
{
  FILE *out = fopen(fn, "w");

  if(!out)
    return -1;

  if(fprintf(out, "GIMP Palette\n") < 0)
  {
    fclose(out);
    return -1;
  }

  if(fprintf(out, "#\n") < 0)
  {
    fclose(out);
    return -1;
  }

  int i;

  for(i = 0; i < max; i++)
  {
    int c = data[i];

    if(fprintf(out, "%d %d %d\n", getr(c), getg(c), getb(c)) < 0)
    {
      fclose(out);
      return -1;
    }
  }

  fclose(out);

  return 0;
}

// various fixed palettes
void Palette::setDefault()
{
  int r, g, b;
  int h, v;
  int index = 0;

  const int sat[5] = { 255, 255, 255, 128, 64 };
  const int val[5] = { 64, 128, 255, 255, 255 };

  // colors
  for(v = 4; v >= 0; v--)
  {
    for(h = 0; h < 12; h++)
    {
      Blend::hsvToRgb(h * 128, sat[v], val[v], &r, &g, &b);

      data[index++] = makeRgb(r, g, b);
    }
  }

  // grays
  for(v = 0; v < 12; v++)
    data[index++] = makeRgb(v * 23.19, v * 23.19, v * 23.19);

  max = index;
  fillTable();
}

void Palette::setWebSafe()
{
  int r, g, b;
  int index = 0;

  for(b = 0; b < 6; b++)
  {
    for(g = 0; g < 6; g++)
    {
      for(r = 0; r < 6; r++)
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
  int r, g, b;
  int index = 0;

  for(r = 0; r < 3; r++)
  {
    for(g = 0; g < 3; g++)
    {
      for(b = 0; b < 3; b++)
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
  int r, g, b;
  int index = 0;

  for(r = 0; r < 4; r++)
  {
    for(g = 0; g < 4; g++)
    {
      for(b = 0; b < 4; b++)
      {
        data[index++] = makeRgb(r * 85, g * 85, b * 85);
      }
    }
  }

  max = index;
  fillTable();
}

void Palette::set332()
{
  int r, g, b;
  int index = 0;

  for(b = 0; b < 4; b++)
  {
    for(g = 0; g < 8; g++)
    {
      for(r = 0; r < 8; r++)
      {
        data[index++] = makeRgb(r * 36.43, g * 36.43, b * 85);
      }
    }
  }

  max = index;
  fillTable();
}

