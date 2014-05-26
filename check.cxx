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

void check_size(Widget *widget, void *var)
{
  Brush *brush = Brush::main;

  int size = brush_sizes[*(int *)var];
  int shape = gui->shape->var;

  brush->make(shape, size);
  gui->brush->bitmap->clear(makecol(255, 255, 255));
  int i;
  for(i = 0; i < Brush::main->solid_count; i++)
    gui->brush->bitmap->setpixel_solid(48 + brush->solidx[i], 48 + brush->solidy[i], makecol(0, 0, 0), 0);
  gui->brush->redraw();
}

void check_stroke(Widget *widget, void *var)
{
  gui->view->stroke->type = *(int *)var;
}

void check_edge(Widget *widget, void *var)
{
  Brush::main->edge = *(int *)var;
}

void check_smooth(Widget *widget, void *var)
{
  Brush::main->smooth = *(int *)var;
}

void check_color(Widget *widget, void *var)
{
  int h = gui->hue->var * 16;
  int s = gui->sat->var * 2.684f;
  int v = gui->val->var * 2.684f;

  int r, g, b;

  Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
  Brush::main->color = makecol(r, g, b);
  Brush::main->trans = gui->trans->var * 2.685f;
  Brush::main->blend = gui->blend->var;

  int i;
  for(i = 0; i < 96; i++)
  {
    Blend::hsv_to_rgb(i * 16, 255, 255, &r, &g, &b);
    gui->hue->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
    Blend::hsv_to_rgb(h, i * 2.685f, v, &r, &g, &b);
    gui->sat->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
    Blend::hsv_to_rgb(h, s, i * 2.685f, &r, &g, &b);
    gui->val->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
  }

  gui->hue->redraw();
  gui->sat->redraw();
  gui->val->redraw();
}

