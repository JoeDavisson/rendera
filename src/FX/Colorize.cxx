/*
Copyright (c) 2021 Joe Davisson.

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

#include "Colorize.H"

void Colorize::apply(Bitmap *bmp, int color)
{
  rgba_type rgba_color = getRgba(color);

  Gui::progressShow(bmp->h);

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      int r = rgba.r;
      int g = rgba.g;
      int b = rgba.b;
      int h, s, v;

      Blend::rgbToHsv(r, g, b, &h, &s, &v);

      int sat = s;

      if(sat < 64)
	sat = 64;

      r = rgba_color.r;
      g = rgba_color.g;
      b = rgba_color.b;
      Blend::rgbToHsv(r, g, b, &h, &s, &v);
      Blend::hsvToRgb(h, (sat * s) / (sat + s), v, &r, &g, &b);

      *p = Blend::colorize(*p, makeRgba(r, g, b, rgba.a), 0);
      p++;
    }

    if(Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void Colorize::begin()
{
  Project::undo->push();
  apply(Project::bmp, Project::brush->color);
}

