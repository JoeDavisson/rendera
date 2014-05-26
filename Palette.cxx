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
  else if(max <= 1024)
    step = 3;
  else if(max <= 2304)
    step = 2;
  else if(max <= 9216)
    step = 1;

  widget->stepx = step;
  widget->stepy = step;

  int div = 96 / step;

  widget->bitmap->clear(makecol(0, 0, 0));

  // put x's on empty palette locations
  if(step)
  {
    for(y = 0; y < 96; y++)
    {
      for(x = 0; x < 96; x++)
      {
        if((x % step) == (y % step))
        {
          widget->bitmap->setpixel_solid(x, y, makecol(255, 255, 255), 0);
          widget->bitmap->setpixel_solid(95 - x, y, makecol(255, 255, 255), 0);
        }
      }
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

