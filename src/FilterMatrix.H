/*
Copyright (c) 2015 Joe Davisson.

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
namespace FilterMatrix
{
  const int identity[3][3] =
  {
    {  0,  0,  0 },
    {  0,  1,  0 },
    {  0,  0,  0 }
  };

  const int blur[3][3] =
  {
    {  1,  1,  1 },
    {  1,  1,  1 },
    {  1,  1,  1 }
  };

  const int sharpen[3][3] =
  {
    { -1, -1, -1 },
    { -1,  9, -1 },
    { -1, -1, -1 }
  };

  const int gaussian[3][3] =
  {
    {  1,  2,  1 },
    {  2,  4,  2 },
    {  1,  2,  1 }
  };

  const int edge[3][3] =
  {
    { -1, -1, -1 },
    { -1,  8, -1 },
    { -1, -1, -1 }
  };

  const int emboss[3][3] =
  {
    { -2, -1,  0 },
    { -1,  1,  1 },
    {  0,  1,  2 }
  };

  const int emboss_reverse[3][3] =
  {
    {  2,  1,  0 },
    {  1,  1, -1 },
    {  0, -1, -2 }
  };
}

#endif

