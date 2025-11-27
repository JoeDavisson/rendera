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

This averages the input colors down to improve efficiency.
*/

#include <vector>
#include <algorithm>

#include "Blend.H"
#include "Bitmap.H"
#include "Editor.H"
#include "Gui.H"
#include "Inline.H"
#include "Palette.H"
#include "Progress.H"
#include "Quantize.H"

#include "Project.H"
#include "ImagesOptions.H"
#include "Undo.H"

namespace
{
  const int colors_max = 3072;
  char str[256];
}

void Quantize::makeColor(color_type *c,
                         const double r, const double g, const double b,
                         const double freq)
{
  c->r = r;
  c->g = g;
  c->b = b;
  c->freq = freq;
}

double Quantize::error(const color_type *c1, const color_type *c2)
{
  const double r = c1->r - c2->r;
  const double g = c1->g - c2->g;
  const double b = c1->b - c2->b;
  const double f1 = c1->freq;
  const double f2 = c2->freq;

  return ((f1 * f2) / (f1 + f2)) * (r * r + g * g + b * b);
}

void Quantize::merge(color_type *c1, color_type *c2)
{
  const double f1 = c1->freq;
  const double f2 = c2->freq;
  const double div = f1 + f2;
  const double mul = 1.0 / div;

  c1->r = (f1 * c1->r + f2 * c2->r) * mul;
  c1->g = (f1 * c1->g + f2 * c2->g) * mul;
  c1->b = (f1 * c1->b + f2 * c2->b) * mul;
  c1->freq = div;
}

int Quantize::limitColors(double *histogram, color_type *colors,
                          gamut_type *g, int pal_size)
{
  const int diff_x = (double)g->high_x - g->low_x;
  const int diff_y = (double)g->high_y - g->low_y;
  const int diff_z = (double)g->high_z - g->low_z;

  double root = std::cbrt(colors_max);
  double div_x = root;
  double div_y = root * 2;
  double div_z = root / 2;

  int last_count = 0;
  int count = 0;

  std::vector<double> temp_hist(16777216, 0);
  std::vector<color_type> temp_colors(colors_max);

  while (true)
  {
    for (int i = 0; i < 16777216; i++)
      temp_hist[i] = histogram[i];

    for (int i = 0; i < colors_max; i++)
    {
      temp_colors[i].r = 0;
      temp_colors[i].g = 0;
      temp_colors[i].b = 0;
      temp_colors[i].freq = 0;
    }

    double step_x = diff_x / div_x;
    double step_y = diff_y / div_y;
    double step_z = diff_z / div_z;

    count = 0;
    int last_z = g->low_z;

    for (double z = g->low_z; z < g->high_z - step_z; z += step_z)
    {
      int size_z = (int)(z + step_z) - last_z;
      int last_y = g->low_y;

      for (double y = g->low_y; y < g->high_y - step_y; y += step_y)
      {
        int size_y = (int)(y + step_y) - last_y;
        int last_x = g->low_x;

        for (double x = g->low_x; x < g->high_x - step_x; x += step_x)
        {
          int size_x = (int)(x + step_x) - last_x;
          double rr = 0;
          double gg = 0;
          double bb = 0;
          double freq = 0;

          for (int k = 0; k < size_z; k++)
          {
            const int zk = z + k;

            for (int j = 0; j < size_y; j++)
            {
              const int yj = y + j;

              for (int i = 0; i < size_x; i++)
              {
                const int xi = x + i;

                const int r = xi;
                const int g = yj;
                const int b = zk;
 
                if (r < 256 && g < 256 && b < 256)
                {
                  const double d = temp_hist[makeRgb24(r, g, b)];

                  if (d > 0)
                  {
                    temp_hist[makeRgb24(r, g, b)] = 0;

                    rr += d * r;
                    gg += d * g;
                    bb += d * b;
                    freq += d;
                  }
                }

                last_x = x;
              }

              last_y = y;
            }

            last_z = z;
          }

          if (freq > 0)
          {
            rr /= freq;
            gg /= freq;
            bb /= freq;

            rr = clamp(rr, 255);
            gg = clamp(gg, 255);
            bb = clamp(bb, 255);

            makeColor(&temp_colors[count], rr, gg, bb, freq);
            count++;
          }

          if (count >= colors_max)
            break;
        }

        if (count >= colors_max)
          break;
      }

      if (count >= colors_max)
        break;
    }

    if (count >= colors_max)
      break;

    snprintf(str, sizeof(str), "Colors = %d/%d", count, colors_max);
    Gui::statusInfo(str);

    for (int i = 0; i < colors_max; i++)
    {
      colors[i].r = temp_colors[i].r;
      colors[i].g = temp_colors[i].g;
      colors[i].b = temp_colors[i].b;
      colors[i].freq = temp_colors[i].freq;
    }

    div_x *= 1.05;
    div_y *= 1.05;
    div_z *= 1.05;

    last_count = count;
  }

  return last_count;
}

void Quantize::pca(Bitmap *src, Palette *pal, int size)
{
  // popularity histogram
  Gui::saveStatusInfo();
  Gui::statusInfo("Calculating Histogram...");
  std::vector<double> histogram(16777216, 0);

  // range of RGB values in image
  gamut_type gamut;

  gamut.low_x = 255;
  gamut.low_y = 255;
  gamut.low_z = 255;
  gamut.high_x = -255;
  gamut.high_y = -255;
  gamut.high_z = -255;

  // build histogram
  const double weight = 1.0 / (src->cw * src->ch);
  int count = 0;

  for (int j = src->ct; j <= src->cb; j++)
  {
    for (int i = src->cl; i <= src->cr; i++)
    {
      rgba_type rgba = getRgba(src->getpixel(i, j));

      double freq = histogram[makeRgb24(rgba.r, rgba.g, rgba.b)];

      if (freq < weight)
        count++;

      histogram[makeRgb24(rgba.r, rgba.g, rgba.b)] = freq + weight;

      const double x = rgba.r;
      const double y = rgba.g;
      const double z = rgba.b;

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
  std::vector<color_type> colors(colors_max);

  // quantization error matrix
  std::vector<double> err_data(((colors_max + 1) * colors_max) / 2);

  // skip if already enough colors
  if (count <= size)
  {
    count = 0;

    for (int i = 0; i < 16777216; i++)
    {
      rgba_type rgba = getRgba(i);
      const double freq = histogram[makeRgb24(rgba.r, rgba.g, rgba.b)];

      if (freq > 0)
      {
        makeColor(&colors[count], rgba.r, rgba.g, rgba.b, freq);
        count++;
      }
    }
  }
    else
  {
    count = limitColors(histogram.data(), &colors[0], &gamut, size);
  }

  // set max
  int max = count;

  if (max < size)
    size = max;

  // init error matrix
  for (int j = 0; j < max; j++)
  {
    for (int i = 0; i < j; i++)
    {
      err_data[i + (j + 1) * j / 2] = error(&colors[i], &colors[j]);
    }
  }

  // show progress bar
  Progress::show(count - size);
  Gui::statusInfo("Merging...");

  // measure offset between array elements
  const int step = &(colors[1].freq) - &(colors[0].freq);

  // find minimum quantization error in matrix
  while (count > size)
  {
    int ii = 0, jj = 0;
    double least_err = 999999999;
    double *a = &(colors[0].freq);

    // find lowest value in error matrix
    for (int j = 0; j < max; j++)
    {
      if (*a > 0)
      {
        double *e = &err_data[(j + 1) * j / 2];
        double *b = &(colors[0].freq);

        for (int i = 0; i < j; i++)
        {
          if (*b > 0 && *e < least_err)
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
      {
        err_data[ii + (j + 1) * j / 2] = error(&colors[ii], &colors[j]);
      }
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
  Gui::restoreStatusInfo();
}

