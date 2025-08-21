/*
Copyright (c) 2025 Joe Davisson.

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

#include "Randomize.H"

void Randomize::apply()
{
  Bitmap *bmp = Project::bmp;

  for (int j = 0; j < 1; j++)
  {
    // horizontal
    for (int y = bmp->ct; y <= bmp->cb; y += 1)
    {
      for (int x = bmp->cl + 1 + j; x <= bmp->cr; x += 2)
      {
        if ((rnd() & 1) == 1)
        {
          const int temp = bmp->getpixel(x, y);

          bmp->setpixel(x, y, bmp->getpixel(x - 1, y), 128);
          bmp->setpixel(x - 1, y, temp, 128);
        }
      }
    }

    // vertical
    for (int x = bmp->cl; x <= bmp->cr; x += 1)
    {
      for (int y = bmp->ct + 1 + j; y <= bmp->cb; y += 2)
      {
        if ((rnd() & 1) == 1)
        {
          const int temp = bmp->getpixel(x, y);

          bmp->setpixel(x, y, bmp->getpixel(x, y - 1), 128);
          bmp->setpixel(x, y - 1, temp, 128);
        }
      }
    }
  }
}

void Randomize::begin()
{
  Project::undo->push();
  apply();
}

