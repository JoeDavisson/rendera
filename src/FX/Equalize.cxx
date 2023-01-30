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

#include "Equalize.H"

void Equalize::apply(Bitmap *bmp)
{
  std::vector<int> list_r(256, 0);
  std::vector<int> list_g(256, 0);
  std::vector<int> list_b(256, 0);

  const int size = bmp->cw * bmp->ch;

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      const int r = rgba.r;
      const int g = rgba.g;
      const int b = rgba.b;

      list_r[r]++;
      list_g[g]++;
      list_b[b]++;
      p++;
    }
  }

  for(int j = 255; j >= 0; j--)
  {
    for(int i = 0; i < j; i++)
    {
      list_r[j] += list_r[i];
      list_g[j] += list_g[i];
      list_b[j] += list_b[i];
    }
  }

  const double scale = 255.0 / size;

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

      r = list_r[r] * scale;
      g = list_g[g] * scale;
      b = list_b[b] * scale;

      *p = makeRgba(r, g, b, rgba.a);
      p++;
    }

    if(Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void Equalize::begin()
{
  Project::undo->push();
  apply(Project::bmp);
}

