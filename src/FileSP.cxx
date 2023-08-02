/*
Copyright (c) 2023 Joe Davisson.

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

#include "FileSP.H"

// This is a C-style FILE pointer wrapped in a "smart pointer", which
// automatically closes the file when the pointer goes out of scope.
FileSP::FileSP(const char *fn, const char *mode)
{
  f = fopen(fn, mode);
}

FileSP::~FileSP()
{
  if (f)
    fclose(f);
}

FILE *FileSP::get()
{
  return f;
}

