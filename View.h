#ifndef VIEW_H
#define VIEW_H

#include "rendera.h"

class View : public Fl_Widget
{
public:
  View(Fl_Group *, int, int, int, int, const char *);
  virtual ~View();

  virtual int handle(int);
  virtual void resize(int, int, int, int);
  Fl_Group *group;
  Fl_RGB_Image *image;
  Bitmap *bitmap;
protected:
  virtual void draw();
};

#endif

