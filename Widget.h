#ifndef WIDGET_H
#define WIDGET_H

#include "rendera.h"

class Widget : public Fl_Widget
{
public:
  Widget(Fl_Group *, int, int, int, int, const char *, const char *, int, int);
  Widget(Fl_Group *, int, int, int, int, const char *, int, int);
  virtual ~Widget();

  virtual int handle(int);
  int var;
  int stepx;
  int stepy;
  Fl_Group *group;
  Fl_PNG_Image *png_image;
  Fl_RGB_Image *image;
  int *image_data;
protected:
  virtual void draw();
  int use_png;
};

#endif

