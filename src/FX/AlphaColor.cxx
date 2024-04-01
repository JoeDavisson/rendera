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

#include "AlphaColor.H"

void AlphaColor::apply(Bitmap *bmp, int color)
{
  Gui::progressShow(bmp->h);

  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      int c = Blend::trans(*p, color, geta(*p));

      c &= 0xffffff;
      *p &= 0xff000000;
      *p |= c;
      p++;
    }

    if (Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void AlphaColor::begin()
{
  Project::undo->push();
  apply(Project::bmp, Project::brush->color);
}

