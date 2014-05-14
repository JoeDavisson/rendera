#ifndef MAP_H
#define MAP_H

#include "rendera.h"

class Map
{
public:
  Map(int, int);
  virtual ~Map();

  int w, h;
  unsigned char *data;
  unsigned char *row;

  void clear(int);
  void setpixel(int, int, int);
  int getpixel(int, int);
};

#endif
