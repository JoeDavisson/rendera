#ifndef BUTTON_H
#define BUTTON_H

#include "rendera.h"

class Button : public Fl_Button
{
public:
  Button(Fl_Group *, int, int, int, int, const char *, const char *);
  virtual ~Button();

  Fl_Group *group;
  Fl_PNG_Image *image;
protected:
  virtual void draw();
};

#endif

