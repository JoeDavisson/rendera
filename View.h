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
  void draw_move();
  void draw_main();
  void begin_move();
  void move();

  Fl_Group *group;
  Fl_RGB_Image *image;
  Bitmap *backbuf;
  int mousex, mousey;
  int imgx, imgy;
  int ox, oy;
  int zoom;
  int moving;
  int px, py, pw, ph;
  int bx, by, bw, bh;
  double aspect, winaspect;
protected:
  virtual void draw();
};

#endif

