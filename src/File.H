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

#ifndef FILE_H
#define FILE_H

#include "../fltk/png/png.h"

#ifdef __WIN32
#define HAVE_BOOLEAN
// prevent jmorecfg.h from defining INT32
#ifndef XMD_H
#define XMD_H
#endif
#endif

#include "../fltk/jpeg/jpeglib.h"

// this must be included after pnglib
#include <setjmp.h>

class Fl_Widget;
class Fl_Image;
class Bitmap;

class File
{
public:
  static void init();
  static void load(Fl_Widget *, void *);
  static int loadFile(const char *);
  static Bitmap *loadJpeg(const char *);
  static Bitmap *loadBmp(const char *);
  static Bitmap *loadTarga(const char *);
  static Bitmap *loadPng(const char *);
  static Bitmap *loadPngFromArray(const unsigned char *);

  static void save(Fl_Widget *, void *);
  static int saveBmp(Bitmap *, const char *);
  static int saveTarga(Bitmap *, const char *);
  static int savePng(Bitmap *, const char *);
  static int saveJpeg(Bitmap *, const char *);

  static void loadPalette();
  static void savePalette();
  static void loadSelection();
  static void saveSelection();

  //Fl_Image *previewJpeg(const char *, unsigned char *, int);
  //Fl_Image *previewPng(const char *, unsigned char *, int);
  //Fl_Image *previewBmp(const char *, unsigned char *, int);
  //Fl_Image *previewTarga(const char *, unsigned char *, int);
  //Fl_Image *previewGimpPalette(const char *, unsigned char *, int);

  static void decodeURI(char *);
  static void getDirectory(char *, const char *);
  static void getFilename(char *, const char *);

private:
  File() { }
  ~File() { }

  static int last_type;
  static char load_dir[256];
  static char save_dir[256];
  static char pal_load_dir[256];
  static char pal_save_dir[256];
  static const char *ext_string[];

  static void jpeg_exit(j_common_ptr);
  static void errorMessage();
  static bool fileExists(const char *);
  static bool isPng(const unsigned char *);
  static bool isJpeg(const unsigned char *);
  static bool isBmp(const unsigned char *);
  static bool isTarga(const char *);
  static bool isGimpPalette(const unsigned char *);
  static void pngReadFromArray(png_structp, png_bytep, png_uint_32);
};

#endif

