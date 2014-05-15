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
  int wrap;
  int mirror;
};

#endif

