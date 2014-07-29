#ifndef PREVIEW_H
#define PREVIEW_H

#include "rendera.h"

Fl_Image *preview_jpg(const char *, unsigned char *, int);
Fl_Image *preview_png(const char *, unsigned char *, int);
Fl_Image *preview_bmp(const char *, unsigned char *, int);
Fl_Image *preview_tga(const char *, unsigned char *, int);
Fl_Image *preview_pal(const char *, unsigned char *, int);

#endif

