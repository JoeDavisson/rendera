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

#include "Quantize.H"
#include "Bitmap.H"
#include "Palette.H"
#include "Gui.H"
#include "Dialog.H"
#include "Widget.H"

#define MAX_COLORS 4096

extern int *fix_gamma;
extern int *unfix_gamma;

namespace
{
  // 18-bit color routines
  inline int make_rgb18(const int &r, const int &g, const int &b)
  {
    return r | g << 6 | b << 12;
  }

  inline int getr18(const int &c)
  {
    return c & 0x3F;
  }

  inline int getg18(const int &c)
  {
    return (c >> 6) & 0x3F;
  }

  inline int getb18(const int &c)
  {
    return (c >> 12) & 0x3F;
  }

  inline int diff18(const int &c1, const int &c2)
  {
    const int r = getr18(c1) - getr18(c2);
    const int g = getg18(c1) - getg18(c2);
    const int b = getb18(c1) - getb18(c2);

    return r * r + g * g + b * b;
  }

  inline int convert(const int &c1)
  {
    return make_rgb(getr18(c1) * 4.05, getg18(c1) * 4.05, getb18(c1) * 4.05);
  }

  // qsort callback to sort palette by luminance
  int comp_lum(const void *a, const void *b)
  {
    int c1 = *(int *)a;
    int c2 = *(int *)b;

    return getl(c1) - getl(c2);
  }

  // compute quantization error
  inline float error18(const int &c1, const int &c2,
                     const float &f1, const float &f2)
  {
    return ((f1 * f2) / (f1 + f2)) * diff18(c1, c2);
  }

  // merge two colors
  inline int merge18(const int &c1, const int &c2,
                     const float &f1, const float &f2)
  {
    const float mul = 1.0f / (f1 + f2);
    const int r = (f1 * getr18(c1) + f2 * getr18(c2)) * mul;
    const int g = (f1 * getg18(c1) + f2 * getg18(c2)) * mul;
    const int b = (f1 * getb18(c1) + f2 * getb18(c2)) * mul;

    return make_rgb18(r, g, b);
  }

  // 1D bilinear filter to stretch a palette
  void stretch_palette(int *data, int current, int target)
  {
    int *temp = new int[target];

    float ax = (float)(current - 1) / (float)(target - 1);

    int *c[2];
    c[0] = c[1] = &data[0];

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
        r += (float)fix_gamma[getr(*c[i])] * f[i];
        g += (float)fix_gamma[getg(*c[i])] * f[i];
        b += (float)fix_gamma[getb(*c[i])] * f[i];
        i++;
      }
      while(i < 2);

      r = unfix_gamma[(int)r];
      g = unfix_gamma[(int)g];
      b = unfix_gamma[(int)b];

      temp[x] = make_rgb((int)r, (int)g, (int)b);

      c[0] -= u1;
      c[1] -= u2;
      x++;
    }
    while(x < target);

    for(x = 0; x < target; x++)
      data[x] = temp[x];

    delete[] temp;
  }

  // limit colors being clustered to a reasonable number (4096)
  int limit_colors(float *list, int *colors)
  {
    int r, g, b;
    int i, j, k;
    int step = 4;
    int count = 0;

    for(b = 0; b <= 64 - step; b += step)
    {
      for(g = 0; g <= 64 - step; g += step)
      {
        for(r = 0; r <= 64 - step; r += step)
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
                int c = make_rgb18(r + i, g + j, b + k);
                float d = list[c];

                rr += d * (r + i);
                gg += d * (g + j);
                bb += d * (b + k);
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
            colors[count] = make_rgb18((int)rr, (int)gg, (int)bb);
            list[colors[count]] = div;
            count++;
          }
        }
      }
    }

    return count;
  }
}
// pairwise clustering algorithm
void Quantize::pca(Bitmap *src, int size)
{
  // popularity histogram
  float *list = new float[262144];

  // color list
  int *colors = new int[MAX_COLORS];

  // quantization error matrix
  int i, j;
  float *err_data = new float[MAX_COLORS * MAX_COLORS];
  float **err_row = new float *[MAX_COLORS];
  for(i = 0; i < MAX_COLORS; i++)
    err_row[i] = &err_data[MAX_COLORS * i];

  int max;
  int rep = size;
  int overscroll = src->overscroll;

  // reset lists
  for(i = 0; i < 262144; i++)
   list[i] = 0;

  for(i = 0; i < MAX_COLORS; i++)
    colors[i] = -1;

  // build histogram
  float inc = 1.0 / ((src->w - overscroll * 2) * (src->h - overscroll * 2));
  int count = 0;

  int r_high = 0;
  int g_high = 0;
  int b_high = 0;
  int r_low = 0xFFFFFF;
  int g_low = 0xFFFFFF;
  int b_low = 0xFFFFFF;

  for(j = overscroll; j < src->h - overscroll; j++)
  {
    int *p = src->row[j] + overscroll;
    for(i = overscroll; i < src->w - overscroll; i++)
    {
      // reduce to 18-bit equivalent
      const struct rgba_t rgba = get_rgba(*p++);

      if(rgba.r < r_low)
        r_low = rgba.r;
      if(rgba.r > r_high)
        r_high = rgba.r;
      if(rgba.g < g_low)
        g_low = rgba.g;
      if(rgba.g > g_high)
        g_high = rgba.g;
      if(rgba.b < b_low)
        b_low = rgba.b;
      if(rgba.b > b_high)
        b_high = rgba.b;

      const int c = make_rgb18(rgba.r / 4.04, rgba.g / 4.04, rgba.b / 4.04);

      if(list[c] < inc)
        count++;

      list[c] += inc;
    }
  }

  // if trying to make 1-color palette
  if(count < 2)
  {
    count = 2;
    colors[0] = make_rgb18(0, 0, 0);
    colors[1] = make_rgb18(0x3F, 0x3F, 0x3F);
    list[colors[0]] = inc;
    list[colors[1]] = inc;
  }
  else
  {
    // preserve darkest and lightest colors
    list[make_rgb18(r_low / 4.04, g_low / 4.04, b_low / 4.04)] = 100.0f;
    list[make_rgb18(r_high / 4.04, g_high / 4.04, b_high / 4.04)] = 100.0f;
  }

  // skip if already enough colors
  if(count <= rep)
  {
    count = 0;
    for(i = 0; i < 262144; i++)
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
      *(err_row[j] + i) = error18(colors[i], colors[j],
                                  list[colors[i]], list[colors[j]]);
    }
  }

  Dialog::showProgress(count - rep);

  while(count > rep)
  {
    // find lowest value in error matrix
    int ii = 0, jj = 0;
    float error = 999999;

    for(j = 0; j < max; j++)
    {
      if(colors[j] >= 0)
      {
        float *pos = err_row[j];

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

    colors[ii] = merge18(colors[ii], colors[jj],
                         list[colors[ii]], list[colors[jj]]);
    list[colors[ii]] = temp + list[colors[jj]];
    colors[jj] = -1;
    count--;

    // recompute error matrix for new row
    float *pos = err_row[ii] + ii;

    for(j = ii; j < max; j++)
    {
      if(colors[j] >= 0)
      {
        *pos = error18(colors[ii], colors[j],
                       list[colors[ii]], list[colors[j]]);
      }
      pos += MAX_COLORS;
    }

    // user cancelled operation
    if(Fl::get_key(FL_Escape))
    {
      delete[] err_row;
      delete[] err_data;
      delete[] colors;
      delete[] list;
      return;
    }

    Dialog::updateProgress();
  }

  Dialog::hideProgress();

  // build palette
  int index = 0;

  for(i = 0; i < max; i++)
  {
    if(colors[i] != -1)
    {
      Palette::main->data[index] = convert(colors[i]);
      index++;
    }
  }

  Palette::main->max = index;

  // sort palette
  qsort(Palette::main->data, Palette::main->max, sizeof(int), comp_lum);

  // stretch palette
  if(Palette::main->max != size)
  {
    stretch_palette(Palette::main->data, Palette::main->max, size);
    Palette::main->max = size;
  }

  // free memory
  delete[] err_row;
  delete[] err_data;
  delete[] colors;
  delete[] list;

  // redraw palette widget
  Gui::drawPalette();
  Palette::main->fillLookup();
}

