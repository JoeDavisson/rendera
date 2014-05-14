#include "rendera.h"

// load PNG from file
Widget::Widget(Fl_Group *g, int x, int y, int w, int h, const char *label, const char *filename, int sx, int sy)
: Fl_Widget(x, y, w, h, label)
{
  use_png = 1;
  var = 0;
  stepx = sx;
  stepy = sy;
  group = g;
  png_image = new Fl_PNG_Image(filename);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
}

// user-defined image
Widget::Widget(Fl_Group *g, int x, int y, int w, int h, const char *label, int sx, int sy)
: Fl_Widget(x, y, w, h, label)
{
  use_png = 0;
  var = 0;
  stepx = sx;
  stepy = sy;
  group = g;
  bitmap = new Bitmap(w, h);
  image = new Fl_RGB_Image((unsigned char *)bitmap->data, w, h, 4, 0);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
}

Widget::~Widget()
{
}

int Widget::handle(int event)
{
  int x1, y1;

  if(stepx == 0 || stepy == 0)
    return 0;

  switch(event)
  {
    case FL_PUSH:
    case FL_DRAG:
      x1 = (Fl::event_x() - x()) / stepx;
      if(x1 > w() / stepx - 1)
        x1 = w() / stepx - 1;
      if(x1 < 0)
        x1 = 0;

      y1 = (Fl::event_y() - y()) / stepy;
      if(y1 > h() / stepy - 1)
        y1 = h() / stepy - 1;
      if(y1 < 0)
        y1 = 0;

      var = x1 + (w() / stepx) * y1;
      do_callback();
      redraw();
      break;
  }

  return 1;
}

void Widget::draw()
{
  if(use_png)
    png_image->draw(x(), y());
  else
    image->draw(x(), y());
    
  fl_draw_box(FL_DOWN_FRAME, x(), y(), w(), h(), FL_BLACK);

  if(stepx == 0 || stepy == 0)
    return;

  int offsety = (var / (w() / stepx)) * stepy;
  int offsetx = var;

  while(offsetx >= (w() / stepx))
    offsetx -= (w() / stepx);

  offsetx *= stepx;

  fl_push_clip(x() + offsetx, y() + offsety, stepx, stepy);

  if(use_png)
    png_image->draw(x() + offsetx, y() + offsety, stepx, stepy,
                    offsetx + 1, offsety + 1);
  else
    image->draw(x() + offsetx, y() + offsety, stepx, stepy,
                offsetx + 1, offsety + 1);

  fl_pop_clip();

  fl_draw_box(FL_UP_FRAME, x() + offsetx, y() + offsety, stepx, stepy, FL_BLACK);
}

