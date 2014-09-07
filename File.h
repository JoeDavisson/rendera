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

class Bitmap;

#include "rendera.h"

namespace File
{
  void load(Fl_Widget *, void *);
  int loadFile(const char *);
  Bitmap *loadJPG(const char *, int);
  Bitmap *loadBMP(const char *, int);
  Bitmap *loadTGA(const char *, int);
  Bitmap *loadPNG(const char *, int);

  void save(Fl_Widget *, void *);
  int saveBMP(const char *);
  int saveTGA(const char *);
  int savePNG(const char *);
  int saveJPG(const char *);

  void loadPalette();
  void savePalette();

  Fl_Image *previewJPG(const char *, unsigned char *, int);
  Fl_Image *previewPNG(const char *, unsigned char *, int);
  Fl_Image *previewBMP(const char *, unsigned char *, int);
  Fl_Image *previewTGA(const char *, unsigned char *, int);
  Fl_Image *previewGPL(const char *, unsigned char *, int);

  void decodeURI(char *);
}

#endif

