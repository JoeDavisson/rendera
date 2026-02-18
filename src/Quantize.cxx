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

namespace
{
  int makeRgbShift(const int r, const int g, const int b, const int shift)
  {
    return r | g << shift | b << (shift * 2);
  }

  int getrShift(const int c, const int shift)
  {
    return c & ((1 << shift) - 1);
  }

  int getgShift(const int c, const int shift)
  {
    return (c >> shift) & ((1 << shift) - 1);
  }

  int getbShift(const int c, const int shift)
  {
    return (c >> (shift * 2)) & ((1 << shift) - 1);
  }
}

bool Quantize::sort_greater_freq(const color_type &a, const color_type &b)
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
                          int num_bins, int samples, int size, int pixel_count)
{
  // reserve up to 512 colors with a higher frequency (based on the image size)
  // to preserve small areas of color which may otherwise get wiped out
  const int bin_size = std::cbrt(num_bins);
  const int bin_step = bin_size / 8;
  const int bin_shift = std::log2(bin_size);

  for (int b = 0; b <= bin_size - bin_step; b += bin_step)
  {
    for (int g = 0; g <= bin_size - bin_step; g += bin_step)
    {
      for (int r = 0; r <= bin_size - bin_step; r += bin_step)
      {
        double r_avg = 0;
        double g_avg = 0;
        double b_avg = 0;
        int div = 0;

        for (int k = 0; k < bin_step; k++)
        {
          const int bk = b + k;

          for (int j = 0; j < bin_step; j++)
          {
            const int gj = g + j;

            for (int i = 0; i < bin_step; i++)
            {
              const int ri = r + i;
              const int index = makeRgbShift(ri, gj, bk, bin_shift);

              double freq = color_bin[index].freq;

              if (freq > 0)
              {
                double rr = color_bin[index].r;
                double gg = color_bin[index].g;
                double bb = color_bin[index].b;

                r_avg += rr * rr;
                g_avg += gg * gg;
                b_avg += bb * bb;
                div++;
              }
            }
          }
        }

        if (div > 0)
        {
          r_avg = std::sqrt(r_avg / div);
          g_avg = std::sqrt(g_avg / div);
          b_avg = std::sqrt(b_avg / div);

          const int index = makeRgbShift((int)r_avg >> (8 - bin_shift),
                                         (int)g_avg >> (8 - bin_shift),
                                         (int)b_avg >> (8 - bin_shift),
                                         bin_shift);

          color_bin[index].r = r_avg;
          color_bin[index].g = g_avg;
          color_bin[index].b = b_avg;
          color_bin[index].freq = (double)pixel_count / samples;
        }
      }
    }
  }

  // sort by popularity
  std::sort(color_bin.begin(), color_bin.end(), sort_greater_freq);

  // find last element
  int color_bin_count = 0;

  for (int i = 0; i < num_bins; i++)
  {
    if (color_bin[i].freq == 0)
      break;

    color_bin_count++;
  }

  // the sampling curve depends on the number of target colors
  const double r = std::pow(color_bin_count, 1.0 / (samples - 1));
  const double curve = (double)size / 256;
  int count = 0;

  for (int i = 0; i < samples; i++)
  {
    const double index_lin = (double)i * (double)(color_bin_count - 1) /
                             (samples - 1);
    const double index_log = std::pow(r, (double)i) - 1.0;
    int index = index_lin + curve * (index_log - index_lin);

    if (index < 0)
      index = 0;

    if (index > samples - 1)
      index = samples - 1;

    colors[count].r = color_bin[index].r;
    colors[count].g = color_bin[index].g;
    colors[count].b = color_bin[index].b;
    colors[count].freq = color_bin[index].freq;

    count++;
  }

  return count;
}

void Quantize::pca(Bitmap *src, Palette *pal, int size, int samples)
{
  const int pixel_count = src->w * src->h;
  const int num_bins = size < 64 ? 32768 : 262144;
  const int bin_size = std::cbrt(num_bins);
  const int bin_step = 256 / bin_size;
  const int bin_shift = std::log2(bin_size);

  Gui::saveStatusInfo();
  Gui::statusInfo("Creating Color List...");

  // average colors and place them into bins
  std::vector<color_type> color_bin(num_bins);

  for (int i = 0; i < num_bins; i++)
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

      const int rs = rgba.r >> (8 - bin_shift);
      const int gs = rgba.g >> (8 - bin_shift);
      const int bs = rgba.b >> (8 - bin_shift);
      const int index = makeRgbShift(rs, gs, bs, bin_shift);

      color_bin[index].r += rgba.r * rgba.r;
      color_bin[index].g += rgba.g * rgba.g;
      color_bin[index].b += rgba.b * rgba.b;
      color_bin[index].freq += weight;
    }
  }

  for (int b = 0; b < 256; b += bin_step)
  {
    for (int g = 0; g < 256; g += bin_step)
    {
      for (int r = 0; r < 256; r += bin_step)
      {
        const int rs = r >> (8 - bin_shift);
        const int gs = g >> (8 - bin_shift);
        const int bs = b >> (8 - bin_shift);

        const int index = makeRgbShift(rs, gs, bs, bin_shift);
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

    for (int i = 0; i < num_bins; i++)
    {
      const int rs = getrShift(i, bin_shift);
      const int gs = getgShift(i, bin_shift);
      const int bs = getbShift(i, bin_shift);
      const int index = makeRgbShift(rs, gs, bs, bin_shift);
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
    count = limitColors(color_bin, colors,
                        num_bins, samples, size, pixel_count);
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
    double least_err = std::numeric_limits<double>::max();

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

