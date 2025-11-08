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

// compute quantization error
float Quantize::error(const color_type *c1, const color_type *c2)
{
  const float r = c1->r - c2->r;
  const float g = c1->g - c2->g;
  const float b = c1->b - c2->b;

  return ((c1->freq * c2->freq) / (c1->freq + c2->freq))
         * (r * r + g * g + b * b);
}

// merge two colors
void Quantize::merge(color_type *c1, color_type *c2)
{
  const float div = c1->freq + c2->freq;
  const float mul = 1.0f / div;

  c1->r = (c1->freq * c1->r + c2->freq * c2->r) * mul;
  c1->g = (c1->freq * c1->g + c2->freq * c2->g) * mul;
  c1->b = (c1->freq * c1->b + c2->freq * c2->b) * mul;
  c1->freq = div;
}

//FIXME todo:
// allow user selection of ycc/rgb averaging

// reduces input color count by averaging sections of the color cube and
// adding their popularities together
int Quantize::limitColors(Octree *histogram, color_type *colors,
                          gamut_type *gamut, int pal_size)
{
  int count = 0;

  int div_y = 64;
  int div_cb = 8;
  int div_cr = 8;

  float step_y = (gamut->high_y - gamut->low_y) / div_y;
  float step_cb = (gamut->high_cb - gamut->low_cb) / div_cb;
  float step_cr = (gamut->high_cr - gamut->low_cr) / div_cr;

  if (step_y < 1)
    step_y = 1;

  if (step_cb < 1)
    step_cb = 1;

  if (step_cr < 1)
    step_cr = 1;

  for (int cr = gamut->low_cr; cr <= (gamut->high_cr + 1) - step_cr; cr += step_cr)
  {
    for (int cb = gamut->low_cb; cb <= (gamut->high_cb + 1) - step_cb; cb += step_cb)
    {
      for (int y = gamut->low_y; y <= (gamut->high_y + 1) - step_y; y += step_y)
      {
        float rr = 0;
        float gg = 0;
        float bb = 0;
        float div = 0;

        for (float k = 0; k < step_cr; k++)
        {
          const float crk = cr + k;

          for (float j = 0; j < step_cb; j++)
          {
            const float cbj = cb + j;

            for (float i = 0; i < step_y; i++)
            {
              const float yi = y + i;

              int r, g, b;

              Blend::yccToRgb(yi, cbj, crk, &r, &g, &b);
               
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

  return count;
}

// An approximation of pairwise clustering quantization, adapted from the
// algorithm described here:
//
// http://www.visgraf.impa.br/Projects/quantization/quant.html
// http://www.visgraf.impa.br/sibgrapi97/anais/pdf/art61.pdf
//
// To make this more practical, input colors are reduced to a maximum of
// 4096 by the limitColors() function above.

void Quantize::pca(Bitmap *src, Palette *pal, int size)
{
  // popularity histogram
  Octree histogram;

  // range of RGB values in image
  gamut_type gamut;

  gamut.low_y = 255;
  gamut.low_cb = 255;
  gamut.low_cr = 255;
  gamut.high_y = 0;
  gamut.high_cb = -255;
  gamut.high_cr = -255;

  // build histogram
  float weight = 1.0 / (src->cw * src->ch);
  int count = 0;

  for (int j = src->ct; j <= src->cb; j++)
  {
    for (int i = src->cl; i <= src->cr; i++)
    {
      rgba_type rgba = getRgba(src->getpixel(i, j));

      float freq = histogram.read(rgba.r, rgba.g, rgba.b);

      if (freq < weight)
        count++;

      histogram.write(rgba.r, rgba.g, rgba.b, freq + weight);

      float y, cb, cr;

      Blend::rgbToYcc(rgba.r, rgba.g, rgba.b, &y, &cb, &cr);

      if (y < gamut.low_y)
        gamut.low_y = y;

      if (cb < gamut.low_cb)
        gamut.low_cb = cb;

      if (cr < gamut.low_cr)
        gamut.low_cr = cr;

      if (y > gamut.high_y)
        gamut.high_y = y; 

      if (cb > gamut.high_cb)
        gamut.high_cb = cb; 

      if (cr > gamut.high_cr)
        gamut.high_cr = cr; 
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
      return;

    // update progress bar
    Progress::update(count);
  }

  // hide progress bar
  Progress::hide();

  // push undo
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

