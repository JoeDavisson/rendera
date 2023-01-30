/*
Copyright (c) 2023 Joe Davisson.

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

#include "Normalize.H"

void Normalize::apply(Bitmap *bmp)
{
  // search for highest & lowest RGB values
  int r_high = 0;
  int g_high = 0;
  int b_high = 0;
  int r_low = 0xFFFFFF;
  int g_low = 0xFFFFFF;
  int b_low = 0xFFFFFF;

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      const int r = rgba.r;
      const int g = rgba.g;
      const int b = rgba.b;

      if(r < r_low)
        r_low = r;
      if(r > r_high)
        r_high = r;
      if(g < g_low)
        g_low = g;
      if(g > g_high)
        g_high = g;
      if(b < b_low)
        b_low = b;
      if(b > b_high)
        b_high = b;

      p++;
    }
  }

  if(!(r_high - r_low))
    r_high++;
  if(!(g_high - g_low))
    g_high++;
  if(!(b_high - b_low))
    b_high++;

  // scale image
  double r_scale = 255.0 / (r_high - r_low);
  double g_scale = 255.0 / (g_high - g_low);
  double b_scale = 255.0 / (b_high - b_low);

  Gui::progressShow(bmp->h);

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      const int r = (rgba.r - r_low) * r_scale;
      const int g = (rgba.g - g_low) * g_scale;
      const int b = (rgba.b - b_low) * b_scale;

      *p = makeRgba(r, g, b, rgba.a);
      p++;
    }

    if(Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void Normalize::begin()
{
  Project::undo->push();
  apply(Project::bmp);
}

