#ifndef FIELD_H
#define FIELD_H

#include "rendera.h"

class Field : public Fl_Input
{
public:
  Field(Fl_Group *, int, int, int, int, const char *);
  virtual ~Field();

  Fl_Group *group;
};

#endif

