#ifndef BITMAP_H
#define BITMAP_H

#include "rendera.h"

class Bitmap
{
public:
  Bitmap(int, int);
  virtual ~Bitmap();

  void clear(int);

  int x, y, w, h;
  int cx, cy, cw, ch;
  int *data;
  int **row;
};

#endif
