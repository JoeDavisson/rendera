#include "rendera.h"

Button::Button(Fl_Group *g, int x, int y, int w, int h, const char *label, const char *filename)
: Fl_Button(x, y, w, h, label)
{
  group = g;
  image = new Fl_PNG_Image(filename);
  resize(group->x() + x, group->y() + y, w, h);
  tooltip(label);
}

Button::~Button()
{
}

void Button::draw()
{
  if(value())
    image->draw(x() + 1, y() + 1);
  else
    image->draw(x(), y());
  fl_draw_box(value() ? FL_DOWN_FRAME : FL_UP_FRAME, x(), y(), w(), h(), FL_BLACK);
}

