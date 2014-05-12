#include "rendera.h"

Field::Field(Fl_Group *g, int x, int y, int w, int h, const char *label)
: Fl_Input(x, y, w, h, 0)
{
  group = g;
  resize(group->x() + x, group->y() + y, w, h);
  type(FL_INT_INPUT);
  maximum_size(3);
  tooltip(label);
}

Field::~Field()
{
}

