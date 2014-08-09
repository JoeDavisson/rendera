#include "rendera.h"

//extern Dialog *dialog;
extern Gui *gui;

static int brush_sizes[16] =
{
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
  gui->sat->var = s / 2.685;
  gui->val->var = v / 2.685;

  gui->hue->do_callback();

  Brush::main->color = c;
}

void check_palette(Widget *widget, void *var)
{
  Palette *palette = Palette::main;
  int pos = *(int *)var;

  int step = widget->stepx;
  int div = widget->w() / step;

  int x = pos % div;
  int y = pos / div;

  if(y > (palette->max - 1) / div)
  {
    y = (palette->max - 1) / div;
    pos = x + div * y;
    x = pos % div;
    y = pos / div;
    widget->var = pos;
  }

  if(pos > palette->max - 1)
  {
    pos = palette->max - 1;
    x = pos % div;
    y = pos / div;
    widget->var = pos;
  }

  int c = widget->bitmap->getpixel(x * step, y * step);
  update_color(c);
}

void check_zoom_in(Button *button, void *var)
{
  gui->view->zoom_in(gui->view->w() / 2, gui->view->h() / 2);
  check_zoom();
}

void check_zoom_out(Button *button, void *var)
{
  gui->view->zoom_out(gui->view->w() / 2, gui->view->h() / 2);
  check_zoom();
}

void check_zoom_fit(ToggleButton *button, void *var)
{
  gui->view->zoom_fit(*(int *)var);
  check_zoom();
}

void check_zoom_one(Button *button, void *var)
{
  gui->zoom_fit->var = 0;
  gui->zoom_fit->redraw();
  gui->view->zoom_one();
  check_zoom();
}

void check_zoom()
{
  char s[8];
  snprintf(s, sizeof(s), "%2.3f", gui->view->zoom);
  gui->zoom->value(s);
  gui->zoom->redraw();
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

void check_paint_size(Widget *widget, void *var)
{
  Brush *brush = Brush::main;

  int size = brush_sizes[*(int *)var];
  int shape = gui->paint_shape->var;

  brush->make(shape, size);
  gui->paint_brush->bitmap->clear(makecol(255, 255, 255));
  int i;
  for(i = 0; i < Brush::main->solid_count; i++)
    gui->paint_brush->bitmap->setpixel_solid(48 + brush->solidx[i], 48 + brush->solidy[i], makecol(0, 0, 0), 0);
  gui->paint_brush->redraw();
}

void check_paint_shape(Widget *widget, void *var)
{
  gui->paint_size->do_callback();
}

void check_paint_stroke(Widget *widget, void *var)
{
  gui->view->tool->stroke->type = *(int *)var;
}

void check_airbrush_size(Widget *widget, void *var)
{
  Brush *brush = Brush::main;

  int size = brush_sizes[*(int *)var];
  int shape = gui->airbrush_shape->var;

  brush->make(shape, size);
  gui->airbrush_brush->bitmap->clear(makecol(255, 255, 255));
  int i;
  for(i = 0; i < Brush::main->solid_count; i++)
    gui->airbrush_brush->bitmap->setpixel_solid(48 + brush->solidx[i], 48 + brush->solidy[i], makecol(0, 0, 0), 0);
  gui->airbrush_brush->redraw();
}

void check_airbrush_shape(Widget *widget, void *var)
{
  gui->airbrush_size->do_callback();
}

void check_airbrush_stroke(Widget *widget, void *var)
{
  gui->view->tool->stroke->type = *(int *)var;
}

void check_airbrush_edge(Widget *widget, void *var)
{
  Brush::main->edge = *(int *)var;
}

void check_airbrush_smooth(Widget *widget, void *var)
{
  Brush::main->smooth = *(int *)var;
}

void check_pixelart_brush(Widget *widget, void *var)
{
  int xpos = *(int *)var % 6;
  int ypos = *(int *)var / 6;
  xpos *= 16;
  ypos *= 16;
  int x, y;

  Map *map = new Map(96, 96);
  map->clear(0);

  for(y = 0; y < 16; y++)
  {
    for(x = 0; x < 16; x++)
    {
      if(widget->bitmap->getpixel(xpos + x, ypos + y) == makecol(0, 0, 0))
      {
        map->setpixel(41 + x, 41 + y, 255);
      }
    }
  }

  Brush::main->solid_count = 0;
  Brush::main->hollow_count = 0;

  for(y = 0; y < 96; y++)
  {
    for(x = 0; x < 96; x++)
    {
      if(map->getpixel(x, y))
      {
        Brush::main->solidx[Brush::main->solid_count] = x - 48;
        Brush::main->solidy[Brush::main->solid_count] = y - 48;
        Brush::main->hollowx[Brush::main->hollow_count] = x - 48;
        Brush::main->hollowy[Brush::main->hollow_count] = y - 48;
        Brush::main->solid_count++;
        Brush::main->hollow_count++;
      }
    }
  }

  Brush::main->size = 16;

  delete map;

//  gui->paint_brush->bitmap->clear(makecol(255, 255, 255));
//  int i;
//  for(i = 0; i < Brush::main->solid_count; i++)
//    gui->paint_brush->bitmap->setpixel_solid(48 + brush->solidx[i], 48 + brush->solidy[i], makecol(0, 0, 0), 0);
}

void check_pixelart_stroke(Widget *widget, void *var)
{
  gui->view->tool->stroke->type = *(int *)var;
}

void check_pixelart_pattern(Widget *widget, void *var)
{
  int x, y;
  int xpos = *(int *)var % 3;
  int ypos = *(int *)var / 3;

  xpos *= 32;
  ypos *= 32;

  for(y = 0; y < 8; y++)
  {
    for(x = 0; x < 8; x++)
    {
      PixelArt::pattern->setpixel(x, y, widget->bitmap->getpixel(xpos + x * 4, ypos + y * 4), 0);
    }
  }
}

void check_pixelart_lock(Widget *widget, void *var)
{
  PixelArt::lock = *(int *)var;  
}

void check_pixelart_invert(Widget *widget, void *var)
{
  PixelArt::invert = *(int *)var;  
}

void check_stump_size(Widget *widget, void *var)
{
  Brush *brush = Brush::main;

  int size = brush_sizes[*(int *)var];
  int shape = gui->stump_shape->var;

  brush->make(shape, size);
  gui->stump_brush->bitmap->clear(makecol(255, 255, 255));
  int i;
  for(i = 0; i < Brush::main->solid_count; i++)
    gui->stump_brush->bitmap->setpixel_solid(48 + brush->solidx[i], 48 + brush->solidy[i], makecol(0, 0, 0), 0);
  gui->stump_brush->redraw();
}

void check_stump_shape(Widget *widget, void *var)
{
  gui->stump_size->do_callback();
}

void check_stump_stroke(Widget *widget, void *var)
{
  gui->view->tool->stroke->type = *(int *)var;
}

void check_stump_amount(Widget *widget, void *var)
{ 
  gui->view->tool->amount = 255 - *(int *)var * 2.685;
  if(gui->view->tool->amount > 252)
    gui->view->tool->amount = 252;
}

void check_tool(Widget *widget, void *var)
{
  gui->paint->hide();
  gui->airbrush->hide();
  gui->pixelart->hide();
  gui->stump->hide();
  gui->crop->hide();
  gui->getcolor->hide();
  gui->offset->hide();

  switch(*(int *)var)
  {
    case 0:
      gui->view->tool = Tool::paint;
      gui->paint_brush->do_callback();
      gui->paint_shape->do_callback();
      gui->paint->show();
      break;
    case 1:
      gui->view->tool = Tool::airbrush;
      gui->airbrush_brush->do_callback();
      gui->airbrush_shape->do_callback();
      gui->airbrush->show();
      break;
    case 2:
      gui->view->tool = Tool::pixelart;
      gui->pixelart_brush->do_callback();
      gui->pixelart_pattern->do_callback();
      gui->pixelart->show();
      break;
    case 3:
      gui->view->tool = Tool::stump;
      gui->stump_brush->do_callback();
      gui->stump_shape->do_callback();
      gui->stump_amount->do_callback();
      gui->stump->show();
      break;
    case 4:
      gui->view->tool = Tool::crop;
      gui->crop->show();
      break;
    case 5:
      gui->view->tool = Tool::getcolor;
      gui->getcolor->show();
      break;
    case 6:
      gui->view->tool = Tool::offset;
      gui->offset->show();
      break;
  }

//  gui->view->tool->reset();
}

void check_color(Widget *widget, void *var)
{
  int pos = gui->hue->var;
  int mx = pos % 96;
  int my = pos / 96;

  float mouse_angle = atan2f(my - 48, mx - 48);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = gui->sat->var * 2.685;
  int v = gui->val->var * 2.685;

  int r, g, b;

  Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
  Brush::main->color = makecol(r, g, b);
  Brush::main->trans = gui->trans->var * 2.685;
//  Brush::main->blend = gui->blend->var;
  Brush::main->blend = gui->blend->value();

  int i;
  int lastx1 = 48 + 40;
  int lasty1 = 48;
  int lastx2 = 48 + 20;
  int lasty2 = 48;
  int px[4];
  int py[4];

  gui->hue->bitmap->clear((Fl::get_color(FL_BACKGROUND_COLOR) >> 8) | 0xFF000000);
  gui->sat->bitmap->clear(0xFF000000);
  gui->val->bitmap->clear(0xFF000000);
  gui->color->bitmap->clear(0xFF000000);

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
    Blend::hsv_to_rgb(h, i * 2.685, v, &r, &g, &b);
    gui->sat->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
    Blend::hsv_to_rgb(h, s, i * 2.685, &r, &g, &b);
    gui->val->bitmap->vline(0, i, 23, makecol(r, g, b), 0);
  }

  gui->hue->redraw();
  gui->sat->redraw();
  gui->val->redraw();

  gui->color->bitmap->rectfill(0, 0, gui->color->bitmap->w, gui->color->bitmap->h, Brush::main->color, 0);
  gui->color->redraw();
}

void check_hue(Widget *widget, void *var)
{
  check_color(0, 0);
}

void check_sat(Widget *widget, void *var)
{
  check_color(0, 0);
}

void check_val(Widget *widget, void *var)
{
  check_color(0, 0);
}

void check_trans(Widget *widget, void *var)
{
  check_color(0, 0);
}

void check_blend(Widget *widget, void *var)
{
  check_color(0, 0);
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
  gui->view->tool->stroke->origin = *(int *)var;
}

void check_constrain(Widget *widget, void *var)
{
  gui->view->tool->stroke->constrain = *(int *)var;
}

void check_crop()
{
  char s[8];

  int overscroll = Bitmap::main->overscroll;

  int x = gui->view->tool->beginx - overscroll;
  int y = gui->view->tool->beginy - overscroll;
  int w = (gui->view->tool->lastx - gui->view->tool->beginx) + 1;
  int h = (gui->view->tool->lasty - gui->view->tool->beginy) + 1;

  snprintf(s, sizeof(s), "%d", x);
  gui->crop_x->value(s);
  gui->crop_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  gui->crop_y->value(s);
  gui->crop_y->redraw();

  snprintf(s, sizeof(s), "%d", w);
  gui->crop_w->value(s);
  gui->crop_w->redraw();

  snprintf(s, sizeof(s), "%d", h);
  gui->crop_h->value(s);
  gui->crop_h->redraw();
}

void check_rgba()
{
  gui->set_menu_item("Mode/RGBA");
  gui->clear_menu_item("Mode/Indexed");
  gui->view->mode = 0;
  gui->view->draw_main(1);
}

void check_indexed()
{
  gui->clear_menu_item("Mode/RGBA");
  gui->set_menu_item("Mode/Indexed");
  gui->view->mode = 1;
  gui->view->draw_main(1);
}

