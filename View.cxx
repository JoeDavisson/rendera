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
  switch(event)
  {
    case FL_PUSH:
    case FL_DRAG:
      int x1 = Fl::event_x();
      int y1 = Fl::event_y();
//      int button = Fl::event_button1();

//      if(button == 0)
//        return 0;

//      bmp->main->clear(makecol(0, 255, 0));
      bmp->main->setpixel_solid(x1 - x() - 1, y1 - y() - 2, makecol(0, 0, 0), 0);
      bmp->main->blit(bitmap, 0, 0, 0, 0, bmp->main->w, bmp->main->h);
      redraw();

      return 1;
  }
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  delete image;
  delete bitmap;
  bitmap = new Bitmap(w, h);
  bitmap->clear(makecol(0, 0, 0));
  bmp->main->blit(bitmap, 0, 0, 0, 0, bmp->main->w, bmp->main->h);
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);
  Fl_Widget::resize(x, y, w, h);
}

void View::draw()
{
  image->draw(x(), y());
}

