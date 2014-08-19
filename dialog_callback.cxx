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

static int file_exists(const char *s)
{
  FILE *temp = fopen(s, "r");

  if(temp)
  {
    fclose(temp);
    return 1;
  }

  return 0;
}

void jpeg_quality_close_callback(Fl_Widget *widget, void *)
{
  // needed to prevent quality dialog from being closed
  // by the window manager
}

void show_jpeg_quality()
{
  dialog->jpeg_quality->show();

  while(1)
  {
    Fl_Widget *action = Fl::readqueue();

    if(!action)
      Fl::wait();
    else if(action == dialog->jpeg_quality_ok)
    {
      char s[8];
      int q = atoi(dialog->jpeg_quality_amount->value());

      if(q < 1)
      {
        snprintf(s, sizeof(s), "%d", 1);
        dialog->jpeg_quality_amount->value(s);
      }
      else if(q > 100)
      {
        snprintf(s, sizeof(s), "%d", 100);
        dialog->jpeg_quality_amount->value(s);
      }
      else
      {
        dialog->jpeg_quality->hide();
        break;
      }
    }
  }
}

void show_progress(float step)
{
  progress_value = 0;
  progress_step = 100.0 / step;
  dialog->progress->show();
}

void update_progress()
{
  dialog->progress_bar->value(progress_value);
  char percent[16];
  sprintf(percent, "%d%%", (int)progress_value);
  dialog->progress_bar->label(percent);
  Fl::check();
  progress_value += progress_step;
}

void hide_progress()
{
  dialog->progress->hide();
}

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

  FILE *in = fopen(fn, "r");
  if(!in)
  {
    delete fc;
    return;
  }

  fclose(in);

  Palette::main->load(fn);
  Palette::main->draw(gui->palette);
  gui->palette->var = 0;
  gui->palette->redraw();

  delete fc;
}

void show_save_palette()
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Palette");
  fc->filter("GIMP Palette\t*.gpl\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
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

  if(file_exists(fn))
  {
    fl_message_title("Replace File?");
    if(fl_choice("Replace File?", "No", "Yes", NULL) == 0)
      return;
  }

  if(strcasecmp(ext, ".gpl") != 0)
  {
    delete fc;
    return;
  }

  FILE *out = fopen(fn, "w");
  if(!out)
  {
    delete fc;
    return;
  }

  fclose(out);

  Palette::main->save(fn);

  delete fc;
}
void show_editor()
{
  Palette::main->draw(dialog->editor_palette);
  do_editor_set_hsv();
  dialog->editor->show();
  undo = 0;
  ramp_begin = 0;
  ramp_started = 0;
}

void hide_editor()
{
  Palette::main->fill_lookup();
  dialog->editor->hide();
}

void do_editor_palette(Widget *widget, void *var)
{
  int i;
  int begin, end;
  Palette *pal = Palette::main;

  if(ramp_started > 0)
  {
    do_editor_store_undo();
    begin = ramp_begin;
    end = *(int *)var;
    if(begin > end)
      SWAP(begin, end);
    int num = end - begin;

    if(ramp_started == 1)
    {
      // rgb ramp
      int c1 = pal->data[begin];
      int c2 = pal->data[end];
      double stepr = (double)(getr(c2) - getr(c1)) / num;
      double stepg = (double)(getg(c2) - getg(c1)) / num;
      double stepb = (double)(getb(c2) - getb(c1)) / num;
      double r = getr(c1);
      double g = getg(c1);
      double b = getb(c1);

      for(i = begin; i < end; i++)
      {
        pal->data[i] = makecol(r, g, b);
        r += stepr;
        g += stepg;
        b += stepb;
      }

      dialog->editor_rgb_ramp->value(0);
      dialog->editor_rgb_ramp->redraw();
    }
    else if(ramp_started == 2)
    {
      // hsv ramp
      int c1 = pal->data[begin];
      int c2 = pal->data[end];
      int h1, s1, v1;
      int h2, s2, v2;
      Blend::rgb_to_hsv(getr(c1), getg(c1), getb(c1), &h1, &s1, &v1);
      Blend::rgb_to_hsv(getr(c2), getg(c2), getb(c2), &h2, &s2, &v2);
      double steph = (double)(h2 - h1) / num;
      double steps = (double)(s2 - s1) / num;
      double stepv = (double)(v2 - v1) / num;
      int r, g, b;
      double h = h1;
      double s = s1;
      double v = v1;

      for(i = begin; i < end; i++)
      {
        Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
        pal->data[i] = makecol(r, g, b);
        h += steph;
        s += steps;
        v += stepv;
      }

      dialog->editor_hsv_ramp->value(0);
      dialog->editor_hsv_ramp->redraw();
    }

    ramp_started = 0;
    Palette::main->draw(dialog->editor_palette);
    Palette::main->draw(gui->palette);

    return;
  }

  check_palette(widget, var);
  ramp_begin = *(int *)var;
  do_editor_set_hsv();
  int h, s, v;
  int color = Brush::main->color;
  Blend::rgb_to_hsv(getr(color), getg(color), getb(color), &h, &s, &v);
  dialog->editor_h->var = h / 6;
  dialog->editor_sv->var = s + 256 * v;
  dialog->editor_h->redraw();
  dialog->editor_sv->redraw();
}

void do_editor_set_hsv()
{
  int x , y;
  int r = 0, g = 0, b = 0;
  int h = 0, s = 0, v = 0;
  int color = Brush::main->color;

  Blend::rgb_to_hsv(getr(color), getg(color), getb(color), &h, &s, &v);

  dialog->editor_h->bitmap->clear(makecol(0, 0, 0));
  dialog->editor_sv->bitmap->clear(makecol(0, 0, 0));

  for(y = 0; y < 256; y++)
  {
    for(x = 0; x < 256; x++)
    {
      Blend::hsv_to_rgb(h, x, y, &r, &g, &b);
      dialog->editor_sv->bitmap->setpixel_solid(x, y, makecol(r, g, b), 0);
    }

    Blend::hsv_to_rgb(y * 6, 255, 255, &r, &g, &b);
    dialog->editor_h->bitmap->hline(0, y, 23, makecol(r, g, b), 0);
  }

  x = dialog->editor_sv->var & 255;
  y = dialog->editor_sv->var / 256;
  if(x < 4)
    x = 4;
  if(y < 4)
    y = 4;
  if(x > 251)
    x = 251;
  if(y > 251)
    y = 251;

  dialog->editor_sv->bitmap->xor_rect(x - 4, y - 4, x + 4, y + 4);

  dialog->editor_h->redraw();
  dialog->editor_sv->redraw();

  dialog->editor_color->bitmap->clear(Brush::main->color);
  dialog->editor_color->redraw();
}

void do_editor_get_hsv()
{
  int h = dialog->editor_h->var * 6;
  int s = dialog->editor_sv->var & 255;
  int v = dialog->editor_sv->var / 256;
  int r, g, b;

  Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
  Brush::main->color = makecol(r, g, b);

  update_color(Brush::main->color);
  do_editor_set_hsv();
}

void do_editor_insert()
{
  do_editor_store_undo();
  Palette::main->insert_color(Brush::main->color, dialog->editor_palette->var);
  Palette::main->draw(dialog->editor_palette);
  Palette::main->draw(gui->palette);
//  dialog->editor_palette->var = Palette::main->max;
  dialog->editor_palette->do_callback();
//  gui->palette->do_callback();
}

void do_editor_delete()
{
  do_editor_store_undo();
  Palette::main->delete_color(dialog->editor_palette->var);
  Palette::main->draw(dialog->editor_palette);
  Palette::main->draw(gui->palette);
  if(dialog->editor_palette->var > Palette::main->max - 1)
    dialog->editor_palette->var = Palette::main->max - 1;
  dialog->editor_palette->do_callback();
//  gui->palette->do_callback();
}

void do_editor_replace()
{
  do_editor_store_undo();
  Palette::main->replace_color(Brush::main->color, dialog->editor_palette->var);
  Palette::main->draw(dialog->editor_palette);
  Palette::main->draw(gui->palette);
  dialog->editor_palette->var = Palette::main->max;
  dialog->editor_palette->do_callback();
//  gui->palette->do_callback();
}

void do_editor_store_undo()
{
  Palette::main->copy(Palette::undo);
  undo = 1;
}

void do_editor_get_undo()
{
  if(undo)
  {
    Palette::undo->copy(Palette::main);
    undo = 0;
    Palette::main->draw(dialog->editor_palette);
    Palette::main->draw(gui->palette);
    dialog->editor_palette->do_callback();
    gui->palette->do_callback();
  }
}

void do_editor_rgb_ramp()
{
  if(!ramp_started)
  {
    dialog->editor_rgb_ramp->value(1);
    dialog->editor_rgb_ramp->redraw();
    ramp_started = 1;
  }
}

void do_editor_hsv_ramp()
{
  if(!ramp_started)
  {
    dialog->editor_hsv_ramp->value(1);
    dialog->editor_hsv_ramp->redraw();
    ramp_started = 2;
  }
}

