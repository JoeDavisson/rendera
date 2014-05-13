#ifndef BLEND_H
#define BLEND_H

#include "rendera.h"

class Blend
{
public:
  Blend();
  virtual ~Blend();

  int invert(int, int, int);
  int trans(int, int, int);
  int add(int, int, int);
  int sub(int, int, int);
  int colorize(int, int, int);
  int force_lum(int, int);
  void hsv_to_rgb(int, int, int, int *, int *, int *);
  void rgb_to_hsv(int, int, int, int *, int *, int *);

  int (*mode)(int, int, int);
};

#endif

