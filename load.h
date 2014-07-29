#ifndef LOAD_H
#define LOAD_H

#include "rendera.h"

void load(Fl_Widget *, void *);
void load_jpg(const char *, Bitmap *, int);
void load_bmp(const char *, Bitmap *, int);
void load_tga(const char *, Bitmap *, int);
void load_png(const char *, Bitmap *, int);

#endif

