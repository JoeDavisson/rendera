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
  const int colors_max = 2048;
  char str[256];
}

bool Quantize::sort_greater_cb(const color_type &a, const color_type &b)
{
  return a.freq > b.freq;
}

bool Quantize::sort_less_cb(const color_type &a, const color_type &b)
{
  return a.freq < b.freq;
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
                          gamut_type *gmt, int pal_size)
{
  int root = 16;
  int div_x = root;
  int div_y = root * 2;
  int div_z = root / 2;
  
  int step_x = 256 / div_x;
  int step_y = 256 / div_y;
  int step_z = 256 / div_z;

  int count = 0;

  for (int z = 0; z <= 256 - step_z; z += step_z)
  {
    for (int y = 0; y <= 256 - step_y; y += step_y)
    {
      for (int x = 0; x <= 256 - step_x; x += step_x)
      {
        double rr = 0;
        double gg = 0;
        double bb = 0;
        double freq = 0;

        for (int k = 0; k < step_z; k++)
        {
          const int zk = z + k;

          for (int j = 0; j < step_y; j++)
          {
            const int yj = y + j;

            for (int i = 0; i < step_x; i++)
            {
              const int xi = x + i;

              int r = xi;
              int g = yj;
              int b = zk;

              if (r < 256 && g < 256 && b < 256)
              {
                const double d = histogram[makeRgb24(r, g, b)];

                if (d > 0)
                {
                  rr += (r * r) * d;
                  gg += (g * g) * d;
                  bb += (b * b) * d;

                  freq += d;
                }
              }
            }
          }
        }

        if (freq > 0)
        {
          rr /= freq;
          gg /= freq;
          bb /= freq;

          rr = std::sqrt(rr);
          gg = std::sqrt(gg);
          bb = std::sqrt(bb);

          rr = clamp(rr, 255);
          gg = clamp(gg, 255);
          bb = clamp(bb, 255);

          makeColor(&colors[count], rr, gg, bb, freq);
          count++;
        }
      }
    }

    snprintf(str, sizeof(str), "Colors = %d/%d", count, colors_max);
    Gui::statusInfo(str);
  }

  root = 64;
  div_x = root;
  div_y = root * 2;
  div_z = root / 2;
  
  step_x = 256 / div_x;
  step_y = 256 / div_y;
  step_z = 256 / div_z;

  std::vector<color_type> temp_colors(262144);
  int temp_count = 0;

  for (int i = 0; i < 262144; i++)
  {
    temp_colors[i].r = 0;
    temp_colors[i].g = 0;
    temp_colors[i].b = 0;
    temp_colors[i].freq = 0;
  }

  for (int z = 0; z <= 256 - step_z; z += step_z)
  {
    for (int y = 0; y <= 256 - step_y; y += step_y)
    {
      for (int x = 0; x <= 256 - step_x; x += step_x)
      {
        double rr = 0;
        double gg = 0;
        double bb = 0;
        double freq = 0;

        for (int k = 0; k < step_z; k++)
        {
          const int zk = z + k;

          for (int j = 0; j < step_y; j++)
          {
            const int yj = y + j;

            for (int i = 0; i < step_x; i++)
            {
              const int xi = x + i;

              int r = xi;
              int g = yj;
              int b = zk;

              if (r < 256 && g < 256 && b < 256)
              {
                const double d = histogram[makeRgb24(r, g, b)];

                if (d > 0)
                {
                  rr += (r * r) * d;
                  gg += (g * g) * d;
                  bb += (b * b) * d;

                  freq += d;
                }
              }
            }
          }
        }

        if (freq > 0)
        {
          rr /= freq;
          gg /= freq;
          bb /= freq;

          rr = std::sqrt(rr);
          gg = std::sqrt(gg);
          bb = std::sqrt(bb);

          rr = clamp(rr, 255);
          gg = clamp(gg, 255);
          bb = clamp(bb, 255);

          makeColor(&temp_colors[temp_count], rr, gg, bb, freq);
          temp_count++;
        }
      }
    }

    snprintf(str, sizeof(str), "Colors = %d/%d", count, colors_max);
    Gui::statusInfo(str);
  }

  std::sort(temp_colors.begin(), temp_colors.end(), sort_greater_cb);

  for (int i = 0; i < temp_count; i++)
  {
    if (temp_colors[i].freq == 0)
      continue;

    colors[count].r = temp_colors[i].r;
    colors[count].g = temp_colors[i].g;
    colors[count].b = temp_colors[i].b;
    colors[count].freq = temp_colors[i].freq;
    count++;

    if (count >= colors_max)
      break;
  }

  printf("count = %d\n", count);
  printf("temp_count = %d\n", temp_count);
  return count;
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
//  std::vector<double> err_data(((colors_max + 1) * colors_max) / 2);

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

  // quantization error matrix
  std::vector<double> err_data(((count + 1) * count) / 2);

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
      Gui::restoreStatusInfo();
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

