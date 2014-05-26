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

static int factors[] = { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 96, 192 };

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
  int stepx = 6;
  int stepy = 6;

  if(max > 256)
    return;

  int x = 0, y = 0;

  for(x = sqrtf(max); x >= 0; x--)
  {
    y = max / x;
    if(x * y >= max)
        break;
  }

  int tempx = x;
  int tempy = y;

  for(x = 0; x < 12; x++)
  {
    if(factors[x] >= tempx)
    {
      stepx = 96 / factors[x];
      break;
    }
  }
      
  for(y = 0; y < 13; y++)
  {
    if(factors[y] >= tempy)
    {
      stepy = 192 / factors[y];
      break;
    }
  }

  widget->stepx = stepx;
  widget->stepy = stepy;

  int divx = 96 / stepx;
  int divy = 192 / stepy;

  widget->bitmap->clear(makecol(0, 0, 0));

  for(y = 0; y < 192; y++)
  {
    for(x = 0; x < 96; x++)
    {
      if((x & 1) ^ (y & 1))
      {
        widget->bitmap->setpixel_solid(x, y, makecol(255, 255, 255), 0);
      }
    }
  }

  int i = 0;
  for(y = 0; y < divy; y++)
  {
    for(x = 0; x < divx; x++)
    {
      if(i >= max)
        break;
      int x1 = x * stepx;
      int y1 = y * stepy;
      widget->bitmap->rectfill(x1, y1, x1 + stepx - 1, y1 + stepy - 1, data[i], 0);
      i++;
    }
  }
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

