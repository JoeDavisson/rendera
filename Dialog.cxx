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

#include "Dialog.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Palette.H"
#include "Blend.H"
#include "Map.H"
#include "Field.H"
#include "Widget.H"
#include "Separator.H"
#include "Gui.H"
#include "View.H"
#include "Quantize.H"

namespace About
{
  // about
  Fl_Double_Window *dialog;
  Widget *logo;
  Fl_Button *ok;

  void begin()
  {
    dialog->show();
  }

  void end()
  {
    dialog->hide();
  }
}

namespace JpegQuality
{
  Fl_Double_Window *dialog;
  Field *amount;
  Fl_Button *ok;

  void closeCallback(Fl_Widget *, void *)
  {
    // needed to prevent dialog from being closed by window manager
  }

  void begin()
  {
    dialog->show();

    while(1)
    {
      Fl_Widget *action = Fl::readqueue();

      if(!action)
      {
        Fl::wait();
      }
      else if(action == ok)
      {
        char s[8];
        int q = atoi(amount->value());

        if(q < 1)
        {
          snprintf(s, sizeof(s), "%d", 1);
          amount->value(s);
        }
        else if(q > 100)
        {
          snprintf(s, sizeof(s), "%d", 100);
          amount->value(s);
        }
        else
        {
          dialog->hide();
          break;
        }
      }
    }
  }
}

namespace Progress
{
  Fl_Double_Window *dialog;
  Fl_Progress *bar;
  float value;
  float step;
}

namespace NewImage
{
  Fl_Double_Window *dialog;
  Field *width;
  Field *height;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Bitmap::main->w - Bitmap::main->overscroll * 2);
    width->value(s);
    snprintf(s, sizeof(s), "%d", Bitmap::main->h - Bitmap::main->overscroll * 2);
    height->value(s);
    dialog->show();
  }

  void end()
  {
    char s[8];

    int w = atoi(width->value());
    int h = atoi(height->value());

    if(w < 1)
    {
      snprintf(s, sizeof(s), "%d", 1);
      width->value(s);
      return;
    }

    if(h < 1)
    {
      snprintf(s, sizeof(s), "%d", 1);
      height->value(s);
      return;
    }

    if(w > 10000)
    {
      snprintf(s, sizeof(s), "%d", 10000);
      width->value(s);
      return;
    }

    if(h > 10000)
    {
      snprintf(s, sizeof(s), "%d", 10000);
      height->value(s);
      return;
    }

    dialog->hide();

    int overscroll = Bitmap::main->overscroll;
    delete Bitmap::main;
    Bitmap::main = new Bitmap(w, h, overscroll,
                              make_rgb(255, 255, 255), make_rgb(128, 128, 128));

    delete Map::main;
    Map::main = new Map(Bitmap::main->w, Bitmap::main->h);

    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    Gui::getView()->zoomFit(0);
    Gui::getView()->drawMain(1);
  }

  void quit()
  {
    dialog->hide();
  }
}

namespace CreatePalette
{
  Fl_Double_Window *dialog;
  Field *colors;
  Fl_Button *ok;
  Fl_Button *cancel;

  void begin()
  {
    char s[8];
    snprintf(s, sizeof(s), "%d", Palette::main->max);
    colors->value(s);
    dialog->show();
  }

  void end()
  {
    char s[8];

    int c = atoi(colors->value());

    if(c < 1)
    {
      snprintf(s, sizeof(s), "%d", 1);
      colors->value(s);
      return;
    }

    if(c > 256)
    {
      snprintf(s, sizeof(s), "%d", 256);
      colors->value(s);
      return;
    }

    dialog->hide();
    Quantize::pca(Bitmap::main, c);
  }

  void quit()
  {
    dialog->hide();
  }
}

namespace
{
  Fl_Double_Window *editor;
  Widget *editor_h;
  Widget *editor_sv;
  Fl_Button *editor_insert;
  Fl_Button *editor_delete;
  Fl_Button *editor_replace;
  Fl_Button *editor_undo;
  Fl_Button *editor_rgb_ramp;
  Fl_Button *editor_hsv_ramp;
  Widget *editor_palette;
  Widget *editor_color;
  Fl_Button *editor_done;

  int undo;
  int ramp_begin;
  int ramp_started;
  int oldsvx, oldsvy;

  int file_exists(const char *s)
  {
    FILE *temp = fopen(s, "r");

    if(temp)
    {
      fclose(temp);
      return 1;
    }

    return 0;
  }
}

void Dialog::init()
{
  // about
  About::dialog = new Fl_Double_Window(336, 112, "About");
  About::logo = new Widget(About::dialog, 8, 8, 320, 64, "Logo", "data/logo_large.png", 0, 0, 0);
  About::ok = new Fl_Button(336 / 2 - 32, 80, 64, 24, "OK");
  About::ok->callback((Fl_Callback *)About::end);
  About::dialog->set_modal();
  About::dialog->end(); 

  // JPEG quality
  JpegQuality::dialog = new Fl_Double_Window(200, 80, "JPEG Quality");
  JpegQuality::dialog->callback(JpegQuality::closeCallback);
  JpegQuality::amount = new Field(JpegQuality::dialog, 80, 8, 72, 24, "Quality:", 0);
  JpegQuality::amount->value("95");
  new Separator(JpegQuality::dialog, 2, 40, 196, 2, "");
  JpegQuality::ok = new Fl_Button(128, 48, 64, 24, "OK");
  JpegQuality::dialog->set_modal();
  JpegQuality::dialog->end();

  // progress
  Progress::dialog = new Fl_Double_Window(272, 80, "Progress");
  Progress::bar = new Fl_Progress(8, 8, 256, 64);
  Progress::bar->minimum(0);
  Progress::bar->maximum(100);
  Progress::bar->color(0);
  Progress::bar->selection_color(0x88CC8800);
  Progress::bar->labelcolor(0xFFFFFF00);
  Progress::dialog->set_modal();
  Progress::dialog->end();

  // new image
  NewImage::dialog = new Fl_Double_Window(200, 112, "New Image");
  NewImage::width = new Field(NewImage::dialog, 88, 8, 72, 24, "Width:", 0);
  NewImage::height = new Field(NewImage::dialog, 88, 40, 72, 24, "Height:", 0);
  NewImage::width->maximum_size(8);
  NewImage::height->maximum_size(8);
  NewImage::width->value("640");
  NewImage::height->value("480");
  new Separator(NewImage::dialog, 2, 72, 196, 2, "");
  NewImage::ok = new Fl_Button(56, 80, 64, 24, "OK");
  NewImage::ok->callback((Fl_Callback *)NewImage::end);
  NewImage::cancel = new Fl_Button(128, 80, 64, 24, "Cancel");
  NewImage::cancel->callback((Fl_Callback *)NewImage::quit);
  NewImage::dialog->set_modal();
  NewImage::dialog->end(); 

  // create palette from image
  CreatePalette::dialog = new Fl_Double_Window(200, 80, "Create Palette");
  CreatePalette::colors = new Field(CreatePalette::dialog, 80, 8, 72, 24, "Colors:", 0);
  CreatePalette::ok = new Fl_Button(56, 48, 64, 24, "OK");
  CreatePalette::ok->callback((Fl_Callback *)CreatePalette::end);
  CreatePalette::cancel = new Fl_Button(128, 48, 64, 24, "Cancel");
  CreatePalette::cancel->callback((Fl_Callback *)CreatePalette::quit);
  CreatePalette::dialog->set_modal();
  CreatePalette::dialog->end(); 

  // palette editor
  editor = new Fl_Double_Window(608, 312, "Palette Editor");
  editor_h = new Widget(editor, 8, 8, 24, 256, "Hue", 24, 1, (Fl_Callback *)doEditorGetH);
  editor_sv = new Widget(editor, 40, 8, 256, 256, "Saturation/Value", 1, 1, (Fl_Callback *)doEditorGetSV);
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

void Dialog::about()
{
  About::begin();
}

void Dialog::jpegQuality()
{
  JpegQuality::begin();
}

int Dialog::jpegQualityValue()
{
  int quality = atoi(JpegQuality::amount->value());

  if(quality < 1)
    quality = 1;
  if(quality > 100)
    quality = 100;

  return quality;
}

void Dialog::showProgress(float step)
{
  Progress::value = 0;
  Progress::step = 100.0 / step;
  Progress::dialog->show();
}

void Dialog::updateProgress()
{
  Progress::bar->value(Progress::value);
  char percent[16];
  sprintf(percent, "%d%%", (int)Progress::value);
  Progress::bar->label(percent);
  Fl::check();
  Progress::value += Progress::step;
}

void Dialog::hideProgress()
{
  Progress::dialog->hide();
}

void Dialog::newImage()
{
  NewImage::begin();
}

void Dialog::createPalette()
{
  CreatePalette::begin();
}

void Dialog::showEditor()
{
  Palette::main->draw(editor_palette);
  doEditorSetHsvSliders();
  doEditorSetHsv(1);
  editor->show();
  undo = 0;
  ramp_begin = 0;
  ramp_started = 0;
}

void Dialog::hideEditor()
{
  Palette::main->fillTable();
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
        pal->data[i] = make_rgb(r, g, b);
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
      Blend::rgbToHsv(getr(c1), getg(c1), getb(c1), &h1, &s1, &v1);
      Blend::rgbToHsv(getr(c2), getg(c2), getb(c2), &h2, &s2, &v2);
      double steph = (double)(h2 - h1) / num;
      double steps = (double)(s2 - s1) / num;
      double stepv = (double)(v2 - v1) / num;
      int r, g, b;
      double h = h1;
      double s = s1;
      double v = v1;

      for(i = begin; i < end; i++)
      {
        Blend::hsvToRgb(h, s, v, &r, &g, &b);
        pal->data[i] = make_rgb(r, g, b);
        h += steph;
        s += steps;
        v += stepv;
      }

      editor_hsv_ramp->value(0);
      editor_hsv_ramp->redraw();
    }

    ramp_started = 0;
    Palette::main->draw(editor_palette);
    Gui::drawPalette();

    return;
  }

  Gui::checkPalette(widget, var);
  ramp_begin = *(int *)var;
  doEditorSetHsvSliders();
  doEditorSetHsv(1);
}

void Dialog::doEditorSetHsv(bool redraw)
{
  int x , y;
  int r = 0, g = 0, b = 0;
  int h = 0, s = 0, v = 0;
  int color = Brush::main->color;

  Blend::rgbToHsv(getr(color), getg(color), getb(color), &h, &s, &v);

  if(redraw)
  {
    editor_h->bitmap->clear(make_rgb(0, 0, 0));
    editor_sv->bitmap->clear(make_rgb(0, 0, 0));

    for(y = 0; y < 256; y++)
    {
      for(x = 0; x < 256; x++)
      {
        Blend::hsvToRgb(h, x, y, &r, &g, &b);
        editor_sv->bitmap->setpixelSolid(x, y, make_rgb(r, g, b), 0);
      }

      Blend::hsvToRgb(y * 6, 255, 255, &r, &g, &b);
      editor_h->bitmap->hline(0, y, 23, make_rgb(r, g, b), 0);
    }
  }
  else
  {
    // erase previous box if not redrawing entire thing
    editor_sv->bitmap->xorRect(oldsvx - 4, oldsvy - 4, oldsvx + 4, oldsvy + 4);
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

  editor_sv->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);
  oldsvx = x;
  oldsvy = y;

  editor_h->redraw();
  editor_sv->redraw();

  editor_color->bitmap->clear(Brush::main->color);
  editor_color->redraw();
}

void Dialog::doEditorSetHsvSliders()
{
  int h, s, v;
  int color = Brush::main->color;
  Blend::rgbToHsv(getr(color), getg(color), getb(color), &h, &s, &v);
  editor_h->var = h / 6;
  editor_sv->var = s + 256 * v;
  editor_h->redraw();
  editor_sv->redraw();
}

void Dialog::doEditorGetH()
{
  int h = editor_h->var * 6;
  int s = editor_sv->var & 255;
  int v = editor_sv->var / 256;
  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Brush::main->color = make_rgb(r, g, b);

  Gui::updateColor(Brush::main->color);
  doEditorSetHsv(1);
}

void Dialog::doEditorGetSV()
{
  int h = editor_h->var * 6;
  int s = editor_sv->var & 255;
  int v = editor_sv->var / 256;
  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Brush::main->color = make_rgb(r, g, b);

  Gui::updateColor(Brush::main->color);
  doEditorSetHsv(0);
}

void Dialog::doEditorInsert()
{
  doEditorStoreUndo();
  Palette::main->insertColor(Brush::main->color, editor_palette->var);
  Palette::main->draw(editor_palette);
  Gui::drawPalette();
  editor_palette->do_callback();
}

void Dialog::doEditorDelete()
{
  doEditorStoreUndo();
  Palette::main->deleteColor(editor_palette->var);
  Palette::main->draw(editor_palette);
  Gui::drawPalette();
  if(editor_palette->var > Palette::main->max - 1)
    editor_palette->var = Palette::main->max - 1;
  editor_palette->do_callback();
}

void Dialog::doEditorReplace()
{
  doEditorStoreUndo();
  Palette::main->replaceColor(Brush::main->color, editor_palette->var);
  Palette::main->draw(editor_palette);
  Gui::drawPalette();
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
    Gui::drawPalette();
    editor_palette->do_callback();
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

