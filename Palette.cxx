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

static inline uint8_t parse_uint8(unsigned char *&buffer)
{
  uint8_t num = buffer[0];

  buffer += 1;
  return num;
}

static inline uint16_t parse_uint16(unsigned char *&buffer)
{
  uint16_t num;

  #if BYTE_ORDER == BIG_ENDIAN
  num = buffer[1] | buffer[0] << 8;
  #else
  num = buffer[0] | buffer[1] << 8;
  #endif

  buffer += 2;
  return num;
}

static inline uint32_t parse_uint32(unsigned char *&buffer)
{
  uint32_t num;

  #if BYTE_ORDER == BIG_ENDIAN
  num = buffer[3] | buffer[2] << 8 | buffer[1] << 16 | buffer[0] << 24;
  #else
  num = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
  #endif

  buffer += 4;
  return num;
}

Palette *Palette::main;

Palette::Palette()
{
  data = new int[256];
  set_default();
}

Palette::~Palette()
{
  delete[] data;
}

void Palette::draw(Widget *widget)
{
  int x, y;

  int step = 3;

  if(max > 256)
    return;

  if(max <= 4)
    step = 48;
  else if(max <= 16)
    step = 24;
  else if(max <= 64)
    step = 12;
  else if(max <= 256)
    step = 6;

  widget->stepx = step;
  widget->stepy = step;

  int div = 96 / step;

  widget->bitmap->clear(makecol(0, 0, 0));

  for(y = 0; y < 96; y += step)
  {
    for(x = 0; x < 96; x += step)
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

void Palette::set_default()
{
  int r, g, b;
  int i = 0;

  for(r = 0; r < 3; r++)
  {
    for(g = 0; g < 3; g++)
    {
      for(b = 0; b < 3; b++)
      {
        data[i++] = makecol(MIN(r * 128, 255),
                         MIN(g * 128, 255), MIN(b * 128, 255));
      }
    }
  }

  max = i;
}

void Palette::insert_color(int color, int index)
{
  if(max >= 256)
    return;

  int i;

  for(i = max - 1; i > index; i--)
    data[i] = data[i - 1];

  data[index] = color;

  max++;
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
}

