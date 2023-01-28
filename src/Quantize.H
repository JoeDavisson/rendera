/*
Copyright (c) 2021 Joe Davisson.

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

#ifndef QUANTIZE_H
#define QUANTIZE_H

class Bitmap;
class Octree;
class Palette;

class Quantize
{
public:
  static void pca(Bitmap *, Palette *, int);

private:
  struct color_type
  {
    float r, g, b, freq;
    bool active;
  };

  static void makeColor(color_type *,
                        const float, const float, const float, const float);
  static float error(color_type *, color_type *);
  static void merge(color_type *, color_type *);
  static int limitColors(Octree *, color_type *, int);

  Quantize() { }
  ~Quantize() { }
};

#endif
