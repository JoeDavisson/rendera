#ifndef LOAD_H
#define LOAD_H

#include "rendera.h"

void load(Fl_Widget *, void *);
Fl_Image *preview_jpg(const char *, unsigned char *, int);
void load_jpg(const char *);
void load_bmp(const char *);
void load_tga(const char *);
void load_png(const char *);

#endif

