#include "rendera.h"

View::View(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Widget(x, y, w, h, label)
{
  group = g;
  image = 0;
  bitmap = 0;
  resize(group->x() + x, group->y() + y, w, h);
}

View::~View()
{
}

int View::handle(int event)
{
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  delete image;
  delete bitmap;
  bitmap = new Bitmap(w, h);
  bitmap->clear(0xff000000);
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);
  Fl_Widget::resize(x, y, w, h);
}

void View::draw()
{
  image->draw(x(), y());
}

