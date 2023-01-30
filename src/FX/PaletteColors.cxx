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

#include "PaletteColors.H"

void PaletteColors::apply(Bitmap *bmp, Palette *pal)
{
  Gui::progressShow(bmp->h);

  const float inc = 1.0 / (bmp->cw * bmp->ch);
  float freq[256];

  for(int i = 0; i < pal->max; i++)
    freq[i] = 0;

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      rgba_type rgba = getRgba(*p);

      rgba.r = (rgba.r >> 3);
      rgba.g = (rgba.g >> 3);
      rgba.b = (rgba.b >> 3);

      rgba.r = (rgba.r << 3) + rgba.r;
      rgba.g = (rgba.g << 3) + rgba.g;
      rgba.b = (rgba.b << 3) + rgba.b;

      int c = makeRgb(rgba.r, rgba.g, rgba.b);

      freq[pal->lookup(c)] += inc;
      p++;
    }
  }

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int lowest = 999999;
      int use1 = 0;
      int use2 = 0;

      for(int i = 0; i < pal->max; i++)
      {
	int d = diff24(*p, pal->data[i]);

	if(d < lowest)
	{
	  lowest = d;
	  use1 = i;
	}
      }

      lowest = 999999;

      for(int i = 0; i < pal->max; i++)
      {
	if(i == use1)
	  continue;

	int d = diff24(*p, pal->data[i]);

	if(d < lowest)
	{
	  lowest = d;
	  use2 = i;
	}
      }

      int c1 = pal->data[use1];
      int c2 = pal->data[use2];

      rgba_type rgba1 = getRgba(c1);
      rgba_type rgba2 = getRgba(c2);

      const float mul = 1.0f / (freq[use1] + freq[use2]);

      rgba1.r = (freq[use1] * rgba1.r + freq[use2] * rgba2.r) * mul; 
      rgba1.g = (freq[use1] * rgba1.g + freq[use2] * rgba2.g) * mul; 
      rgba1.b = (freq[use1] * rgba1.b + freq[use2] * rgba2.b) * mul; 

      int l = getl(*p);

      *p = Blend::keepLum(makeRgb(rgba1.r, rgba1.g, rgba1.b), l);
      p++;
    }

    if(Gui::progressUpdate(y) < 0)
      return;
  }

  Gui::progressHide();
}

void PaletteColors::begin()
{
  Project::undo->push();
  apply(Project::bmp, Project::palette);
}

