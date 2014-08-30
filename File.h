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
  void loadFile(const char *);
  void loadJPG(const char *, Bitmap *, int);
  void loadBMP(const char *, Bitmap *, int);
  void loadTGA(const char *, Bitmap *, int);
  void loadPNG(const char *, Bitmap *, int);

  void save(Fl_Widget *, void *);
  void saveBMP(const char *);
  void saveTGA(const char *);
  void savePNG(const char *);
  void saveJPG(const char *);

  Fl_Image *previewJPG(const char *, unsigned char *, int);
  Fl_Image *previewPNG(const char *, unsigned char *, int);
  Fl_Image *previewBMP(const char *, unsigned char *, int);
  Fl_Image *previewTGA(const char *, unsigned char *, int);
  Fl_Image *previewGPL(const char *, unsigned char *, int);
}

#endif

