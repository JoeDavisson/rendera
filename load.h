#ifndef LOAD_H
#define LOAD_H

#include "rendera.h"

void load(Fl_Widget *, void *);
Fl_Image *preview_jpg(const char *, unsigned char *, int);
Fl_Image *preview_png(const char *, unsigned char *, int);
Fl_Image *preview_bmp(const char *, unsigned char *, int);
Fl_Image *preview_tga(const char *, unsigned char *, int);
void load_jpg(const char *, Bitmap *, int);
void load_bmp(const char *, Bitmap *, int);
void load_tga(const char *, Bitmap *, int);
void load_png(const char *, Bitmap *, int);

#endif

