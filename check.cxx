#include "rendera.h"

extern Dialog *dialog;
extern Gui *gui;

static int brush_sizes[16] = {
  1, 2, 3, 4, 8, 12, 16, 24,
  32, 40, 48, 56, 64, 72, 80, 88
};

void update_color(int c)
{
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);

  int h, s, v;

  Blend::rgb_to_hsv(r, g, b, &h, &s, &v);

  float angle = ((3.14159 * 2) / 1536) * h;
  int mx = 48 + 40 * cosf(angle);
  int my = 48 + 40 * sinf(angle);
  gui->hue->var = mx + 96 * my;
  gui->sat->var = s / 2.684f;
  gui->val->var = v / 2.684f;

  gui->hue->do_callback();

  Brush::main->color = c;
}

void check_palette(Widget *widget, void *var)
{
  Palette *palette = Palette::main;
  int pos = *(int *)var;

  int stepx = widget->stepx;
  int stepy = widget->stepy;
  int divx = 96 / stepx;
  int divy = 96 / stepy;

  int x = pos % divx;
  int y = pos / divx;

  if(y > (palette->max - 1) / divx)
  {
    y = (palette->max - 1) / divx;
    pos = x + divx * y;
    x = pos % divx;
    y = pos / divx;
    widget->var = pos;
  }

  if(pos > palette->max - 1)
  {
    pos = palette->max - 1;
    x = pos % divx;
    y = pos / divx;
    widget->var = pos;
  }

  int c = widget->bitmap->getpixel(x * stepx, y * stepy);
  update_color(c);
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

void check_tool(Widget *widget, void *var)
{
  if(gui->view->tool_started)
    return;
  gui->view->tool = *(int *)var;
}

void check_color(Widget *widget, void *var)
{
  int pos = gui->hue->var;
  int mx = pos % 96;
  int my = pos / 96;

  float mouse_angle = atan2f(my - 48, mx - 48);
  int h = ((int)(mouse_angle * 244.46f) + 1536) % 1536;
  int s = gui->sat->var * 2.684f;
  int v = gui->val->var * 2.684f;

  int r, g, b;

  Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
  Brush::main->color = makecol(r, g, b);
  Brush::main->trans = gui->trans->var * 2.685f;
  Brush::main->blend = gui->blend->var;

  int i;
  int lastx1 = 48 + 40;
  int lasty1 = 48;
  int lastx2 = 48 + 20;
  int lasty2 = 48;
  int px[4];
  int py[4];

  gui->hue->bitmap->clear((Fl::get_color(FL_BACKGROUND_COLOR) >> 8) | 0xFF000000);

  for(i = 1; i < 1536; i++)
  {
    float angle = ((3.14159 * 2) / 1536) * i;
    int x1 = 48 + 40 * cosf(angle);
    int y1 = 48 + 40 * sinf(angle);
    int x2 = 48 + 20 * cosf(angle);
    int y2 = 48 + 20 * sinf(angle);
    Blend::hsv_to_rgb(i, 255, 255, &r, &g, &b);
    gui->hue->bitmap->line(x1, y1, x2, y2, makecol(r, g, b), 0);
    gui->hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makecol(r, g, b), 0);
  }

  int x1 = 48 + 40 * cosf(mouse_angle);
  int y1 = 48 + 40 * sinf(mouse_angle);
  int x2 = 48 + 20 * cosf(mouse_angle);
  int y2 = 48 + 20 * sinf(mouse_angle);
  gui->hue->bitmap->xor_line(x1, y1, x2, y2);

  for(i = 0; i < 96; i++)
  {
    Blend::hsv_to_rgb(h, i * 2.685f, v, &r, &g, &b);
    gui->sat->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
    Blend::hsv_to_rgb(h, s, i * 2.685f, &r, &g, &b);
    gui->val->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
  }

  gui->hue->redraw();
  gui->sat->redraw();
  gui->val->redraw();
}

void check_wrap(Widget *widget, void *var)
{
  Bitmap::wrap = *(int *)var;
}

void check_clone(Widget *widget, void *var)
{
  Bitmap::clone = *(int *)var;
}

void check_mirror(Widget *widget, void *var)
{
  Bitmap::clone_mirror = *(int *)var;
}

void check_origin(Widget *widget, void *var)
{
  gui->view->stroke->origin = *(int *)var;
}

void check_constrain(Widget *widget, void *var)
{
  gui->view->stroke->constrain = *(int *)var;
}

// dialogs
void show_about()
{
  dialog->about->show();
}

void hide_about()
{
  dialog->about->hide();
}

void show_new_image()
{
  char s[8];
  snprintf(s, sizeof(s), "%d", Bitmap::main->w - 64);
  dialog->new_image_width->value(s);
  snprintf(s, sizeof(s), "%d", Bitmap::main->h - 64);
  dialog->new_image_height->value(s);
  dialog->new_image->show();
}

void hide_new_image()
{
  char s[8];

  int w = atoi(dialog->new_image_width->value());
  int h = atoi(dialog->new_image_height->value());

  if(w < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    dialog->new_image_width->value(s);
    return;
  }

  if(h < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    dialog->new_image_height->value(s);
    return;
  }

  if(w > 10000)
  {
    snprintf(s, sizeof(s), "%d", 10000);
    dialog->new_image_width->value(s);
    return;
  }

  if(h > 10000)
  {
    snprintf(s, sizeof(s), "%d", 10000);
    dialog->new_image_height->value(s);
    return;
  }

  dialog->new_image->hide();

  w += 64;
  h += 64;

  delete Bitmap::main;
  int overscroll = Bitmap::overscroll;
  Bitmap::main = new Bitmap(w, h);
  Bitmap::main->clear(makecol(0, 0, 0));
  Bitmap::main->set_clip(overscroll, overscroll, w - overscroll - 1, h - overscroll - 1);
  Bitmap::main->rectfill(overscroll, overscroll, w - overscroll - 1, h - overscroll - 1, makecol(255, 255, 255), 0);

  delete Map::main;
  Map::main = new Map(w, h);

  gui->view->ox = 0;
  gui->view->oy = 0;
  gui->view->zoom_fit(0);
  gui->view->draw_main(1);
}

void cancel_new_image()
{
  dialog->new_image->hide();
}

void show_create_palette()
{
  char s[8];
  snprintf(s, sizeof(s), "%d", Palette::main->max);
  dialog->create_palette_colors->value(s);
  dialog->create_palette->show();
}

void hide_create_palette()
{
  char s[8];

  int colors = atoi(dialog->create_palette_colors->value());

  if(colors < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    dialog->create_palette_colors->value(s);
    return;
  }

  if(colors > 256)
  {
    snprintf(s, sizeof(s), "%d", 256);
    dialog->create_palette_colors->value(s);
    return;
  }

  dialog->create_palette->hide();

  quantize(Bitmap::main, colors, Bitmap::overscroll);
}

void cancel_create_palette()
{
  dialog->create_palette->hide();
}

