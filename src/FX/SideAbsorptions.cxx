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

/*
This is inspired by a side-absorption correction technique presented in
"Digital Color Restoration of Faded Motion Pictures" (M. Chambah & B. Besserer).

It works remarkably well on certain "hard-to-fix" photographs, particularly those with a strong red cast. It is normally used as a first step before running
Restore, Equalize, etc.
*/

#include "SideAbsorptions.H"

void SideAbsorptions::apply(Bitmap *bmp)
{
  Gui::progressShow(bmp->h);

  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      int r = rgba.r;
      int g = rgba.g;
      int b = rgba.b;
      int h, s, v, old_s;

      Blend::rgbToHsv(r, g, b, &h, &old_s, &v);

      int ra = r ;
      int ga = (r * 4 + g * 8 + b * 1) / 13;
      int ba = (r * 2 + g * 4 + b * 8) / 14;

      ra = clamp(ra, 255);
      ga = clamp(ga, 255);
      ba = clamp(ba, 255);

      Blend::rgbToHsv(ra, ga, ba, &h, &s, &v);
      Blend::hsvToRgb(h, old_s, v, &ra, &ga, &ba);

      *p = makeRgba(ra, ga, ba, rgba.a);
      p++;
    }

    if (Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void SideAbsorptions::begin()
{
  Project::undo->push();
  apply(Project::bmp);
}

