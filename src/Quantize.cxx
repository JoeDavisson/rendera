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

#include <vector>
#include <algorithm>

#include "Blend.H"
#include "Bitmap.H"
#include "Dialog.H"
#include "Gui.H"
#include "Inline.H"
#include "Octree.H"
#include "Palette.H"
#include "Project.H"
#include "Quantize.H"
#include "View.H"
#include "Widget.H"

void Quantize::makeColor(color_type *c,
                         const float r, const float g, const float b,
                         const float freq)
{
  c->r = r;
  c->g = g;
  c->b = b;
  c->freq = freq;
}

// compute quantization error
float Quantize::error(color_type *c1, color_type *c2)
{
  const float r = c1->r - c2->r;
  const float g = c1->g - c2->g;
  const float b = c1->b - c2->b;

  return ((c1->freq * c2->freq) / (c1->freq + c2->freq)) *
          (r * r + g * g + b * b);
}

// merge two colors
void Quantize::merge(color_type *c1, color_type *c2)
{
  const float div = c1->freq + c2->freq;

  c1->r = (c1->freq * c1->r + c2->freq * c2->r) / div;
  c1->g = (c1->freq * c1->g + c2->freq * c2->g) / div;
  c1->b = (c1->freq * c1->b + c2->freq * c2->b) / div;
  c1->freq += c2->freq;  
}

// reduces color count by averaging sections of the color cube
int Quantize::limitColors(Octree *histogram, color_type *colors, const int step)
{
  int count = 0;

  for (int b = 0; b <= 256 - step; b += step)
  {
    for (int g = 0; g <= 256 - step; g += step)
    {
      for (int r = 0; r <= 256 - step; r += step)
      {
        float rr = 0;
        float gg = 0;
        float bb = 0;
        float div = 0;

        for (int k = 0; k < step; k++)
        {
          const int bk = b + k;

          for (int j = 0; j < step; j++)
          {
            const int gj = g + j;

            for (int i = 0; i < step; i++)
            {
              const int ri = r + i;
              const float d = histogram->read(ri, gj, bk);

              if (d > 0)
                histogram->write(ri, gj, bk, 0);

              rr += d * ri;
              gg += d * gj;
              bb += d * bk;
              div += d;
            }
          }
        }

        if (div > 0)
        {
          rr /= div;
          gg /= div;
          bb /= div;

          makeColor(&colors[count], rr, gg, bb, div);
          count++;
        }
      }
    }
  }

  return count;
}

// Pairwise clustering quantization, adapted from the algorithm described here:
//
// http://www.visgraf.impa.br/Projects/quantization/quant.html
// http://www.visgraf.impa.br/sibgrapi97/anais/pdf/art61.pdf
//
void Quantize::pca(Bitmap *src, Palette *pal, const int size)
{
  // popularity histogram
  Octree histogram;

  int max;
  int rep = size;

  // build histogram, inc is the weight of 1 pixel in the image
  float inc = 1.0 / (src->cw * src->ch);
  int count = 0;

  for (int j = src->ct; j <= src->cb; j++)
  {
    int *p = src->row[j] + src->cl;

    for (int i = src->cl; i <= src->cr; i++)
    {
      rgba_type rgba = getRgba(*p++);
      float freq = histogram.read(rgba.r, rgba.g, rgba.b);

      if (freq < inc)
        count++;

      histogram.write(rgba.r, rgba.g, rgba.b, freq + inc);
    }
  }

  // color list
  const int colors_max = 4096;
  std::vector<color_type> colors(colors_max);

  for (int i = 0; i < colors_max; i++)
    colors[i].freq = 0;

  // quantization error matrix
  std::vector<float> err_data(((colors_max + 1) * colors_max) / 2);

  // skip if already enough colors
  if (count <= rep)
  {
    count = 0;

    for (int i = 0; i < 16777216; i++)
    {
      rgba_type rgba = getRgba(i);
      const float freq = histogram.read(rgba.r, rgba.g, rgba.b);

      if (freq > 0)
      {
        makeColor(&colors[count], rgba.r, rgba.g, rgba.b, freq);
        count++;
      }
    }
  }
    else
  {
    count = limitColors(&histogram, &colors[0], 16);
  }

  // set max
  max = count;
  if (max < rep)
    rep = max;

  // init error matrix
  for (int j = 0; j < max; j++)
  {
    for (int i = 0; i < j; i++)
      err_data[i + (j + 1) * j / 2] = error(&colors[i], &colors[j]);
  }

  Gui::progressShow(count - rep);

  // measure offset between array elements
  const int step = &(colors[1].freq) - &(colors[0].freq);

  while (count > rep)
  {
    int ii = 0, jj = 0;
    float least_err = 999999;
    float *a = &(colors[0].freq);

    // find lowest value in error matrix
    for (int j = 0; j < max; j++)
    {
      if (*a > 0)
      {
        float *e = &err_data[(j + 1) * j / 2];
        float *b = &(colors[0].freq);

        for (int i = 0; i < j; i++)
        {
          if (*b > 0 && (*e < least_err))
          {
            least_err = *e;
            ii = i;
            jj = j;
          }

          e++;
          b += step;
        }
      }

      a += step;
    }

    // compute quantization level and place in i, delete j
    merge(&colors[ii], &colors[jj]);
    colors[jj].freq = false;
    count--;

    // recompute error matrix for new row
    for (int j = ii; j < max; j++)
    {
      if (colors[j].freq > 0)
        err_data[ii + (j + 1) * j / 2] = error(&colors[ii], &colors[j]);
    }

    // user cancelled operation
    if (Fl::get_key(FL_Escape))
      return;

    Gui::progressUpdate(count);
  }

  Gui::progressHide();

  // build palette
  int index = 0;

  for (int i = 0; i < max; i++)
  {
    if (colors[i].freq > 0)
    {
      pal->data[index] =
        makeRgb((int)colors[i].r, (int)colors[i].g, (int)colors[i].b);

      index++;
    }
  }

  pal->max = index;
}

