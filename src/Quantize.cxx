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

int Quantize::limitColors(std::vector<color_type> &color_bin,
                          std::vector<color_type> &colors,
                          int samples, int size)
{
  // sort by popularity
  std::sort(color_bin.begin(), color_bin.end(), sort_greater_cb);

  // find last element
  int color_bin_count = 0;

  for (int i = 0; i < 32768; i++)
  {
    if (color_bin[i].freq == 0)
      break;

    color_bin_count++;
  }

  // choose a diverse range of popularities
  if (size < 16)
    size = 16;

  const double curve = (double)size / 256;
  const double r = std::pow(color_bin_count, 1.0 / (samples - 1));
  int count = 0;

  for (int i = 0; i < samples; i++)
  {
    const double index_lin = i * (double)(color_bin_count - 1) / (samples - 1);
    const double index_log = std::pow(r, (double)i) - 1.0;
    double index_lerp = index_lin + curve * (index_log - index_lin);

    if (index_lerp > samples - 1)
      index_lerp = samples - 1;

    colors[count].r = color_bin[index_lerp].r;
    colors[count].g = color_bin[index_lerp].g;
    colors[count].b = color_bin[index_lerp].b;
    colors[count].freq = color_bin[index_lerp].freq;
    count++;

    if (count >= samples)
      break;
  }

  printf("count = %d\n", count);
  return count;
}

void Quantize::pca(Bitmap *src, Palette *pal, int size, int samples)
{
  Gui::saveStatusInfo();
  Gui::statusInfo("Creating Color List...");

  // initial color list
  std::vector<color_type> color_bin(32768);

  for (int i = 0; i < 32768; i++)
  {
    color_bin[i].r = 0;
    color_bin[i].g = 0;
    color_bin[i].b = 0;
    color_bin[i].freq = 0;
  }

  const double weight = 1.0;
  int count = 0;

  for (int j = src->ct; j <= src->cb; j++)
  {
    for (int i = src->cl; i <= src->cr; i++)
    {
      const int c = src->getpixel(i, j);
      rgba_type rgba = getRgba(c);

      const int r15 = rgba.r >> 3;
      const int g15 = rgba.g >> 3;
      const int b15 = rgba.b >> 3;
      const int index = makeRgb15(r15, g15, b15);

      color_bin[index].r += rgba.r * rgba.r;
      color_bin[index].g += rgba.g * rgba.g;
      color_bin[index].b += rgba.b * rgba.b;
      color_bin[index].freq += weight;
    }
  }

  for (int b = 0; b < 256; b += 8)
  {
    for (int g = 0; g < 256; g += 8)
    {
      for (int r = 0; r < 256; r += 8)
      {
        const int r15 = r >> 3;
        const int g15 = g >> 3;
        const int b15 = b >> 3;

        const int index = makeRgb15(r15, g15, b15);
        const int freq = color_bin[index].freq;

        if (freq > 0)
        {
          const double r = color_bin[index].r;
          const double g = color_bin[index].g;
          const double b = color_bin[index].b;

          color_bin[index].r = std::sqrt(r / freq);
          color_bin[index].g = std::sqrt(g / freq);
          color_bin[index].b = std::sqrt(b / freq);

          count++;
        }
      }
    }
  }

  // reduced color list
  std::vector<color_type> colors(samples);

  // skip if already enough colors
  if (count <= size)
  {
    count = 0;

    for (int i = 0; i < 32768; i++)
    {
      const int r15 = i & 31;
      const int g15 = (i >> 5) & 31;
      const int b15 = (i >> 10) & 31;
      const int index = makeRgb15(r15, g15, b15);
      const double freq = color_bin[index].freq;

      if (freq > 0)
      {
        makeColor(colors[count],
                  color_bin[index].r,
                  color_bin[index].g,
                  color_bin[index].b,
                  freq);
        count++;
      }
    }
  }
    else
  {
    count = limitColors(color_bin, colors, samples, size);
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

  // bailout value
  const double bailout = 512;

  // find minimum quantization error in matrix
  while (count > size)
  {
    int ii = 0, jj = 0;
    double *a = &(colors[0].freq);
    double least_err = 99999999;

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

            if (least_err < bailout && ii != 0 && jj != 0)
              goto found;
          }

          e++;
          b += freq_step;
        }
      }

      a += freq_step;
    }

    found:

    // merge pair
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
      const int r = colors[i].r;
      const int g = colors[i].g;
      const int b = colors[i].b;

      pal->data[index] = makeRgb(r, g, b);
      index++;
    }
  }

  pal->max = index;
  Gui::restoreStatusInfo();
}

