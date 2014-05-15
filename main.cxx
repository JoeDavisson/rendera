#include "rendera.h"
#include <iostream>

using namespace std;

Gui *gui;
Var *var;
Bmp *bmp;
Blend *blend;
Stroke *stroke;

static void check_palette(Fl_Widget *widget, void *var)
{
  gui->palette->bitmap->clear(*(int *)var * 12345 | 0xff000000);
  gui->palette->redraw();
}

int main(int argc, char **argv)
{
  gui = new Gui();
  var = new Var();
  bmp = new Bmp();
  blend = new Blend();

  gui->palette->callback((Fl_Callback *)check_palette, &gui->palette->var);

  return Fl::run();
}

