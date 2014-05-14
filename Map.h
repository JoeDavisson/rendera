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
  void line(int, int, int, int, int);
  void oval(int, int, int, int, int);
  void ovalfill(int, int, int, int, int);
  void rect(int, int, int, int, int);
  void rectfill(int, int, int, int, int);
  void hline(int, int, int, int);
  void vline(int, int, int, int);
};

#endif
