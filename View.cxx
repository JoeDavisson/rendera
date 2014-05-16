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
  moving = 0;
  resize(group->x() + x, group->y() + y, w, h);
}

View::~View()
{
}

int View::handle(int event)
{
  int mousex = Fl::event_x() - x() - 1;
  int mousey = Fl::event_y() - y() - 2;
  int imgx = (mousex - ox) / zoom;
  int imgy = (mousey - oy) / zoom;
  int button = Fl::event_button();

  switch(event)
  {
    case FL_PUSH:
      switch(button)
      {
        case 1:
          bmp->main->setpixel_solid(imgx, imgy, makecol(0, 0, 0), 0);
          break;
        case 2:
          if(moving == 0)
          {
            last_ox = mousex - ox;
            last_oy = mousey - oy;
            moving = 1;
          }
          break;
      } 
      refresh();
      return 1;
    case FL_DRAG:
      switch(button)
      {
        case 1:
          bmp->main->setpixel_solid(imgx, imgy, makecol(0, 0, 0), 0);
          break;
        case 2:
          if(moving == 1)
          {
            ox = mousex - last_ox;
            oy = mousey - last_oy;
          }
          break;
      } 
      refresh();
      return 1;
    case FL_RELEASE:
      moving = 0;
      refresh();
      return 1;
  }
  return 0;
}

void View::resize(int x, int y, int w, int h)
{
  delete image;
  delete backbuf;
  backbuf = new Bitmap(w, h);
  image = new Fl_RGB_Image((unsigned char *)backbuf->data, w, h, 4, 0);
  refresh();
  Fl_Widget::resize(x, y, w, h);
}

void View::refresh()
{
  int sw = w() / zoom;
  int sh = h() / zoom;

  if(sw > bmp->main->w)
    sw = bmp->main->w;
  if(sh > bmp->main->h)
    sh = bmp->main->h;

  int dw = sw * zoom;
  int dh = sh * zoom;

  int overx = dw - w();
  int overy = dh - h();

  if(zoom < 2)
  {
    overx = 0;
    overy = 0;
  }

  backbuf->clear(makecol(0, 255, 0));
  bmp->main->point_stretch(backbuf, 0, 0, bmp->main->w, bmp->main->h, ox, oy, dw, dh, overx, overy);

  redraw();
}

void View::draw()
{
  image->draw(x(), y());
}

