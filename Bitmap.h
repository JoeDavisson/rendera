#ifndef BITMAP_H
#define BITMAP_H

#include "rendera.h"

class Bitmap
{
public:
  Bitmap(int, int);
  virtual ~Bitmap();

  int w, h;
  int cl, cr, ct, cb, cw, ch;
  int *data;
  int *row;

  void clear(int);
  void setpixel_solid(int, int, int ,int);
  void setpixel_wrap(int, int, int ,int);
  void setpixel_clone(int, int, int ,int);
  void setpixel_wrap_clone(int, int, int ,int);
  int getpixel(int, int);
  void clip(int *, int *, int *, int *);
  void set_clip(int, int, int, int);
  void blit(Bitmap *, int, int, int, int, int, int);
};

#endif
