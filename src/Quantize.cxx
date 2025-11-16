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

/*
Modified pairwise clustering quantization, adapted from the algorithm
described here:

http://www.visgraf.impa.br/Projects/quantization/quant.html
http://www.visgraf.impa.br/sibgrapi97/anais/pdf/art61.pdf

To save time/memory, colors are initially "posterized" to a maximum of 262144,
then further reduced to a maximum of 4096 by averaging sections of the color
cube. The perceptual influence of RGB values are taken into account to improve
results.
*/

#include <vector>
#include <algorithm>

#include "Blend.H"
#include "Bitmap.H"
#include "Editor.H"
#include "Gui.H"
#include "Inline.H"
#include "Octree.H"
#include "Palette.H"
#include "Progress.H"
#include "Quantize.H"

void Quantize::makeColor(color_type *c,
                         const float r, const float g, const float b,
                         const float freq)
{
  c->r = r;
  c->g = g;
  c->b = b;
  c->freq = freq;
}

float Quantize::error(const color_type *c1, const color_type *c2)
{
  const float r = c1->r - c2->r;
  const float g = c1->g - c2->g;
  const float b = c1->b - c2->b;
  const float f1 = c1->freq;
  const float f2 = c2->freq;

  return ((f1 * f2) / (f1 + f2)) * (r * r + g * g + b * b);
}

void Quantize::merge(color_type *c1, color_type *c2)
{
  const float f1 = c1->freq;
  const float f2 = c2->freq;
  const float div = f1 + f2;
  const float mul = 1.0 / div;

  c1->r = (f1 * c1->r + f2 * c2->r) * mul;
  c1->g = (f1 * c1->g + f2 * c2->g) * mul;
  c1->b = (f1 * c1->b + f2 * c2->b) * mul;
  c1->freq = div;
}

int Quantize::limitColors(Octree *histogram, color_type *colors,
                          gamut_type *g, int pal_size)
{
  int count = 0;

  float step_x = (g->high_x - g->low_x) / 16 + 1;
  float step_y = (g->high_y - g->low_y) / 32 + 1;
  float step_z = (g->high_z - g->low_z) / 8 + 1;

  if (step_x < 1)
    step_x = 1;

  if (step_y < 1)
    step_y = 1;

  if (step_z < 1)
    step_z = 1;

  for (float z = g->low_z; z <= g->high_z; z += step_z)
  {
    for (float y = g->low_y; y <= g->high_y; y += step_y)
    {
      for (float x = g->low_x; x <= g->high_x; x += step_x)
      {
        float rr = 0;
        float gg = 0;
        float bb = 0;
        float div = 0;

        for (int k = 0; k < step_z; k++)
        {
          const int zk = z + k;

          for (int j = 0; j < step_y; j++)
          {
            const int yj = y + j;

            for (int i = 0; i < step_x; i++)
            {
              const int xi = x + i;

              const int r = xi;
              const int g = yj;
              const int b = zk;
 
              if (r < 256 && g < 256 && b < 256)
              {
                const float d = histogram->read(r, g, b);

                if (d > 0)
                {
                  histogram->write(r, g, b, 0);

                  rr += d * r;
                  gg += d * g;
                  bb += d * b;
                  div += d;
                }
              }
            }
          }
        }

        if (div > 0)
        {
          rr /= div;
          gg /= div;
          bb /= div;

          rr = clamp(rr, 255);
          gg = clamp(gg, 255);
          bb = clamp(bb, 255);

          makeColor(&colors[count], rr, gg, bb, div);
          count++;
        }
      }
    }
  }

  //printf("count = %d\n", count);
  return count;
}

void Quantize::pca(Bitmap *src, Palette *pal, int size)
{
  // popularity histogram
  Octree histogram;

  // range of RGB values in image
  gamut_type gamut;

  gamut.low_x = 255;
  gamut.low_y = 255;
  gamut.low_z = 255;
  gamut.high_x = -255;
  gamut.high_y = -255;
  gamut.high_z = -255;

  // build histogram
  float weight = 1.0 / (src->cw * src->ch);
  int count = 0;

  for (int j = src->ct; j <= src->cb; j++)
  {
    for (int i = src->cl; i <= src->cr; i++)
    {
      rgba_type rgba = getRgba(src->getpixel(i, j));

      const int step_r = 255 / 63;
      const int step_g = 255 / 127;
      const int step_b = 255 / 31;

      rgba.r = (rgba.r / step_r) * step_r;
      rgba.g = (rgba.g / step_g) * step_g;
      rgba.b = (rgba.b / step_b) * step_b;

      float freq = histogram.read(rgba.r, rgba.g, rgba.b);

      if (freq < weight)
        count++;

      histogram.write(rgba.r, rgba.g, rgba.b, freq + weight);

      const float x = rgba.r;
      const float y = rgba.g;
      const float z = rgba.b;

      if (x < gamut.low_x)
        gamut.low_x = x;

      if (y < gamut.low_y)
        gamut.low_y = y;

      if (z < gamut.low_z)
        gamut.low_z = z;

      if (x > gamut.high_x)
        gamut.high_x = x; 

      if (y > gamut.high_y)
        gamut.high_y = y; 

      if (z > gamut.high_z)
        gamut.high_z = z; 
    }
  }

  // color list
  const int colors_max = 4096;
  std::vector<color_type> colors(colors_max);

  // quantization error matrix
  std::vector<float> err_data(((colors_max + 1) * colors_max) / 2);

  // skip if already enough colors
  if (count <= size)
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
    count = limitColors(&histogram, &colors[0], &gamut, size);
  }

  // set max
  int max = count;

  if (max < size)
    size = max;

  // init error matrix
  for (int j = 0; j < max; j++)
  {
    for (int i = 0; i < j; i++)
      err_data[i + (j + 1) * j / 2] = error(&colors[i], &colors[j]);
  }

  // show progress bar
  Progress::show(count - size);

  // measure offset between array elements
  const int step = &(colors[1].freq) - &(colors[0].freq);

  // find minimum quantization error in matrix
  while (count > size)
  {
    int ii = 0, jj = 0;
    float least_err = 999999999;
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

    // compute quantization level and replace i, delete j
    merge(&colors[ii], &colors[jj]);
    colors[jj].freq = 0;
    count--;

    // recompute error matrix for new row
    for (int j = ii; j < max; j++)
    {
      if (colors[j].freq > 0)
        err_data[ii + (j + 1) * j / 2] = error(&colors[ii], &colors[j]);
    }

    // user cancelled operation
    if (Fl::get_key(FL_Escape))
    {
      Progress::hide();
      return;
    }

    Progress::update(count);
  }

  Progress::hide();
  Editor::push();

  // build palette
  int index = 0;

  for (int i = 0; i < max; i++)
  {
    if (colors[i].freq > 0)
    {
      pal->data[index] = makeRgb(colors[i].r, colors[i].g, colors[i].b);
      index++;
    }
  }

  pal->max = index;
}

