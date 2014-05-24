#include "rendera.h"

extern Gui *gui;

static int brush_sizes[16] = {
  1, 2, 3, 4, 8, 12, 16, 24,
  32, 40, 48, 56, 64, 72, 80, 88
};

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
  gui->view->zoom_fit(*(int *)var);
}

void check_zoom_one(Button *button, void *var)
{
  gui->zoom_fit->var = 0;
  gui->zoom_fit->redraw();
  gui->view->zoom_one();
}

void check_grid(ToggleButton *button, void *var)
{
  gui->view->grid = *(int *)var;
  gui->view->draw_main(1);
}

void check_gridx(Field *field, void *var)
{
  int num = atoi(field->value());
  if(num < 1)
    num = 1;
  if(num > 256)
    num = 256;
  char s[8];
  snprintf(s, sizeof(s), "%d", num);
  field->value(s);
  gui->view->gridx = num;
  gui->view->draw_main(1);
}

void check_gridy(Field *field, void *var)
{
  int num = atoi(field->value());
  if(num < 1)
    num = 1;
  if(num > 256)
    num = 256;
  char s[8];
  snprintf(s, sizeof(s), "%d", num);
  field->value(s);
  gui->view->gridy = num;
  gui->view->draw_main(1);
}

void check_size(Widget *field, void *var)
{
  int size = brush_sizes[*(int *)var];

  gui->view->brush->make(size);
}

