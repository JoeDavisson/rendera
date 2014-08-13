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

#define SHIFT 12

extern Gui *gui;
extern Dialog *dialog;

// qsort callback to sort palette by luminance
static int comp_lum(const void *a, const void *b)
{
  int c1 = *(int *)a;
  int c2 = *(int *)b;

  return getl(c1) - getl(c2);
}

// 1D bilinear filter to stretch a palette
// FIXME add gamma correction
static void stretch_palette(int current, int target)
{
  int *temp = new int[256];

  float ax = (float)(current - 1) / (float)(target - 1);

  int *c[2];
  c[0] = c[1] = &Palette::main->data[0];

  int x = 0;
  do
  {
    float uu = (x * ax);
    int u1 = uu;
    if(u1 > current - 1)
      u1 = current - 1;
    int u2 = (u1 < (current - 1) ? u1 + 1 : u1);
    float u = uu - u1;

    c[0] += u1;
    c[1] += u2;

    float f[2];

    f[0] = (1.0 - u);
    f[1] = u;

    float r = 0, g = 0, b = 0;
    int i = 0;
    do
    {
      r += (float)getr(*c[i]) * f[i];
      g += (float)getg(*c[i]) * f[i];
      b += (float)getb(*c[i]) * f[i];
      i++;
    }
    while(i < 2);

    temp[x] = makecol_notrans((int)r, (int)g, (int)b);

    c[0] -= u1;
    c[1] -= u2;
    x++;
  }
  while(x < target);

  for(x = 0; x < target; x++)
    Palette::main->data[x] = temp[x];

  Palette::main->max = target;

  delete[] temp;
}

// limit colors being clustered to a reasonable number
static int limit_colors(float *list, int *colors)
{
  int r, g, b;
  int i, j, k;
  int step = 16;
  int count = 0;

  for(b = 0; b <= 256 - step; b += step)
  {
    for(g = 0; g <= 256 - step; g += step)
    {
      for(r = 0; r <= 256 - step; r += step)
      {
        float rr = 0;
        float gg = 0;
        float bb = 0;
        float div = 0;

        for(k = 0; k < step; k++)
        {
          for(j = 0; j < step; j++)
          {
            for(i = 0; i < step; i++)
            {
              int c = makecol_notrans(r + i, g + j, b + k);
              float d = list[c];

              rr += d * getr(c);
              gg += d * getg(c);
              bb += d * getb(c);
              div += d;
              list[c] = 0;
            }
          }
        }

        if(div > 0)
        {
          rr /= div;
          gg /= div;
          bb /= div;
          colors[count] = makecol_notrans((int)rr, (int)gg, (int)bb);
          list[colors[count]] = div;
          count++;
        }
      }
    }
  }

  return count;
}

// compute quantization error
static inline float error24(const int c1, const int c2, const float f1,
                            const float f2)
{
  return ((f1 * f2) / (f1 + f2)) * diff24(c1, c2);
}

// merge two colors
static inline int merge24(const int c1, const int c2, const float f1,
                          const float f2)
{
//  const float div = 1.0f / (f1 + f2);

  const int r = (f1 * getr(c1) + f2 * getr(c2)) / (f1 + f2);
  const int g = (f1 * getg(c1) + f2 * getg(c2)) / (f1 + f2);
  const int b = (f1 * getb(c1) + f2 * getb(c2)) / (f1 + f2);
  //const int r = (f1 * getr(c1) + f2 * getr(c2)) * div;
  //const int g = (f1 * getg(c1) + f2 * getg(c2)) * div;
  //const int b = (f1 * getb(c1) + f2 * getb(c2)) * div;

  return makecol_notrans(r, g, b);
}

// pairwise-clustering algorithm
void quantize(Bitmap *src, int size)
{
  // popularity histogram
  float *list = new float[16777216];

  // color list
  int *colors = new int[1 << SHIFT];

  // quantization error matrix
  float *err = new float[(1 << SHIFT) * (1 << SHIFT)];

  int i, j;
  int max;
  int rep = size;
  int overscroll = src->overscroll;

  // reset lists
  for(i = 0; i < 16777216; i++)
    list[i] = 0;

  for(i = 0; i < (1 << SHIFT); i++)
    colors[i] = -1;

  // build histogram
  float inc = 1.0 / ((src->w - overscroll * 2) * (src->h - overscroll * 2));
  int count = 0;
  for(j = overscroll; j < src->h - overscroll; j++)
  {
    for(i = overscroll; i < src->w - overscroll; i++)
    {
      int c = src->getpixel(i, j);

      // reduce to 18-bit equivalent
      int r = (getr(c) >> 2) << 2;
      int g = (getg(c) >> 2) << 2;
      int b = (getb(c) >> 2) << 2;

      // correct lightest values
      if(r >= 252)
        r = 255;
      if(g >= 252)
        g = 255;
      if(b >= 252)
        b = 255;

      c = makecol_notrans(r, g, b);

      if(list[c] < inc)
        count++;

      list[c] += inc;
    }
  }

  // if trying to make 1-color palette
  if(count < 2)
  {
    count = 2;
    colors[0] = makecol_notrans(0, 0, 0);
    colors[1] = makecol_notrans(255, 255, 255);
    list[colors[0]] = inc;
    list[colors[1]] = inc;
  }

  // skip if already enough colors
  if(count <= rep)
  {
    count = 0;
    for(i = 0; i < 16777216; i++)
    {
      if(list[i] > 0)
      {
        colors[count] = i;
        count++;
      }
    }
  }
  else
  {
    // limit color count to 4096
    count = limit_colors(list, colors);
  }

  // set max
  max = count;
  if(max < rep)
    rep = max;

  // init error matrix
  for(j = 0; j < max; j++)
  {
    for(i = 0; i < j; i++)
    {
      err[i + (j << SHIFT)] = error24(colors[i], colors[j],
                                      list[colors[i]], list[colors[j]]);
    }
  }

  float value = 0;
  dialog->progress_bar->value(0);
  float step = 100.0 / (count - rep);
  show_progress();

  while(count > rep)
  {
    // find lowest value in error matrix
    int ii = 0, jj = 0;
    float error = 999999;

    for(j = 0; j < max; j++)
    {
      if(colors[j] >= 0)
      {
        float *pos = &err[(j << SHIFT)];

        for(i = 0; i < j; i++)
        {
          if((colors[i] >= 0) && (*pos < error))
          {
            error = *pos;
            ii = i;
            jj = j;
          }
          pos++;
        }
      }
    }

    // compute quantization level and place in i, delete j
    float temp = list[colors[ii]];

    colors[ii] = merge24(colors[ii], colors[jj],
                         list[colors[ii]], list[colors[jj]]);
    list[colors[ii]] = temp + list[colors[jj]];
    colors[jj] = -1;
    count--;

    // recompute error matrix for new row
    float *pos = &err[ii + (ii << SHIFT)];

    for(j = ii; j < max; j++)
    {
      if(colors[j] >= 0)
      {
        *pos = error24(colors[ii], colors[j],
                       list[colors[ii]], list[colors[j]]);
      }
      pos += (1 << SHIFT);
    }

    dialog->progress_bar->value(value);
    Fl::check();
    value += step;
  }

  hide_progress();

  // build palette
  int index = 0;

  for(i = 0; i < max; i++)
  {
    if(colors[i] != -1)
    {
      Palette::main->data[index] = colors[i];
      index++;
    }
  }

  Palette::main->max = index;

  // sort palette
  qsort(Palette::main->data, Palette::main->max, sizeof(int), comp_lum);

  // stretch palette
  if(Palette::main->max != size)
    stretch_palette(Palette::main->max, size);

  // free memory
  delete[] err;
  delete[] colors;
  delete[] list;

  // redraw palette widget
  Palette::main->draw(gui->palette);
  gui->palette->redraw();
  Palette::main->fill_lookup();
}

