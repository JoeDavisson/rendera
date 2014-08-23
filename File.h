/*
Copyright (c) 2014 Joe Davisson.

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

#ifndef FILE_H
#define FILE_H

#include "rendera.h"

class File
{
public:
  static void load(Fl_Widget *, void *);
  static void loadJPG(const char *, Bitmap *, int);
  static void loadBMP(const char *, Bitmap *, int);
  static void loadTGA(const char *, Bitmap *, int);
  static void loadPNG(const char *, Bitmap *, int);

  static void save(Fl_Widget *, void *);
  static void saveBMP(const char *);
  static void saveTGA(const char *);
  static void savePNG(const char *);
  static void saveJPG(const char *);

  static Fl_Image *previewJPG(const char *, unsigned char *, int);
  static Fl_Image *previewPNG(const char *, unsigned char *, int);
  static Fl_Image *previewBMP(const char *, unsigned char *, int);
  static Fl_Image *previewTGA(const char *, unsigned char *, int);
  static Fl_Image *previewGPL(const char *, unsigned char *, int);
};

#endif

