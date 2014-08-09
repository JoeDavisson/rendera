/*
Copyright (c) 2014 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "rendera.h"

extern Gui *gui;
extern Dialog *dialog;

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
  snprintf(s, sizeof(s), "%d", Bitmap::main->w - Bitmap::main->overscroll * 2);
  dialog->new_image_width->value(s);
  snprintf(s, sizeof(s), "%d", Bitmap::main->h - Bitmap::main->overscroll * 2);
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

  delete Bitmap::main;
  int overscroll = Bitmap::main->overscroll;
  Bitmap::main = new Bitmap(w, h, overscroll,
                            makecol(255, 255, 255), makecol(128, 128, 128));

  delete Map::main;
  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);

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

  quantize(Bitmap::main, colors);
}

void cancel_create_palette()
{
  dialog->create_palette->hide();
}

void show_load_palette()
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Load Palette");
  fc->filter("GIMP Palette\t*.gpl\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_FILE);
  fc->show();

  const char *fn = fc->filename();

  // get file extension
  char ext[16];
  char *p = (char *)fn + strlen(fn) - 1;

  while(p >= fn)
  {
    if(*p == '.')
    {
      strcpy(ext, p);
      break;
    }

    p--;
  }

  if(strcasecmp(ext, ".gpl") != 0)
  {
    delete fc;
    return;
  }

  FILE *in = fl_fopen(fn, "r");
  if(!in)
  {
    delete fc;
    return;
  }

  Palette::main->load(fn);
  Palette::main->draw(gui->palette);
  gui->palette->var = 0;
  gui->palette->redraw();

  delete fc;
}

void show_editor()
{
  Palette::main->draw(dialog->editor_palette);
  do_editor_rgbhsv();
  dialog->editor->show();
}

void hide_editor()
{
  dialog->editor->hide();
}

void do_editor_palette(Widget *widget, void *var)
{
  check_palette(widget, var);
  do_editor_rgbhsv();
}

void do_editor_rgbhsv()
{
  int i;
  int r = 0, g = 0, b = 0;
  int h = 0, s = 0, v = 0;
  int color = Brush::main->color;

  Blend::rgb_to_hsv(getr(color), getg(color), getb(color), &h, &s, &v);

  dialog->editor_r->var = getr(color);
  dialog->editor_g->var = getg(color);
  dialog->editor_b->var = getb(color);
  dialog->editor_h->var = h / 6;
  dialog->editor_s->var = s;
  dialog->editor_v->var = v;

  dialog->editor_r->bitmap->clear(makecol(0, 0, 0));
  dialog->editor_g->bitmap->clear(makecol(0, 0, 0));
  dialog->editor_b->bitmap->clear(makecol(0, 0, 0));
  dialog->editor_h->bitmap->clear(makecol(0, 0, 0));
  dialog->editor_s->bitmap->clear(makecol(0, 0, 0));
  dialog->editor_v->bitmap->clear(makecol(0, 0, 0));

  for(i = 0; i < 256; i++)
  {
    dialog->editor_r->bitmap->hline(0, i, 23, makecol(i, 0, 0), 0);
    dialog->editor_g->bitmap->hline(0, i, 23, makecol(0, i, 0), 0);
    dialog->editor_b->bitmap->hline(0, i, 23, makecol(0, 0, i), 0);

    Blend::hsv_to_rgb(i * 6, 255, 255, &r, &g, &b);
    dialog->editor_h->bitmap->hline(0, i, 23, makecol(r, g, b), 0);
    Blend::hsv_to_rgb(h, i, v, &r, &g, &b);
    dialog->editor_s->bitmap->hline(0, i, 23, makecol(r, g, b), 0);
    Blend::hsv_to_rgb(h, s, i, &r, &g, &b);
    dialog->editor_v->bitmap->hline(0, i, 23, makecol(r, g, b), 0);
  }

  dialog->editor_r->redraw();
  dialog->editor_g->redraw();
  dialog->editor_b->redraw();
  dialog->editor_h->redraw();
  dialog->editor_s->redraw();
  dialog->editor_v->redraw();

  dialog->editor_color->bitmap->clear(Brush::main->color);
  dialog->editor_color->redraw();
}

void do_editor_get_rgb()
{
  int r = dialog->editor_r->var;
  int g = dialog->editor_g->var;
  int b = dialog->editor_b->var;

  Brush::main->color = makecol(r, g, b);

  update_color(Brush::main->color);
  do_editor_rgbhsv();
}

void do_editor_get_hsv()
{
  int h = dialog->editor_h->var * 6;
  int s = dialog->editor_s->var;
  int v = dialog->editor_v->var;
  int r, g, b;

  Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
  Brush::main->color = makecol(r, g, b);

  update_color(Brush::main->color);
  do_editor_rgbhsv();
}

void do_editor_insert()
{
  Palette::main->insert_color(Brush::main->color, dialog->editor_palette->var);
  Palette::main->draw(dialog->editor_palette);
  Palette::main->draw(gui->palette);
  dialog->editor_palette->do_callback();
  gui->palette->do_callback();
}

void do_editor_delete()
{
  Palette::main->delete_color(dialog->editor_palette->var);
  Palette::main->draw(dialog->editor_palette);
  Palette::main->draw(gui->palette);
  dialog->editor_palette->do_callback();
  gui->palette->do_callback();
}

void do_editor_replace()
{
  Palette::main->replace_color(Brush::main->color, dialog->editor_palette->var);
  Palette::main->draw(dialog->editor_palette);
  Palette::main->draw(gui->palette);
  dialog->editor_palette->do_callback();
  gui->palette->do_callback();
}

