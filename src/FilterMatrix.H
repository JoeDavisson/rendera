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

#ifndef FILTER_MATRIX_H
#define FILTER_MATRIX_H

// data used by the convolution matrix filter
class FilterMatrix
{
public:
  static const int identity[3][3];
  static const int blur[3][3];
  static const int sharpen[3][3];
  static const int gaussian[3][3];
  static const int edge[3][3];
  static const int emboss[3][3];
  static const int emboss_reverse[3][3];
  static const int sobel1[3][3];
  static const int sobel2[3][3];
  static const int test[3][3];

private:
  FilterMatrix() { }
  ~FilterMatrix() { }
};

#endif

