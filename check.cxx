#include "rendera.h"

void check_palette(Widget *widget, void *var)
{
  widget->bitmap->clear(*(int *)var * 12345 | 0xff000000);
  widget->redraw();
}

void check_zoom_in(Button *button, void *var)
{
  gui->view->zoom_in(gui->view->w() / 2, gui->view->h() / 2);
}

void check_zoom_out(Button *button, void *var)
{
  gui->view->zoom_out(gui->view->w() / 2, gui->view->h() / 2);
}

void check_zoom_fit(ToggleButton *button, void *var)
{
//  int *v = (int *)var;
//  *v = 1 - *v;
  gui->view->zoom_fit(*(int *)var);
}

void check_zoom_one(Button *button, void *var)
{
  gui->zoom_fit->var = 0;
  gui->zoom_fit->redraw();
  gui->view->zoom_one();
}

