#ifndef BMP_H
#define BMP_H

#include "rendera.h"

class Bmp
{
public:
  Bmp();
  virtual ~Bmp();

  Bitmap *main;
  Bitmap *clone;
};

#endif

