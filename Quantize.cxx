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

#include <cmath>

#include "Quantize.H"
#include "Bitmap.H"
#include "Blend.H"
#include "Palette.H"
#include "Gui.H"
#include "Dialog.H"
#include "Widget.H"
#include "Octree.H"

#define MAX_COLORS 4096

namespace
{
  // color struct, stores RGB values as floats for increased clustering
  // accuracy, also stores the frequency of the color in the image
  struct color_t
  {
    float r, g, b;
    float freq;
    int active;
  };

  // create a color_t structure
  inline void make_color(struct color_t *c,
                         float r, float g, float b, float freq)
  {
    c->r = r;
    c->g = g;
    c->b = b;
    c->freq = freq;
    c->active = 1;
  }

  // qsort callback to sort palette
  int comp_lum(const void *a, const void *b)
  {
    int c1 = *(int *)a;
    int c2 = *(int *)b;

    return getl(c1) - getl(c2);
  }

  // compute quantization error
  inline float error(struct color_t *c1, struct color_t *c2)
  {
    const float r = c1->r - c2->r;
    const float g = c1->g - c2->g;
    const float b = c1->b - c2->b;

    return ((c1->freq * c2->freq) / (c1->freq + c2->freq)) *
            (r * r + g * g + b * b);
  }

  // merge two colors
  inline void merge(struct color_t *c1, struct color_t *c2)
  {
    const float mul = 1.0f / (c1->freq + c2->freq);
    c1->r = (c1->freq * c1->r + c2->freq * c2->r) * mul;
    c1->g = (c1->freq * c1->g + c2->freq * c2->g) * mul;
    c1->b = (c1->freq * c1->b + c2->freq * c2->b) * mul;

    c1->freq += c2->freq;  
  }

  // averages the color cube down to a maximum of 4096 colors
  int limit_colors(Octree *histogram, color_t *colors)
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
                float d = histogram->read(r + i, g + j, b + k);

                if(d > 0)
                  histogram->write(r + i, g + j, b + k, 0);

                rr += d * (r + i);
                gg += d * (g + j);
                bb += d * (b + k);
                div += d;
              }
            }
          }

          if(div > 0)
          {
            rr /= div;
            gg /= div;
            bb /= div;
            make_color(&colors[count], rr, gg, bb, div);
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
  int i, j;

  // popularity histogram
  Octree *histogram = new Octree();

  // color list
  struct color_t *colors = new color_t[MAX_COLORS];

  for(i = 0; i < MAX_COLORS; i++)
    colors[i].active = 0;

  // quantization error matrix
  float *err_data = new float[((MAX_COLORS + 1) * MAX_COLORS) / 2];

  int max;
  int rep = size;
  int overscroll = src->overscroll;

  // build histogram, inc is the weight of 1 pixel in the image
  float inc = 1.0 / ((src->w - overscroll * 2) * (src->h - overscroll * 2));
  int count = 0;

  for(j = overscroll; j < src->h - overscroll; j++)
  {
    int *p = src->row[j] + overscroll;

    for(i = overscroll; i < src->w - overscroll; i++)
    {
      struct rgba_t rgba = get_rgba(*p++);
      float freq = histogram->read(rgba.r, rgba.g, rgba.b);

      if(freq < inc)
        count++;

      histogram->write(rgba.r, rgba.g, rgba.b, freq + inc);
    }
  }

  // if trying to make 1-color palette
  if(count < 2)
  {
    count = 2;
    make_color(&colors[0], 0, 0, 0, inc);
    make_color(&colors[1], 255, 255, 255, inc);
  }

  // skip if already enough colors
  if(count <= rep)
  {
    count = 0;

    for(i = 0; i < 16777216; i++)
    {
      struct rgba_t rgba = get_rgba(i);
      float freq = histogram->read(rgba.r, rgba.g, rgba.b);

      if(freq > 0)
      {
        make_color(&colors[count], rgba.r, rgba.g, rgba.b, freq);
        count++;
      }
    }
  }
  else
  {
    // limit color count to 4096
    count = limit_colors(histogram, colors);
  }

  // don't need histogram anymore
  delete histogram;

  // set max
  max = count;
  if(max < rep)
    rep = max;

  // init error matrix
  for(j = 0; j < max; j++)
  {
    for(i = 0; i < j; i++)
      err_data[i + (j + 1) * j / 2] = error(&colors[i], &colors[j]);
  }

  Dialog::showProgress(count - rep);

  while(count > rep)
  {
    // find lowest value in error matrix
    int ii = 0, jj = 0;
    float least_err = 999999;

    for(j = 0; j < max; j++)
    {
      if(colors[j].active)
      {
        for(i = 0; i < j; i++)
        {
          float e = err_data[i + (j + 1) * j / 2];

          if(colors[i].active && err_data[i + (j + 1) * j / 2] < least_err)
          {
            least_err = e;
            ii = i;
            jj = j;
          }
        }
      }
    }

    // compute quantization level and place in i, delete j
    float temp = colors[ii].freq;

    merge(&colors[ii], &colors[jj]);
    colors[jj].active = 0;
    count--;

    // recompute error matrix for new row
    for(j = ii; j < max; j++)
    {
      if(colors[j].active)
        err_data[ii + (j + 1) * j / 2] = error(&colors[ii], &colors[j]);
    }

    // user cancelled operation
    if(Fl::get_key(FL_Escape))
    {
      delete[] err_data;
      delete[] colors;
      return;
    }

    Dialog::updateProgress();
  }

  Dialog::hideProgress();

  // build palette
  int index = 0;

  for(i = 0; i < max; i++)
  {
    if(colors[i].active)
    {
      Palette::main->data[index] =
        make_rgb((int)colors[i].r, (int)colors[i].g, (int)colors[i].b);
      index++;
    }
  }

  Palette::main->max = index;

  // sort palette
  qsort(Palette::main->data, Palette::main->max, sizeof(int), comp_lum);

  // free memory
  delete[] err_data;
  delete[] colors;

  // redraw palette widget
  Gui::drawPalette();
  Palette::main->fillTable();
}

