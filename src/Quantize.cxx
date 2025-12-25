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

const int colors_max = 3000;

bool Quantize::sort_greater_cb(const color_type &a, const color_type &b)
{
  return a.freq > b.freq;
}

void Quantize::makeColor(color_type &c,
                         const double r, const double g, const double b,
                         const double freq)
{
  c.r = r;
  c.g = g;
  c.b = b;
  c.freq = freq;
}

double Quantize::error(const color_type &c1, const color_type &c2)
{
  const double r = c1.r - c2.r;
  const double g = c1.g - c2.g;
  const double b = c1.b - c2.b;
  const double f1 = c1.freq;
  const double f2 = c2.freq;

  return ((f1 * f2) / (f1 + f2)) * (r * r + g * g + b * b);
}

void Quantize::merge(color_type &c1, color_type &c2)
{
  const double f1 = c1.freq;
  const double f2 = c2.freq;
  const double sum = f1 + f2;

  c1.r = (f1 * c1.r + f2 * c2.r) / sum;
  c1.g = (f1 * c1.g + f2 * c2.g) / sum;
  c1.b = (f1 * c1.b + f2 * c2.b) / sum;
  c1.freq = sum;
}

int Quantize::limitColors(const std::vector<double> &histogram,
                          std::vector<color_type> &colors)
{
  int temp_count = 0;
  std::vector<color_type> temp_colors(16777216);

  // build color list
  for (int i = 0; i < 16777216; i++)
  {
    temp_colors[i].r = 0;
    temp_colors[i].g = 0;
    temp_colors[i].b = 0;
    temp_colors[i].freq = 0;
  }

  for (int z = 0; z < 256; z++)
  {
    for (int y = 0; y < 256; y++)
    {
      for (int x = 0; x < 256; x++)
      {
        const double freq = histogram[makeRgb24(x, y, z)];

        if (freq > 0)
        {
          makeColor(temp_colors[temp_count], x, y, z, freq);
          temp_count++;
        }
      }
    }
  }

  // sort by popularity
  std::sort(temp_colors.begin(), temp_colors.end(), sort_greater_cb);

  // choose a diverse range of popularities
  int count = 0;
  double step = (double)temp_count / colors_max;

  if (step < 1.0)
    step = 1.0;

  for (double i = 0; i < temp_count; i += step)
  {
    colors[count].r = temp_colors[i].r;
    colors[count].g = temp_colors[i].g;
    colors[count].b = temp_colors[i].b;
    colors[count].freq = temp_colors[i].freq;
    count++;

    if (count >= colors_max)
      break;
  }

  //printf("count = %d\n", count);
  return count;
}

void Quantize::pca(Bitmap *src, Palette *pal, int size)
{
  // popularity histogram
  Gui::saveStatusInfo();
  Gui::statusInfo("Creating Color List...");
  std::vector<double> histogram(16777216, 0);

  // build histogram
  const double weight = 1.0 / (src->cw * src->ch);
  int count = 0;

  for (int j = src->ct; j <= src->cb; j++)
  {
    for (int i = src->cl; i <= src->cr; i++)
    {
      const int c = src->getpixel(i, j);
      rgba_type rgba = getRgba(c);

      double freq = histogram[makeRgb24(rgba.r, rgba.g, rgba.b)];

      if (freq < weight)
        count++;

      histogram[makeRgb24(rgba.r, rgba.g, rgba.b)] = freq + weight;
    }
  }

  // color list
  std::vector<color_type> colors(colors_max);

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
        makeColor(colors[count], rgba.r, rgba.g, rgba.b, freq);
        count++;
      }
    }
  }
    else
  {
    count = limitColors(histogram, colors);
  }

  // set max
  int max = count;

  if (max < size)
    size = max;

  // init error matrix
  std::vector<double> err_data(((max + 1) * max) / 2);

  for (int j = 0; j < max; j++)
  {
    for (int i = 0; i < j; i++)
    {
      err_data[i + (j + 1) * j / 2] = error(colors[i], colors[j]);
    }
  }

  // show progress bar
  Progress::show(max - size);
  Gui::statusInfo("Merging...");

  // measure offset between array elements
  const int freq_step = &(colors[1].freq) - &(colors[0].freq);

  // find minimum quantization error in matrix
  while (count > size)
  {
    int ii = 0, jj = 0;
    double least_err = 99999;
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
          if (*e < least_err && *b > 0)
          {
            least_err = *e;
            ii = i;
            jj = j;
          }

          e++;
          b += freq_step;
        }
      }

      a += freq_step;
    }

    // compute quantization level and replace i, delete j
    merge(colors[ii], colors[jj]);
    colors[jj].freq = 0;
    count--;

    // recompute error matrix
    for (int j = ii; j < max; j++)
    {
      if (colors[j].freq > 0)
      {
        err_data[ii + (j + 1) * j / 2] = error(colors[ii], colors[j]);
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

