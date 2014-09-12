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

#include <cmath>

#include "Gui.H"
#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Palette.H"
#include "Widget.H"
#include "Button.H"
#include "ToggleButton.H"
#include "Field.H"
#include "View.H"
#include "Undo.H"
#include "Dialog.H"
#include "File.H"
#include "FX.H"
#include "Separator.H"
#include "Tool.H"
#include "Stroke.H"

namespace
{
  // window
  Fl_Double_Window *window;

  // main menu
  Fl_Menu_Bar *menubar;

  // containers
  //Fl_Group *group_top;
  Fl_Group *group_left;

  // panels
  //Fl_Group *top_left;
  Fl_Group *top_right;
  Fl_Group *tools;
  Fl_Group *paint;
  Fl_Group *crop;
  Fl_Group *getcolor;
  Fl_Group *offset;
  Fl_Group *right;
  Fl_Group *bottom;
  Fl_Group *middle;

  // top left
  //Widget *logo;

  //top right
  ToggleButton *zoom_fit;
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  Field *zoom;
  ToggleButton *grid;
  Field *gridx;
  Field *gridy;

  // tools
  Widget *tool;

  // options
  Widget *paint_brush;
  Widget *paint_size;
  Widget *paint_stroke;
  Widget *paint_shape;
  Widget *paint_edge;
  Fl_Choice *paint_mode;

  Widget *getcolor_color;

  Field *crop_x;
  Field *crop_y;
  Field *crop_w;
  Field *crop_h;
  Fl_Button *crop_do;

  Field *offset_x;
  Field *offset_y;

  // right
  Widget *palette;
  Widget *hue;
  Widget *satval;
  Widget *trans;
  Fl_Choice *blend;

  // bottom
  ToggleButton *wrap;
  ToggleButton *clone;
  Widget *mirror;
  Widget *origin;
  Widget *constrain;

  // view
  View *view;

  // prevent escape from closing main window
  void close_callback(Fl_Widget *widget, void *)
  {
    if((Fl::event() == FL_KEYDOWN || Fl::event() == FL_SHORTCUT)
      && Fl::event_key() == FL_Escape)
    {
      return;
    }
    else
    {
      Dialog::hideEditor();
      widget->hide();
    }
  }

  // quit program
  void quit()
  {
    fl_message_title("Quit");
    if(fl_choice("Are You Sure?", "No", "Yes", NULL) == 1)
      exit(0);
  }

  int brush_sizes[16] =
  {
    1, 2, 3, 4, 8, 12, 16, 24,
    32, 40, 48, 56, 64, 72, 80, 88
  };
}

void Gui::init()
{
  int x1, y1;

  // window
  window = new Fl_Double_Window(800, 600, "Rendera");
  window->callback(close_callback);

  // menu
  menubar = new Fl_Menu_Bar(0, 0, window->w(), 24);
  menubar->add("File/New", 0, (Fl_Callback *)Dialog::showNewImage, 0, 0);
  menubar->add("File/Load", 0, (Fl_Callback *)File::load, 0, 0);
  menubar->add("File/Save", 0, (Fl_Callback *)File::save, 0, FL_MENU_DIVIDER);
  menubar->add("File/Quit", 0, (Fl_Callback *)quit, 0, 0);
  menubar->add("Edit/Undo", 0, (Fl_Callback *)Undo::pop, 0, 0);
  menubar->add("Palette/Load", 0, (Fl_Callback *)File::loadPalette, 0, 0);
  menubar->add("Palette/Save", 0, (Fl_Callback *)File::savePalette, 0, FL_MENU_DIVIDER);
  menubar->add("Palette/Editor...", 0, (Fl_Callback *)Dialog::showEditor, 0, FL_MENU_DIVIDER);
  menubar->add("Palette/Create From Image...", 0, (Fl_Callback *)Dialog::showCreatePalette, 0, 0);
  menubar->add("Effects/Normalize", 0, (Fl_Callback *)FX::showNormalize, 0, 0);
  menubar->add("Effects/Equalize", 0, (Fl_Callback *)FX::showEqualize, 0, 0);
  menubar->add("Effects/Value Stretch", 0, (Fl_Callback *)FX::showValueStretch, 0, 0);
  menubar->add("Effects/Saturate", 0, (Fl_Callback *)FX::showSaturate, 0, 0);
  menubar->add("Effects/Rotate Hue...", 0, (Fl_Callback *)FX::showRotateHue, 0, 0);
  menubar->add("Effects/Invert", 0, (Fl_Callback *)FX::showInvert, 0, 0);
  menubar->add("Effects/Restore...", 0, (Fl_Callback *)FX::showRestore, 0, 0);
  menubar->add("Effects/Correction Matrix", 0, (Fl_Callback *)FX::showCorrect, 0, 0);
  menubar->add("Effects/Remove Dust...", 0, (Fl_Callback *)FX::showRemoveDust, 0, 0);
  menubar->add("Effects/Colorize", 0, (Fl_Callback *)FX::showColorize, 0, 0);
  menubar->add("Effects/Apply Palette...", 0, (Fl_Callback *)FX::showApplyPalette, 0, 0);
  menubar->add("Help/About...", 0, (Fl_Callback *)Dialog::showAbout, 0, 0);

  // top_left
  //top_left = new Fl_Group(0, menubar->h(), 112, 40);
  //top_left->box(FL_UP_BOX);
  //logo = new Widget(top_left, 8, 8, 96, 24, "", "data/logo.png", 0, 0, 0);
  //top_left->resizable(0);
  //top_left->end();

  // top right
  //top_right = new Fl_Group(top_left->w(), menubar->h(), window->w() - top_left->w(), 40);
  top_right = new Fl_Group(0, menubar->h(), window->w(), 40);
  top_right->box(FL_UP_BOX);
  x1 = 8;
  zoom_fit = new ToggleButton(top_right, x1, 8, 24, 24, "Fit In Window", "data/zoom_fit.png", (Fl_Callback *)checkZoomFit);
  x1 += 24 + 8;
  zoom_one = new Button(top_right, x1, 8, 24, 24, "Actual Size", "data/zoom_one.png", (Fl_Callback *)checkZoomOne);
  x1 += 24 + 8;
  zoom_in = new Button(top_right, x1, 8, 24, 24, "Zoom In", "data/zoom_in.png", (Fl_Callback *)checkZoomIn);
  x1 += 24 + 8;
  zoom_out = new Button(top_right, x1, 8, 24, 24, "Zoom Out", "data/zoom_out.png", (Fl_Callback *)checkZoomOut);
  x1 += 24 + 8;
  zoom = new Field(top_right, x1, 8, 56, 24, "", 0);
  // make this inactive, display only for now
  zoom->deactivate();
  x1 += 56 + 6;
  new Separator(top_right, x1, 2, 2, 36, "");
  x1 += 8;
  grid = new ToggleButton(top_right, x1, 8, 24, 24, "Show Grid", "data/grid.png", (Fl_Callback *)checkGrid);
  x1 += 24 + 48 + 8;
  gridx = new Field(top_right, x1, 8, 32, 24, "Grid X:", (Fl_Callback *)checkGridX);
  gridx->value("8");
  x1 += 32 + 48 + 8;
  gridy = new Field(top_right, x1, 8, 32, 24, "Grid Y:", (Fl_Callback *)checkGridY);
  gridy->value("8");
  top_right->resizable(0);
  top_right->end();

  // bottom
  bottom = new Fl_Group(176, window->h() - 40, window->w() - 288, 40);
  bottom->box(FL_UP_BOX);
  x1 = 8;
  wrap = new ToggleButton(bottom, x1, 8, 24, 24, "Wrap Edges", "data/wrap.png", (Fl_Callback *)checkWrap);
  x1 += 24 + 6;
  new Separator(bottom, x1, 2, 2, 36, "");
  x1 += 8;
  clone = new ToggleButton(bottom, x1, 8, 24, 24, "Clone Enable", "data/clone.png", (Fl_Callback *)checkClone);
  x1 += 24 + 8;
  mirror = new Widget(bottom, x1, 8, 96, 24, "Clone Mirroring", "data/mirror.png", 24, 24, (Fl_Callback *)checkMirror);
  x1 += 96 + 6;
  new Separator(bottom, x1, 2, 2, 36, "");
  x1 += 8;
  origin = new Widget(bottom, x1, 8, 48, 24, "Origin", "data/origin.png", 24, 24, (Fl_Callback *)checkOrigin);
  x1 += 48 + 8;
  constrain = new Widget(bottom, x1, 8, 48, 24, "Keep Square", "data/constrain.png", 24, 24, (Fl_Callback *)checkConstrain);
  bottom->resizable(0);
  bottom->end();

  // tools
  tools = new Fl_Group(0, top_right->h() + menubar->h(), 64, window->h() - (menubar->h() + top_right->h()));
  tools->label("Tools");
  tools->labelsize(12);
  tools->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  tools->box(FL_UP_BOX);
  y1 = 20;
  tool = new Widget(tools, 8, y1, 48, 192, "Tools", "data/tools.png", 48, 48, (Fl_Callback *)checkTool);
  y1 += 96 + 8;
  tools->resizable(0);
  tools->end();

  // paint
  paint = new Fl_Group(64, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  paint->label("Paint");
  paint->labelsize(12);
  paint->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  paint->box(FL_UP_BOX);
  y1 = 20;
  paint_brush = new Widget(paint, 8, y1, 96, 96, "Brush Preview", 0, 0, 0);
  paint_brush->bitmap->clear(make_rgb(255, 255, 255));
  paint_brush->bitmap->setpixelSolid(48, 48, make_rgb(0, 0, 0), 0);
  y1 += 96 + 8;
  paint_size = new Widget(paint, 8, y1, 96, 24, "Size", "data/size.png", 6, 24, (Fl_Callback *)checkPaintSize);
  y1 += 24 + 8;
  paint_stroke = new Widget(paint, 8, y1, 96, 48, "Stroke", "data/stroke.png", 24, 24, (Fl_Callback *)checkPaintStroke);
  y1 += 48 + 8;
  paint_shape = new Widget(paint, 8, y1, 96, 24, "Shape", "data/shape.png", 24, 24, (Fl_Callback *)checkPaintShape);
  y1 += 24 + 8;
  paint_edge = new Widget(paint, 8, y1, 96, 24, "Soft Edge", "data/soft_edge.png", 12, 24, (Fl_Callback *)checkPaintEdge);
  y1 += 24 + 8;
  paint_mode = new Fl_Choice(8, y1, 96, 24, "");
  paint_mode->textsize(10);
  paint_mode->resize(paint->x() + 8, paint->y() + y1, 96, 24);
  paint_mode->add("Solid");
  paint_mode->add("Coarse");
  paint_mode->add("Fine");
  paint_mode->add("Watercolor");
  paint_mode->add("Chalk");
  paint_mode->value(0);
  y1 += 24 + 8;
  paint->resizable(0);
  paint->end();

  // crop
  crop = new Fl_Group(64, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  crop->label("Crop");
  crop->labelsize(12);
  crop->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  crop->box(FL_UP_BOX);
  y1 = 20;
  crop_x = new Field(crop, 24, y1, 72, 24, "X:", 0);
  crop_x->deactivate();
  y1 += 24 + 6;
  crop_y = new Field(crop, 24, y1, 72, 24, "Y:", 0);
  crop_y->deactivate();
  y1 += 24 + 6;
  crop_w = new Field(crop, 24, y1, 72, 24, "W:", 0);
  crop_w->deactivate();
  y1 += 24 + 6;
  crop_h = new Field(crop, 24, y1, 72, 24, "H:", 0);
  crop_h->deactivate();
  y1 += 24 + 6;
  crop_do = new Fl_Button(crop->x() + 16, crop->y() + y1, 72, 32, "Crop");
//  crop_do->resize(crop->x() + 8, crop->y() + y1, 96, 48);
  crop_do->callback((Fl_Callback *)checkCropDo);
  crop->resizable(0);
  crop->end();

  // getcolor
  getcolor = new Fl_Group(64, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  getcolor->label("Get Color");
  getcolor->labelsize(12);
  getcolor->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  getcolor->box(FL_UP_BOX);
  y1 = 20;
  getcolor_color = new Widget(getcolor, 8, y1, 96, 96, "Color", 0, 0, 0);
  getcolor->resizable(0);
  getcolor->end();

  // offset
  offset = new Fl_Group(64, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  offset->label("Offset");
  offset->labelsize(12);
  offset->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  offset->box(FL_UP_BOX);
  y1 = 20;
  offset_x = new Field(offset, 24, y1, 72, 24, "X:", 0);
  offset_x->deactivate();
  y1 += 24 + 6;
  offset_y = new Field(offset, 24, y1, 72, 24, "Y:", 0);
  offset_y->deactivate();
  y1 += 24 + 6;
  offset->resizable(0);
  offset->end();

  // right
  right = new Fl_Group(window->w() - 112, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  right->label("Colors");
  right->labelsize(12);
  right->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
  right->box(FL_UP_BOX);
  y1 = 20;
  palette = new Widget(right, 8, y1, 96, 96, "Color Palette", 6, 6, (Fl_Callback *)checkPalette);
  y1 += 96 + 8;
  hue = new Widget(right, 8, y1, 96, 96, "Hue", 1, 1, (Fl_Callback *)checkHue);
  y1 += 96 + 8;
//  color = new Widget(right, 8, y1, 96, 48, "Color", 0, 0, 0);
// / y1 += 48 + 8;
  satval = new Widget(right, 8, y1, 96, 96, "Saturation/Value", 1, 1, (Fl_Callback *)checkSatVal);
  y1 += 96 + 8;
  trans = new Widget(right, 8, y1, 96, 24, "Transparency", "data/transparency.png", 1, 24, (Fl_Callback *)checkTrans);
  y1 += 24 + 8;
  blend = new Fl_Choice(8, y1, 96, 24, "");
  blend->textsize(10);
  blend->resize(right->x() + 8, right->y() + y1, 96, 24);
  blend->add("Normal");
  blend->add("Darken");
  blend->add("Lighten");
  blend->add("Colorize");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->add("Smooth (Color Only)");
  blend->value(0);
  blend->callback((Fl_Callback *)checkBlend);
  y1 += 24 + 8;
  right->resizable(0);
  right->end();

  // middle
  middle = new Fl_Group(176, top_right->h() + menubar->h(), window->w() - 288, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  middle->box(FL_FLAT_BOX);
  view = new View(middle, 0, 0, middle->w(), middle->h(), "View");
  middle->resizable(view);
  middle->end();

  // container for top panels
  //group_top = new Fl_Group(0, menubar->h(), window->w() - top_left->w(), 40);
  //group_top = new Fl_Group(0, menubar->h(), window->w(), 40);
  //group_top->add(top_left);
  //group_top->add(top_right);
  //group_top->resizable(top_right);
  //group_top->end();

  // container for left panels
  group_left = new Fl_Group(0, top_right->h() + menubar->h(), 176, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  group_left->add(tools);
  group_left->add(paint);
  group_left->add(getcolor);
  group_left->add(crop);
  group_left->add(offset);
  group_left->end();

  window->size_range(640, 480, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();
  window->show();

  // misc init
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

  drawPalette();
  tool->do_callback();
  checkZoom();
  checkCropValues(0, 0, 0, 0);
  checkOffsetValues(0, 0);
}

void Gui::setMenuItem(const char *s)
{
  Fl_Menu_Item *m;
  m = (Fl_Menu_Item *)menubar->find_item(s);

  if(m)
    m->set();
}

void Gui::clearMenuItem(const char *s)
{
  Fl_Menu_Item *m;
  m = (Fl_Menu_Item *)menubar->find_item(s);

  if(m)
    m->clear();
}

void Gui::updateColor(int c)
{
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);

  int h, s, v;

  Blend::rgbToHsv(r, g, b, &h, &s, &v);

  float angle = ((3.14159 * 2) / 1536) * h;
  int mx = 48 + 40 * std::cos(angle);
  int my = 48 + 40 * std::sin(angle);
  hue->var = mx + 96 * my;
  satval->var = (int)(s / 2.684) + 96 * (int)(v / 2.684);

  hue->do_callback();

  Brush::main->color = c;
}

void Gui::updateGetColor(int c)
{
  getcolor_color->bitmap->clear(c);
  getcolor_color->redraw();
}

void Gui::checkPalette(Widget *widget, void *var)
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

  int c = widget->bitmap->getpixel(x * step + 1, y * step + 1);
  updateColor(c);
}

void Gui::drawPalette()
{
  Palette::main->draw(palette);
  palette->var = 0;
  palette->redraw();
  palette->do_callback();
}

void Gui::checkZoomIn(Button *, void *)
{
  view->zoomIn(view->w() / 2, view->h() / 2);
  checkZoom();
}

void Gui::checkZoomOut(Button *, void *)
{
  view->zoomOut(view->w() / 2, view->h() / 2);
  checkZoom();
}

void Gui::checkZoomFit(ToggleButton *, void *var)
{
  view->zoomFit(*(int *)var);
  checkZoom();
}

void Gui::checkZoomOne(Button *, void *)
{
  zoom_fit->var = 0;
  zoom_fit->redraw();
  view->zoomOne();
  checkZoom();
}

void Gui::checkZoom()
{
  char s[8];
  snprintf(s, sizeof(s), "%2.3f", view->zoom);
  zoom->value(s);
  zoom->redraw();
}

void Gui::checkGrid(ToggleButton *, void *var)
{
  view->grid = *(int *)var;
  view->drawMain(1);
}

void Gui::checkGridX(Field *field, void *)
{
  int num = atoi(field->value());
  if(num < 1)
    num = 1;
  if(num > 256)
    num = 256;
  char s[8];
  snprintf(s, sizeof(s), "%d", num);
  field->value(s);
  view->gridx = num;
  view->drawMain(1);
}

void Gui::checkGridY(Field *field, void *)
{
  int num = atoi(field->value());
  if(num < 1)
    num = 1;
  if(num > 256)
    num = 256;
  char s[8];
  snprintf(s, sizeof(s), "%d", num);
  field->value(s);
  view->gridy = num;
  view->drawMain(1);
}

void Gui::checkPaintSize(Widget *, void *var)
{
  Brush *brush = Brush::main;

  int size = brush_sizes[*(int *)var];
  int shape = paint_shape->var;

  brush->make(shape, size);
  paint_brush->bitmap->clear(make_rgb(255, 255, 255));

  int i;

  for(i = 0; i < Brush::main->solid_count; i++)
  {
    paint_brush->bitmap->setpixelSolid(48 + brush->solidx[i],
                                             48 + brush->solidy[i],
                                             make_rgb(0, 0, 0), 0);
  }

  paint_brush->redraw();
}

void Gui::checkPaintShape(Widget *, void *)
{
  paint_size->do_callback();
}

void Gui::checkPaintStroke(Widget *, void *var)
{
  view->tool->stroke->type = *(int *)var;
}

void Gui::checkPaintEdge(Widget *, void *var)
{
  Brush::main->edge = *(int *)var;
}

void Gui::checkTool(Widget *, void *var)
{
  paint->hide();
  getcolor->hide();
  crop->hide();
  offset->hide();

  switch(*(int *)var)
  {
    case 0:
      view->tool = Tool::paint;
      paint_brush->do_callback();
      paint_shape->do_callback();
      paint->show();
      break;
    case 1:
      view->tool = Tool::getcolor;
      getcolor->show();
      break;
    case 2:
      view->tool = Tool::crop;
      crop->show();
      break;
    case 3:
      view->tool = Tool::offset;
      offset->show();
      break;
  }

  view->tool->active = 0;
  view->tool->started = 0;
}

void Gui::checkColor(Widget *, void *)
{
  int pos = hue->var;
  int mx = pos % 96;
  int my = pos / 96;

  float mouse_angle = atan2f(my - 48, mx - 48);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 96) * 2.685;
  int v = (satval->var / 96) * 2.685;

  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Brush::main->color = make_rgb(r, g, b);
  Brush::main->trans = trans->var * 2.685;
  Brush::main->blend = blend->value();

  int i;

  hue->bitmap->clear((Fl::get_color(FL_BACKGROUND_COLOR) >> 8) | 0xFF000000);
  satval->bitmap->clear(0xFF000000);

  for(i = 1; i < 1536; i++)
  {
    float angle = ((3.14159 * 2) / 1536) * i;
    int x1 = 48 + 40 * std::cos(angle);
    int y1 = 48 + 40 * std::sin(angle);
    int x2 = 48 + 20 * std::cos(angle);
    int y2 = 48 + 20 * std::sin(angle);

    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    hue->bitmap->line(x1, y1, x2, y2, make_rgb(r, g, b), 0);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, make_rgb(r, g, b), 0);
  }

  int x1 = 48 + 40 * std::cos(mouse_angle);
  int y1 = 48 + 40 * std::sin(mouse_angle);
  int x2 = 48 + 20 * std::cos(mouse_angle);
  int y2 = 48 + 20 * std::sin(mouse_angle);

  hue->bitmap->xorLine(x1, y1, x2, y2);

  int x, y;

  for(y = 0; y < 96; y++)
  {
    for(x = 0; x < 96; x++)
    {
      Blend::hsvToRgb(h, x * 2.685, y * 2.685, &r, &g, &b);
      satval->bitmap->setpixelSolid(x, y, make_rgb(r, g, b), 0);
    }
  }

  x = (satval->var % 96);
  y = (satval->var / 96);

  if(x < 4)
    x = 4;
  if(y < 4)
    y = 4;
  if(x > 91)
    x = 91;
  if(y > 91)
    y = 91;

  satval->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);

  hue->redraw();
  satval->redraw();
}

void Gui::checkHue(Widget *, void *)
{
  checkColor(0, 0);
}

void Gui::checkSatVal(Widget *, void *)
{
  checkColor(0, 0);
}

void Gui::checkTrans(Widget *, void *)
{
  checkColor(0, 0);
}

void Gui::checkBlend(Widget *, void *)
{
  checkColor(0, 0);
}

void Gui::checkWrap(Widget *, void *var)
{
  Bitmap::wrap = *(int *)var;
}

void Gui::checkClone(Widget *, void *var)
{
  Bitmap::clone = *(int *)var;
}

void Gui::checkMirror(Widget *, void *var)
{
  Bitmap::clone_mirror = *(int *)var;
}

void Gui::checkOrigin(Widget *, void *var)
{
  view->tool->stroke->origin = *(int *)var;
}

void Gui::checkConstrain(Widget *, void *var)
{
  view->tool->stroke->constrain = *(int *)var;
}

void Gui::checkCropDo()
{
  view->tool->done(view);
}

void Gui::checkCropValues(int x, int y, int w, int h)
{
  char s[8];

  snprintf(s, sizeof(s), "%d", x);
  crop_x->value(s);
  crop_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  crop_y->value(s);
  crop_y->redraw();

  snprintf(s, sizeof(s), "%d", w);
  crop_w->value(s);
  crop_w->redraw();

  snprintf(s, sizeof(s), "%d", h);
  crop_h->value(s);
  crop_h->redraw();
}

void Gui::checkOffsetValues(int x, int y)
{
  char s[8];

  snprintf(s, sizeof(s), "%d", x);
  offset_x->value(s);
  offset_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  offset_y->value(s);
  offset_y->redraw();
}

View *Gui::getView()
{
  return view;
}

int Gui::getTool()
{
  return tool->var;
}

int Gui::getClone()
{
  return clone->var;
}

int Gui::getPaintMode()
{
  return paint_mode->value();
}
