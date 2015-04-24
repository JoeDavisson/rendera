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

#ifndef FILE_H
#define FILE_H

class Fl_Widget;
class Fl_Image;
class Bitmap;

namespace File
{
  void load(Fl_Widget *, void *);
  int loadFile(const char *);
  Bitmap *loadJpeg(const char *, int);
  Bitmap *loadBmp(const char *, int);
  Bitmap *loadTarga(const char *, int);
  Bitmap *loadPng(const char *, int);
  Bitmap *loadPngFromArray(const unsigned char *, int);

  void save(Fl_Widget *, void *);
  int saveBmp(const char *);
  int saveTarga(const char *);
  int savePng(const char *);
  int saveJpeg(const char *);

  void loadPalette();
  void savePalette();

  Fl_Image *previewJpeg(const char *, unsigned char *, int);
  Fl_Image *previewPng(const char *, unsigned char *, int);
  Fl_Image *previewBmp(const char *, unsigned char *, int);
  Fl_Image *previewTarga(const char *, unsigned char *, int);
  Fl_Image *previewGimpPalette(const char *, unsigned char *, int);

  void decodeURI(char *);
  void getDirectory(char *, const char *);
}

#endif

