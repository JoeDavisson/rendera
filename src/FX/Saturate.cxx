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

#include "Saturate.H"

void Saturate::apply(Bitmap *bmp)
{
  std::vector<int> list_s(256, 0);

  const int size = bmp->cw * bmp->ch;

  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;
    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      const int r = rgba.r;
      const int g = rgba.g;
      const int b = rgba.b;

      int h, s, v;
      Blend::rgbToHsv(r, g, b, &h, &s, &v);

      list_s[s]++;
      p++;
    }
  }

  for (int j = 255; j >= 0; j--)
    for (int i = 0; i < j; i++)
      list_s[j] += list_s[i];

  const double scale = 255.0 / size;

  Progress::show(bmp->h);

  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      int r = rgba.r;
      int g = rgba.g;
      int b = rgba.b;

      const int l = getl(*p);
      int h, s, v;

      Blend::rgbToHsv(r, g, b, &h, &s, &v);

      // don't try to saturate grays
      if (s == 0)
      {
        p++;
        continue;
      }

      const int temp = s;
      s = list_s[s] * scale;

      if (s < temp)
        s = temp;

      Blend::hsvToRgb(h, s, v, &r, &g, &b);

      const int c1 = *p;
      *p = Blend::colorize(c1, Blend::keepLum(makeRgba(r, g, b, rgba.a), l), 255 - s);
      p++;
    }

    if (Progress::update(y) < 0)
      return;
  }

  Progress::hide();
}

void Saturate::begin()
{
  Project::undo->push();
  apply(Project::bmp);
}

