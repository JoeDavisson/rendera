#ifndef VAR_H
#define VAR_H

#include "rendera.h"

class Var
{
public:
  Var();
  virtual ~Var();

  int deltax, deltay;
  int clonex, cloney;
  int strokex1, strokex2, strokey1, strokey2;
  int wrap;
  int mirror;
};

#endif

