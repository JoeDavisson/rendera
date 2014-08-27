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

#include "Dialog.h"
#include "Bitmap.h"
#include "Brush.h"
#include "Palette.h"
#include "Blend.h"
#include "Map.h"
#include "Field.h"
#include "Widget.h"
#include "Separator.h"
#include "Gui.h"
#include "View.h"
#include "Quantize.h"

Fl_Double_Window *Dialog::jpeg_quality;
Field *Dialog::jpeg_quality_amount;
Fl_Button *Dialog::jpeg_quality_ok;

Fl_Double_Window *Dialog::progress;
Fl_Progress *Dialog::progress_bar;

Fl_Double_Window *Dialog::about;
Widget *Dialog::about_logo;
Fl_Button *Dialog::about_ok;

Fl_Double_Window *Dialog::new_image;
Field *Dialog::new_image_width;
Field *Dialog::new_image_height;
Fl_Button *Dialog::new_image_ok;
Fl_Button *Dialog::new_image_cancel;

Fl_Double_Window *Dialog::create_palette;
Field *Dialog::create_palette_colors;
Fl_Button *Dialog::create_palette_ok;
Fl_Button *Dialog::create_palette_cancel;

Fl_Double_Window *Dialog::editor;
Widget *Dialog::editor_h;
Widget *Dialog::editor_sv;
Fl_Button *Dialog::editor_insert;
Fl_Button *Dialog::editor_delete;
Fl_Button *Dialog::editor_replace;
Fl_Button *Dialog::editor_undo;
Fl_Button *Dialog::editor_rgb_ramp;
Fl_Button *Dialog::editor_hsv_ramp;
Widget *Dialog::editor_palette;
Widget *Dialog::editor_color;
Fl_Button *Dialog::editor_done;

static int undo;
static int ramp_begin;
static int ramp_started;
static float progress_value;
static float progress_step;

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

void Dialog::init()
{
  // JPEG quality
  jpeg_quality = new Fl_Double_Window(200, 80, "JPEG Quality");
  jpeg_quality->callback(jpegQualityCloseCallback);
  jpeg_quality_amount = new Field(jpeg_quality, 80, 8, 72, 24, "Quality:", 0);
  jpeg_quality_amount->value("95");
  new Separator(jpeg_quality, 2, 40, 196, 2, "");
  jpeg_quality_ok = new Fl_Button(128, 48, 64, 24, "OK");
  // no callback for ok button, see show_jpeg_quality called from File.cxx
  jpeg_quality->set_modal();
  jpeg_quality->end();

  // progress
  progress = new Fl_Double_Window(272, 80, "Progress");
  progress_bar = new Fl_Progress(8, 8, 256, 64);
  progress_bar->minimum(0);
  progress_bar->maximum(100);
  progress_bar->color(0);
  progress_bar->selection_color(0x88CC8800);
  progress_bar->labelcolor(0xFFFFFF00);
  progress->set_modal();
  progress->end();

  // about
  about = new Fl_Double_Window(336, 112, "About");
  about_logo = new Widget(about, 8, 8, 320, 64, "Logo", "data/logo_large.png", 0, 0, 0);
  about_ok = new Fl_Button(336 / 2 - 32, 80, 64, 24, "OK");
  about_ok->callback((Fl_Callback *)hideAbout);
  about->set_modal();
  about->end(); 

  // new image
  new_image = new Fl_Double_Window(200, 112, "New Image");
  new_image_width = new Field(new_image, 88, 8, 72, 24, "Width:", 0);
  new_image_height = new Field(new_image, 88, 40, 72, 24, "Height:", 0);
  new_image_width->maximum_size(8);
  new_image_height->maximum_size(8);
  new_image_width->value("640");
  new_image_height->value("480");
  new Separator(new_image, 2, 72, 196, 2, "");
  new_image_ok = new Fl_Button(56, 80, 64, 24, "OK");
  new_image_ok->callback((Fl_Callback *)hideNewImage);
  new_image_cancel = new Fl_Button(128, 80, 64, 24, "Cancel");
  new_image_cancel->callback((Fl_Callback *)cancelNewImage);
  new_image->set_modal();
  new_image->end(); 

  // create palette from image
  create_palette = new Fl_Double_Window(200, 80, "Create Palette");
  create_palette_colors = new Field(create_palette, 80, 8, 72, 24, "Colors:", 0);
  new_image_width->value("256");
  new Separator(new_image, 2, 40, 196, 2, "");
  create_palette_ok = new Fl_Button(56, 48, 64, 24, "OK");
  create_palette_ok->callback((Fl_Callback *)hideCreatePalette);
  create_palette_cancel = new Fl_Button(128, 48, 64, 24, "Cancel");
  create_palette_cancel->callback((Fl_Callback *)cancelCreatePalette);
  create_palette->set_modal();
  create_palette->end(); 

  // palette editor
  editor = new Fl_Double_Window(608, 312, "Palette Editor");
  editor_h = new Widget(editor, 8, 8, 24, 256, "Hue", 24, 1, (Fl_Callback *)doEditorGetHsv);
  editor_sv = new Widget(editor, 40, 8, 256, 256, "Saturation/Value", 1, 1, (Fl_Callback *)doEditorGetHsv);
  editor_insert = new Fl_Button(304, 8, 96, 24, "Insert");
  editor_insert->callback((Fl_Callback *)doEditorInsert);
  editor_delete = new Fl_Button(304, 48, 96, 24, "Delete");
  editor_delete->callback((Fl_Callback *)doEditorDelete);
  editor_replace = new Fl_Button(304, 88, 96, 24, "Replace");
  editor_replace->callback((Fl_Callback *)doEditorReplace);
  editor_undo = new Fl_Button(304, 144, 96, 24, "Undo");
  editor_undo->callback((Fl_Callback *)doEditorGetUndo);
  editor_rgb_ramp = new Fl_Button(304, 200, 96, 24, "RGB Ramp");
  editor_rgb_ramp->callback((Fl_Callback *)doEditorRgbRamp);
  editor_hsv_ramp = new Fl_Button(304, 240, 96, 24, "HSV Ramp");
  editor_hsv_ramp->callback((Fl_Callback *)doEditorHsvRamp);
  editor_palette = new Widget(editor, 408, 8, 192, 192, "Palette", 24, 24, (Fl_Callback *)doEditorPalette);
  editor_color = new Widget(editor, 408, 208, 192, 56, "Color", 0, 0, 0);
  new Separator(editor, 2, 272, 604, 2, "");
  editor_done = new Fl_Button(504, 280, 96, 24, "Done");
  editor_done->callback((Fl_Callback *)hideEditor);
  editor->set_modal();
  editor->end(); 
}

void Dialog::jpegQualityCloseCallback(Fl_Widget */* widget */, void *)
{
  // needed to prevent quality dialog from being closed
  // by the window manager
}

void Dialog::showJpegQuality()
{
  jpeg_quality->show();

  while(1)
  {
    Fl_Widget *action = Fl::readqueue();

    if(!action)
      Fl::wait();
    else if(action == jpeg_quality_ok)
    {
      char s[8];
      int q = atoi(jpeg_quality_amount->value());

      if(q < 1)
      {
        snprintf(s, sizeof(s), "%d", 1);
        jpeg_quality_amount->value(s);
      }
      else if(q > 100)
      {
        snprintf(s, sizeof(s), "%d", 100);
        jpeg_quality_amount->value(s);
      }
      else
      {
        jpeg_quality->hide();
        break;
      }
    }
  }
}

void Dialog::showProgress(float step)
{
  progress_value = 0;
  progress_step = 100.0 / step;
  progress->show();
}

void Dialog::updateProgress()
{
  progress_bar->value(progress_value);
  char percent[16];
  sprintf(percent, "%d%%", (int)progress_value);
  progress_bar->label(percent);
  Fl::check();
  progress_value += progress_step;
}

void Dialog::hideProgress()
{
  progress->hide();
}

void Dialog::showAbout()
{
  about->show();
}

void Dialog::hideAbout()
{
  about->hide();
}

void Dialog::showNewImage()
{
  char s[8];
  snprintf(s, sizeof(s), "%d", Bitmap::main->w - Bitmap::main->overscroll * 2);
  new_image_width->value(s);
  snprintf(s, sizeof(s), "%d", Bitmap::main->h - Bitmap::main->overscroll * 2);
  new_image_height->value(s);
  new_image->show();
}

void Dialog::hideNewImage()
{
  char s[8];

  int w = atoi(new_image_width->value());
  int h = atoi(new_image_height->value());

  if(w < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    new_image_width->value(s);
    return;
  }

  if(h < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    new_image_height->value(s);
    return;
  }

  if(w > 10000)
  {
    snprintf(s, sizeof(s), "%d", 10000);
    new_image_width->value(s);
    return;
  }

  if(h > 10000)
  {
    snprintf(s, sizeof(s), "%d", 10000);
    new_image_height->value(s);
    return;
  }

  new_image->hide();

  delete Bitmap::main;
  int overscroll = Bitmap::main->overscroll;
  Bitmap::main = new Bitmap(w, h, overscroll,
                            makecol(255, 255, 255), makecol(128, 128, 128));

  delete Map::main;
  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);

  Gui::view->ox = 0;
  Gui::view->oy = 0;
  Gui::view->zoom_fit(0);
  Gui::view->draw_main(1);
}

void Dialog::cancelNewImage()
{
  new_image->hide();
}

void Dialog::showCreatePalette()
{
  char s[8];
  snprintf(s, sizeof(s), "%d", Palette::main->max);
  create_palette_colors->value(s);
  create_palette->show();
}

void Dialog::hideCreatePalette()
{
  char s[8];

  int colors = atoi(create_palette_colors->value());

  if(colors < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    create_palette_colors->value(s);
    return;
  }

  if(colors > 256)
  {
    snprintf(s, sizeof(s), "%d", 256);
    create_palette_colors->value(s);
    return;
  }

  create_palette->hide();

  Quantize::pca(Bitmap::main, colors);
}

void Dialog::cancelCreatePalette()
{
  create_palette->hide();
}

void Dialog::showLoadPalette()
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
  Palette::main->draw(Gui::palette);
  Gui::palette->var = 0;
  Gui::palette->redraw();

  delete fc;
}

void Dialog::showSavePalette()
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
void Dialog::showEditor()
{
  Palette::main->draw(editor_palette);
  doEditorSetHsvSliders();
  doEditorSetHsv();
  editor->show();
  undo = 0;
  ramp_begin = 0;
  ramp_started = 0;
}

void Dialog::hideEditor()
{
  Palette::main->fill_lookup();
  editor->hide();
}

void Dialog::doEditorPalette(Widget *widget, void *var)
{
  int i;
  int begin, end;
  Palette *pal = Palette::main;

  if(ramp_started > 0)
  {
    doEditorStoreUndo();
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

      editor_rgb_ramp->value(0);
      editor_rgb_ramp->redraw();
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

      editor_hsv_ramp->value(0);
      editor_hsv_ramp->redraw();
    }

    ramp_started = 0;
    Palette::main->draw(editor_palette);
    Palette::main->draw(Gui::palette);

    return;
  }

  Gui::checkPalette(widget, var);
  ramp_begin = *(int *)var;
  doEditorSetHsvSliders();
  doEditorSetHsv();
}

void Dialog::doEditorSetHsv()
{
  int x , y;
  int r = 0, g = 0, b = 0;
  int h = 0, s = 0, v = 0;
  int color = Brush::main->color;

  Blend::rgb_to_hsv(getr(color), getg(color), getb(color), &h, &s, &v);

  editor_h->bitmap->clear(makecol(0, 0, 0));
  editor_sv->bitmap->clear(makecol(0, 0, 0));

  for(y = 0; y < 256; y++)
  {
    for(x = 0; x < 256; x++)
    {
      Blend::hsv_to_rgb(h, x, y, &r, &g, &b);
      editor_sv->bitmap->setpixel_solid(x, y, makecol(r, g, b), 0);
    }

    Blend::hsv_to_rgb(y * 6, 255, 255, &r, &g, &b);
    editor_h->bitmap->hline(0, y, 23, makecol(r, g, b), 0);
  }

  x = editor_sv->var & 255;
  y = editor_sv->var / 256;

  if(x < 4)
    x = 4;
  if(y < 4)
    y = 4;
  if(x > 251)
    x = 251;
  if(y > 251)
    y = 251;

  editor_sv->bitmap->xor_rect(x - 4, y - 4, x + 4, y + 4);

  editor_h->redraw();
  editor_sv->redraw();

  editor_color->bitmap->clear(Brush::main->color);
  editor_color->redraw();
}

void Dialog::doEditorSetHsvSliders()
{
  int h, s, v;
  int color = Brush::main->color;
  Blend::rgb_to_hsv(getr(color), getg(color), getb(color), &h, &s, &v);
  editor_h->var = h / 6;
  editor_sv->var = s + 256 * v;
  editor_h->redraw();
  editor_sv->redraw();
}

void Dialog::doEditorGetHsv()
{
  int h = editor_h->var * 6;
  int s = editor_sv->var & 255;
  int v = editor_sv->var / 256;
  int r, g, b;

  Blend::hsv_to_rgb(h, s, v, &r, &g, &b);
  Brush::main->color = makecol(r, g, b);

  Gui::updateColor(Brush::main->color);
  doEditorSetHsv();
}

void Dialog::doEditorInsert()
{
  doEditorStoreUndo();
  Palette::main->insert_color(Brush::main->color, editor_palette->var);
  Palette::main->draw(editor_palette);
  Palette::main->draw(Gui::palette);
  editor_palette->do_callback();
}

void Dialog::doEditorDelete()
{
  doEditorStoreUndo();
  Palette::main->delete_color(editor_palette->var);
  Palette::main->draw(editor_palette);
  Palette::main->draw(Gui::palette);
  if(editor_palette->var > Palette::main->max - 1)
    editor_palette->var = Palette::main->max - 1;
  editor_palette->do_callback();
}

void Dialog::doEditorReplace()
{
  doEditorStoreUndo();
  Palette::main->replace_color(Brush::main->color, editor_palette->var);
  Palette::main->draw(editor_palette);
  Palette::main->draw(Gui::palette);
  editor_palette->do_callback();
}

void Dialog::doEditorStoreUndo()
{
  Palette::main->copy(Palette::undo);
  undo = 1;
}

void Dialog::doEditorGetUndo()
{
  if(undo)
  {
    Palette::undo->copy(Palette::main);
    undo = 0;
    Palette::main->draw(editor_palette);
    Palette::main->draw(Gui::palette);
    editor_palette->do_callback();
    Gui::palette->do_callback();
  }
}

void Dialog::doEditorRgbRamp()
{
  if(!ramp_started)
  {
    editor_rgb_ramp->value(1);
    editor_rgb_ramp->redraw();
    ramp_started = 1;
  }
}

void Dialog::doEditorHsvRamp()
{
  if(!ramp_started)
  {
    editor_hsv_ramp->value(1);
    editor_hsv_ramp->redraw();
    ramp_started = 2;
  }
}

