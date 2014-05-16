#include "rendera.h"

View::View(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Widget(x, y, w, h, label)
{
  group = g;
  image = 0;
  backbuf = 0;
  ox = 0;
  oy = 0;
  zoom = 2;
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
      int x1 = Fl::event_x() - x() - 1;
      int y1 = Fl::event_y() - y() - 1;
/*
      switch(Fl::event_button())
      {
        case 1:
          
          return 1;
*/
      bmp->main->setpixel_solid(x1 / zoom, y1 / zoom, makecol(0, 0, 0), 0);
      refresh();

      return 1;
  }
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  delete backbuf;
  delete image;
  backbuf = new Bitmap(w, h);
  backbuf->clear(makecol(0, 0, 0));
  image = new Fl_RGB_Image((unsigned char *)backbuf->data, w, h, 4, 0);
  refresh();
  Fl_Widget::resize(x, y, w, h);
}

void View::refresh()
{
  int sw = w() / zoom;
  int sh = h() / zoom;
  sw += 2;
  sh += 2;

  if(sw > bmp->main->w - ox)
    sw = bmp->main->w - ox;
  if(sh > bmp->main->h - oy)
    sh = bmp->main->h - oy;

  Bitmap *temp = new Bitmap(sw, sh);
  bmp->main->blit(temp, ox, oy, 0, 0, sw, sh);

  int dw = sw * zoom;
  int dh = sh * zoom;

  int overx = dw - w();
  int overy = dh - h();

  if(zoom < 2)
  {
    overx = 0;
    overy = 0;
  }

//  temp->blit(backbuf, 0, 0, 0, 0, sw, sh);
  temp->point_stretch(backbuf, 0, 0, sw, sh, 0, 0, dw, dh, overx, overy);

  delete temp;
  redraw();
}

void View::draw()
{
  image->draw(x(), y());
}

