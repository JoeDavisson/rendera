#ifndef STROKE_H
#define STROKE_H

#include "rendera.h"

class Stroke
{
public:
  Stroke();
  virtual ~Stroke();

  void freehand();
  void filledarea();
  void line();
  void polygon();
  void rect();
  void rectfill();
  void oval();
  void ovalfill();

  int current;
  int x1, y1, x2, y2;
};

#endif

