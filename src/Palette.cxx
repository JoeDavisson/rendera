/*
Copyright (c) 2015 Joe Davisson.

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

#include "Bitmap.H"
#include "Blend.H"
#include "FileSP.H"
#include "Inline.H"
#include "Octree.H"
#include "Palette.H"
#include "Widget.H"

namespace
{
  bool sortByLum(const int &c1, const int &c2)
  {
    return getl(c1) < getl(c2);
  }
}

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
  if(max > 256)
    return;

  int w = widget->w();
  int h = widget->h();

  int step = std::sqrt((w * h) / max);

  while((w % step) != 0)
    step--;

  widget->stepx = step;
  widget->stepy = step;

  widget->bitmap->clear(makeRgb(0, 0, 0));

  for(int y = 0; y < h; y += step)
  {
    for(int x = 0; x < w; x += step)
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

  for(int y = 0; y < h / step; y++)
  {
    for(int x = 0; x < w / step; x++)
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

void Palette::copy(Palette *dest)
{
  for(int i = 0; i < 256; i++)
    dest->data[i] = data[i];

  dest->max = max;
}

void Palette::insertColor(int color, int index)
{
  if(max >= 256)
    return;

  max++;

  for(int i = max - 1; i > index; i--)
    data[i] = data[i - 1];

  data[index] = color;
}

void Palette::deleteColor(int index)
{
  if(max <= 1)
    return;

  for(int i = index; i < max - 1; i++)
    data[i] = data[i + 1];

  max--;
}

void Palette::replaceColor(int color, int index)
{
  data[index] = color;
}

void Palette::fillTable()
{
  delete table;
  table = new Octree();

  const int step = 4;

  // each 4x4 block of the color cube gets a palette entry
  // close enough and avoids a huge octree
  for(int b = 0; b <= 256 - step; b += step)
  {
    for(int g = 0; g <= 256 - step; g += step)
    {
      for(int r = 0; r <= 256 - step; r += step)
      {
        int c = makeRgb24(r, g, b);
        int smallest = 0xFFFFFF;
        int use = 0;

        for(int i = 0; i < max; i++)
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
  for(int i = 0; i < max; i++)
    table->writePath(getr(data[i]), getg(data[i]), getb(data[i]), i);
}

int Palette::lookup(const int &c)
{
  rgba_type rgba = getRgba(c);

  return table->read(rgba.r, rgba.g, rgba.b);
}

void Palette::sort()
{
  std::sort(data, data + max, sortByLum);
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

  while(true)
  {
    for(int i = 0; i < 255; i++)
    {
      line[i] = '\0';
      ch = fgetc(in.get());
      if(ch == '\n' || ch == EOF)
        break;
      line[i] = ch;
    }

    if(ch == EOF)
      break;

    // replace tabs with spaces
    for(int i = 0; i < len; i++)
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

  max = index;
  fillTable();

  return 0;
}

int Palette::save(const char *fn)
{
  FileSP out(fn, "w");

  if(fprintf(out.get(), "GIMP Palette\n") < 0)
    return -1;

  if(fprintf(out.get(), "#\n") < 0)
    return -1;

  for(int i = 0; i < max; i++)
  {
    int c = data[i];

    if(fprintf(out.get(), "%d %d %d\n", getr(c), getg(c), getb(c)) < 0)
      return -1;
  }

  return 0;
}

void Palette::setDefault()
{
  int r, g, b;
  int index = 0;

  for(int h = 0; h < 31; h++)
  {
    for(int v = 0; v < 8; v++)
    {
      int val = 16 + v * 32;
      if(v == 0)
        val = 28;
      if(v == 7)
        val = 232;
      Blend::hsvToRgb(h * 49.55, 255, 255, &r, &g, &b);

      switch(v)
      {
        case 0:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(0, 0, 0), 64);
          break;
        case 1:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(0, 0, 0), 96);
          break;
        case 2:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(0, 0, 0), 128);
          break;
        case 3:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(0, 0, 0), 192);
          break;
        case 4:
          data[index] = makeRgb(r, g, b);
          break;
        case 5:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(255, 255, 255), 192);
          break;
        case 6:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(255, 255, 255), 128);
          break;
        case 7:
          data[index] = makeRgb(r, g, b);
          data[index] = Blend::trans(data[index], makeRgb(255, 255, 255), 64);
          break;
      }

      index++;
    }
  }

  // grays
  for(int v = 0; v < 8; v++)
  {
    int val = v * 32;
    if(v == 7)
      val = 255;
    data[index++] = makeRgb(val, val, val);
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

void Palette::setWebSafe()
{
  int index = 0;

  for(int b = 0; b < 6; b++)
  {
    for(int g = 0; g < 6; g++)
    {
      for(int r = 0; r < 6; r++)
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

  for(int r = 0; r < 3; r++)
  {
    for(int g = 0; g < 3; g++)
    {
      for(int b = 0; b < 3; b++)
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

  for(int r = 0; r < 4; r++)
  {
    for(int g = 0; g < 4; g++)
    {
      for(int b = 0; b < 4; b++)
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
  int index = 0;

  for(int b = 0; b < 4; b++)
  {
    for(int g = 0; g < 8; g++)
    {
      for(int r = 0; r < 8; r++)
      {
        data[index++] = makeRgb(r * 36.43, g * 36.43, b * 85);
      }
    }
  }

  max = index;
  fillTable();
}

