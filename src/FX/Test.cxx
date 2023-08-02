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

#include "Test.H"

void Test::apply(Bitmap *bmp)
{
  Gui::progressShow(bmp->h);

  for (int y = bmp->ct; y <= bmp->cb - 8; y += 1)
  {
    for (int x = bmp->cl; x <= bmp->cr - 8; x += 1)
    {
      if (x & y)
        bmp->setpixel(x, y, makeRgb(255, 255, 255));
      else
        bmp->setpixel(x, y, makeRgb(0, 0, 0));
/*
      for (int j = 0; j < 8; j++)
      {
        for (int i = 0; i < 8; i++)
        {
          if ((z + (i & j)) > (i + j) & 7)
            bmp->setpixel(x + i, y + j, makeRgb(255, 255, 255));
          else
            bmp->setpixel(x + i, y + j, makeRgb(0, 0, 0));
        }
      }
*/
    }

    if (Gui::progressUpdate(y) < 0)
      return;

  }

  Gui::progressHide();
}

void Test::begin()
{
  Project::undo->push();
  apply(Project::bmp);
}

