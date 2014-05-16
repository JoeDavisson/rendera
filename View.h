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
  void refresh();

  Fl_Group *group;
  Fl_RGB_Image *image;
  Bitmap *backbuf;
  int ox, oy;
  int last_ox, last_oy;
  int zoom;
  int moving;
protected:
  virtual void draw();
};

#endif

