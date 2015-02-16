/*
Copyright (c) 2015 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
state WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <cmath>

#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tooltip.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "Button.H"
#include "CheckBox.H"
#include "Clone.H"
#include "Dialog.H"
#include "FX.H"
#include "File.H"
#include "Group.H"
#include "Gui.H"
#include "Inline.H"
#include "InputInt.H"
#include "InputText.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "Separator.H"
#include "Stroke.H"
#include "ToggleButton.H"
#include "Tool.H"
#include "Transform.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

namespace Gui
{
  // window
  Fl_Double_Window *window;

  // main menu
  Fl_Menu_Bar *menubar;

  // containers
  Fl_Group *group_left;

  // panels
  Group *top_right;
  Group *tools;
  Group *paint;
  Group *crop;
  Group *getcolor;
  Group *offset;
  Group *text;
  Group *fill;
  Group *right;
  Group *bottom;
  Group *status;
  Fl_Group *middle;

  // status
  Fl_Progress *progress;
  Fl_Box *coords;
  Fl_Box *info;

  // top right
  ToggleButton *zoom_fit;
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  InputInt *zoom;
  ToggleButton *grid;
  InputInt *gridx;
  InputInt *gridy;

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

  InputInt *crop_x;
  InputInt *crop_y;
  InputInt *crop_w;
  InputInt *crop_h;
  Fl_Button *crop_do;

  InputInt *offset_x;
  InputInt *offset_y;

  Fl_Hold_Browser *font_browse;
  InputInt *font_size;
  Fl_Input *text_input;
  CheckBox *text_smooth;

  // right
  Widget *palette;
  Widget *swatch;
  Widget *hue;
  Widget *satval;
  InputText *hexcolor;
  Widget *trans;
  Fl_Choice *blend;

  // bottom
  ToggleButton *wrap;
  ToggleButton *clone;
  Widget *mirror;
  ToggleButton *origin;
  ToggleButton *constrain;

  // view
  View *view;

  float progress_value = 0;
  float progress_step = 0;

  // tables
  const int brush_sizes[16] =
  {
    1, 2, 3, 4, 8, 12, 16, 24,
    32, 40, 48, 56, 64, 72, 80, 88
  };

  // prevent escape from closing main window
  void closeCallback(Fl_Widget *widget, void *)
  {
    if((Fl::event() == FL_KEYDOWN || Fl::event() == FL_SHORTCUT)
       && Fl::event_key() == FL_Escape)
    {
        return;
    }
    else
    {
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
}

void Gui::init()
{
  int x1, y1;

  // main window
  window = new Fl_Double_Window(800, 600, "Rendera");
  window->callback(closeCallback);

  // menu
  menubar = new Fl_Menu_Bar(0, 0, window->w(), 24);
  menubar->box(FL_THIN_UP_BOX);

  menubar->add("&File/New...", 0,
    (Fl_Callback *)Dialog::newImage, 0, 0);
  menubar->add("&File/&Open...", 0,
    (Fl_Callback *)File::load, 0, 0);
  menubar->add("&File/&Save...", 0,
    (Fl_Callback *)File::save, 0, FL_MENU_DIVIDER);
  menubar->add("&File/&Quit...", 0,
    (Fl_Callback *)quit, 0, 0);
  menubar->add("&Edit/Undo (Ctrl+Z)", 0,
    (Fl_Callback *)Undo::pop, 0, FL_MENU_DIVIDER);
  menubar->add("&Edit/Clear/Transparent", 0,
    (Fl_Callback *)checkClearToTransparent, 0, 0);
  menubar->add("&Edit/Clear/Paint Color", 0,
    (Fl_Callback *)checkClearToPaintColor, 0, 0);
  menubar->add("&Edit/Clear/Black", 0,
    (Fl_Callback *)checkClearToBlack, 0, 0);
  menubar->add("&Edit/Clear/White", 0,
    (Fl_Callback *)checkClearToWhite, 0, 0);
  menubar->add("Image/Flip Horizontal", 0,
    (Fl_Callback *)Transform::mirror, 0, 0);
  menubar->add("Image/Flip Vertical", 0,
    (Fl_Callback *)Transform::flip, 0, FL_MENU_DIVIDER);
  menubar->add("Image/Resize...", 0,
    (Fl_Callback *)Transform::resize, 0, 0);
  menubar->add("Image/Scale...", 0,
    (Fl_Callback *)Transform::scale, 0, 0);
  menubar->add("Image/Rotate...", 0,
    (Fl_Callback *)Transform::rotate, 0, 0);
  menubar->add("Palette/Load...", 0,
    (Fl_Callback *)File::loadPalette, 0, 0);
  menubar->add("Palette/Save...", 0,
    (Fl_Callback *)File::savePalette, 0, FL_MENU_DIVIDER);
  menubar->add("Palette/Create From Image...", 0,
    (Fl_Callback *)Dialog::makePalette, 0, 0);
  menubar->add("Palette/Apply to Image...", 0,
    (Fl_Callback *)FX::applyPalette, 0, FL_MENU_DIVIDER);
  menubar->add("Palette/Presets/Default", 0,
    (Fl_Callback *)paletteDefault, 0, 0);
  menubar->add("Palette/Presets/Black and White", 0,
    (Fl_Callback *)paletteBlackAndWhite, 0, 0);
  menubar->add("Palette/Presets/Web Safe", 0,
    (Fl_Callback *)paletteWebSafe, 0, 0);
  menubar->add("Palette/Presets/3-level RGB", 0,
    (Fl_Callback *)palette3LevelRGB, 0, 0);
  menubar->add("Palette/Presets/4-level RGB", 0,
    (Fl_Callback *)palette4LevelRGB, 0, 0);
  menubar->add("Palette/Presets/3-3-2", 0,
    (Fl_Callback *)palette332, 0, 0);
  menubar->add("Palette/Sort", 0,
    (Fl_Callback *)paletteSort, 0, FL_MENU_DIVIDER);
  menubar->add("Palette/Editor...", 0,
    (Fl_Callback *)Dialog::editor, 0, 0);
  menubar->add("Effects/Normalize", 0,
    (Fl_Callback *)FX::normalize, 0, 0);
  menubar->add("Effects/Equalize", 0,
    (Fl_Callback *)FX::equalize, 0, 0);
  menubar->add("Effects/Value Stretch", 0,
    (Fl_Callback *)FX::valueStretch, 0, 0);
  menubar->add("Effects/Saturate", 0,
    (Fl_Callback *)FX::saturate, 0, 0);
  menubar->add("Effects/Rotate Hue...", 0,
    (Fl_Callback *)FX::rotateHue, 0, 0);
  menubar->add("Effects/Invert", 0,
    (Fl_Callback *)FX::invert, 0, 0);
  menubar->add("Effects/Invert Alpha", 0,
    (Fl_Callback *)FX::invertAlpha, 0, 0);
  menubar->add("Effects/Restore...", 0,
    (Fl_Callback *)FX::restore, 0, 0);
  menubar->add("Effects/Correction Matrix", 0,
    (Fl_Callback *)FX::correctionMatrix, 0, 0);
  menubar->add("Effects/Remove Dust...", 0,
    (Fl_Callback *)FX::removeDust, 0, 0);
  menubar->add("Effects/Desaturate", 0,
    (Fl_Callback *)FX::desaturate, 0, 0);
  menubar->add("Effects/Colorize", 0,
    (Fl_Callback *)FX::colorize, 0, 0);
  menubar->add("Effects/Stained Glass...", 0,
    (Fl_Callback *)FX::stainedGlass, 0, 0);
  menubar->add("Effects/Blur...", 0,
    (Fl_Callback *)FX::blur, 0, 0);
  menubar->add("Effects/Sharpen...", 0,
    (Fl_Callback *)FX::sharpen, 0, 0);
  menubar->add("Effects/Unsharp Mask...", 0,
    (Fl_Callback *)FX::unsharpMask, 0, 0);
  menubar->add("Effects/Convolution Matrix...", 0,
    (Fl_Callback *)FX::convolutionMatrix, 0, 0);
  menubar->add("Effects/Artistic...", 0,
    (Fl_Callback *)FX::artistic, 0, 0);
//  menubar->add("Effects/Forward FFT", 0,
//    (Fl_Callback *)FX::forwardFFT, 0, 0);
//  menubar->add("Effects/Inverse FFT", 0,
//    (Fl_Callback *)FX::inverseFFT, 0, 0);
  menubar->add("&Help/&About...", 0,
    (Fl_Callback *)Dialog::about, 0, 0);

  // status
  status = new Group(0, window->h() - 24, window->w(), 24, "");
  x1 = 8;
  coords = new Fl_Box(FL_FLAT_BOX, x1, 4, 96, 16, "");
  coords->resize(status->x() + x1, status->y() + 4, 96, 16);
  coords->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  coords->copy_label("(0, 0)");
  x1 += 96 + 6;
  new Separator(status, x1, 2, 2, 20, "");
  x1 += 8;
  info = new Fl_Box(FL_FLAT_BOX, x1, 4, window->w() - x1, 16, "");
  info->resize(status->x() + x1, status->y() + 4, window->w() - x1, 16);
  info->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  info->copy_label("Welcome to Rendera!");
  progress = new Fl_Progress(x1, window->w() - 256 - 8, 256, 16);
  progress->resize(status->x() + window->w() - 256 - 8, status->y() + 4, 256, 16);
  progress->minimum(0);
  progress->maximum(100);
  progress->color(0x40404000);
  progress->selection_color(0x88CC8800);
  progress->labelcolor(0xFFFFFF00);
  progress->hide();
  status->resizable(0);
  status->end();

  // top right
  top_right = new Group(0, menubar->h(), window->w(), 40, "");
  x1 = 8;
  zoom_fit = new ToggleButton(top_right, x1, 8, 24, 24,
                              "Fit In Window", File::themePath("zoom_fit.png"),
                              (Fl_Callback *)checkZoomFit);
  x1 += 24 + 8;
  zoom_one = new Button(top_right, x1, 8, 24, 24,
                        "Actual Size", File::themePath("zoom_one.png"),
                        (Fl_Callback *)checkZoomOne);
  x1 += 24 + 8;
  zoom_in = new Button(top_right, x1, 8, 24, 24,
                       "Zoom In", File::themePath("zoom_in.png"),
                       (Fl_Callback *)checkZoomIn);
  x1 += 24 + 8;
  zoom_out = new Button(top_right, x1, 8, 24, 24,
                        "Zoom Out", File::themePath("zoom_out.png"),
                        (Fl_Callback *)checkZoomOut);
  x1 += 24 + 8;
  zoom = new InputInt(top_right, x1, 8, 56, 24, "", 0);
  // make this inactive, display only for now
  zoom->deactivate();
  x1 += 56 + 6;
  new Separator(top_right, x1, 2, 2, 36, "");
  x1 += 8;
  grid = new ToggleButton(top_right, x1, 8, 24, 24,
                          "Show Grid", File::themePath("grid.png"),
                          (Fl_Callback *)checkGrid);
  x1 += 24 + 48 + 8;
  gridx = new InputInt(top_right, x1, 8, 32, 24,
                       "Grid X:",
                       (Fl_Callback *)checkGridX);
  gridx->value("8");
  x1 += 32 + 48 + 8;
  gridy = new InputInt(top_right, x1, 8, 32, 24,
                       "Grid Y:",
                       (Fl_Callback *)checkGridY);
  gridy->value("8");
  top_right->resizable(0);
  top_right->end();

  // bottom
  bottom = new Group(160, window->h() - status->h() - 40, window->w() - 272, 40, "");
  x1 = 8;
  wrap = new ToggleButton(bottom, x1, 8, 24, 24,
                          "Wrap Edges", File::themePath("wrap.png"),
                          (Fl_Callback *)checkWrap);
  x1 += 24 + 6;
  new Separator(bottom, x1, 2, 2, 36, "");
  x1 += 8;
  clone = new ToggleButton(bottom, x1, 8, 24, 24,
                           "Clone Enable (Shift+Click sets clone target!)", File::themePath("clone.png"),
                           (Fl_Callback *)checkClone);
  x1 += 24 + 8;
  mirror = new Widget(bottom, x1, 8, 96, 24,
                      "Clone Mirroring", File::themePath("mirror.png"), 24, 24,
                      (Fl_Callback *)checkMirror);
  x1 += 96 + 6;
  new Separator(bottom, x1, 2, 2, 36, "");
  x1 += 8;
  origin = new ToggleButton(bottom, x1, 8, 24, 24,
                            "Start Shape From Center", File::themePath("origin.png"),
                            (Fl_Callback *)checkOrigin);
  x1 += 24 + 8;
  constrain = new ToggleButton(bottom, x1, 8, 24, 24,
                              "Lock Shape Proportions",
                              File::themePath("constrain.png"),
                              (Fl_Callback *)checkConstrain);
  bottom->resizable(0);
  bottom->end();

  // tools
  tools = new Group(0, top_right->h() + menubar->h(),
                    48, window->h() - (menubar->h() + top_right->h() + status->h()),
                    "Tools");
  y1 = 20;
  tool = new Widget(tools, 8, y1, 32, 192,
                    "Tools", File::themePath("tools.png"), 32, 32,
                    (Fl_Callback *)checkTool);
  y1 += 96 + 8;
  tools->resizable(0);
  tools->end();

  // paint
  paint = new Group(48, top_right->h() + menubar->h(),
                    112, window->h() - top_right->h() - menubar->h() - status->h(),
                    "Painting");
  y1 = 20;
  paint_brush = new Widget(paint, 8, y1, 96, 96,
                           "Brush Preview", 0, 0, 0);
  paint_brush->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR));
  paint_brush->bitmap->setpixelSolid(48, 48, makeRgb(192, 192, 192), 0);
  y1 += 96 + 8;
  paint_size = new Widget(paint, 8, y1, 96, 24,
                          "Size", File::themePath("size.png"), 6, 24,
                          (Fl_Callback *)checkPaintSize);
  y1 += 24 + 8;
  paint_stroke = new Widget(paint, 8, y1, 96, 48,
                            "Stroke", File::themePath("stroke.png"), 24, 24,
                            (Fl_Callback *)checkPaintStroke);
  y1 += 48 + 8;
  paint_shape = new Widget(paint, 8, y1, 96, 24,
                           "Shape", File::themePath("shape.png"), 24, 24,
                           (Fl_Callback *)checkPaintShape);
  y1 += 24 + 8;
  paint_edge = new Widget(paint, 8, y1, 96, 24,
                          "Soft Edge", File::themePath("soft_edge.png"), 12, 24,
                          (Fl_Callback *)checkPaintEdge);
  y1 += 24 + 8;
  paint_mode = new Fl_Choice(8, y1, 96, 24, "");
  paint_mode->tooltip("Paint Mode");
  paint_mode->textsize(10);
  paint_mode->resize(paint->x() + 8, paint->y() + y1, 96, 24);
  paint_mode->add("Solid");
  paint_mode->add("Antialiased");
  paint_mode->add("Coarse Airbrush");
  paint_mode->add("Fine Airbrush");
  paint_mode->add("Gaussian Blur");
  paint_mode->add("Watercolor");
  paint_mode->add("Chalk");
  paint_mode->value(0);
  paint_mode->callback((Fl_Callback *)checkPaintMode);
  y1 += 24 + 8;
  paint->resizable(0);
  paint->end();

  // crop
  crop = new Group(48, top_right->h() + menubar->h(),
                   112, window->h() - top_right->h() - menubar->h() - status->h(),
                   "Crop");
  y1 = 20;
  crop_x = new InputInt(crop, 24, y1, 72, 24, "X:", 0);
  crop_x->deactivate();
  y1 += 24 + 6;
  crop_y = new InputInt(crop, 24, y1, 72, 24, "Y:", 0);
  crop_y->deactivate();
  y1 += 24 + 6;
  crop_w = new InputInt(crop, 24, y1, 72, 24, "W:", 0);
  crop_w->deactivate();
  y1 += 24 + 6;
  crop_h = new InputInt(crop, 24, y1, 72, 24, "H:", 0);
  crop_h->deactivate();
  y1 += 24 + 6;
  crop_do = new Fl_Button(crop->x() + 16, crop->y() + y1, 72, 32, "Crop");
  crop_do->callback((Fl_Callback *)checkCropDo);
  crop->resizable(0);
  crop->end();

  // getcolor
  getcolor = new Group(48, top_right->h() + menubar->h(),
                       112, window->h() - top_right->h() - menubar->h() - status->h(),
                       "GetColor");
  y1 = 20;
  getcolor_color = new Widget(getcolor, 8, y1, 96, 96, "Color", 0, 0, 0);
  getcolor->resizable(0);
  getcolor->end();

  // offset
  offset = new Group(48, top_right->h() + menubar->h(),
                     112, window->h() - top_right->h() - menubar->h() - status->h(),
                     "Offset");
  y1 = 20;
  offset_x = new InputInt(offset, 24, y1, 72, 24, "X:", 0);
  offset_x->deactivate();
  y1 += 24 + 6;
  offset_y = new InputInt(offset, 24, y1, 72, 24, "Y:", 0);
  offset_y->deactivate();
  y1 += 24 + 6;
  offset->resizable(0);
  offset->end();

  // text
  text = new Group(48, top_right->h() + menubar->h(),
                   112, window->h() - top_right->h() - menubar->h() - status->h(),
                   "Text");
  y1 = 20;
  // add font names
  font_browse = new Fl_Hold_Browser(8, y1, 96, 192);
  font_browse->textsize(9);
  font_browse->resize(text->x() + 8, text->y() + y1, 96, 192);

  for(int i = 0; i < Fl::set_fonts(0); i++)
  {
    int t = 0;
    const char *name = Fl::get_font_name((Fl_Font)i, &t);
    font_browse->add(name);
  }

  font_browse->value(0);
  font_browse->callback((Fl_Callback *)textStartOver);
  y1 += 192 + 8;

  // font size
  font_size = new InputInt(text, 40, y1, 64, 24, "Size:",
                           (Fl_Callback *)textStartOver);
  font_size->value("12");
  y1 += 24 + 8;

  text_input = new Fl_Input(8, y1, 96, 24, "");
  text_input->textsize(10);
  text_input->value("Text");
  text_input->resize(text->x() + 8, text->y() + y1, 96, 24);
  text_input->callback((Fl_Callback *)textStartOver);
  y1 += 24 + 8;

  text_smooth = new CheckBox(text, 12, y1, 16, 16, "Smooth", 0);
  text_smooth->value(1);

  text->resizable(0);
  text->end();

  // fill
  fill = new Group(48, top_right->h() + menubar->h(),
                   112, window->h() - top_right->h() - menubar->h() - status->h(),
                   "Fill");
  fill->end();

  // right
  right = new Group(window->w() - 112, top_right->h() + menubar->h(),
                    112, window->h() - top_right->h() - menubar->h() - status->h(),
                    "Colors");
  y1 = 20;
  palette = new Widget(right, 8, y1, 96, 96,
                       "Color Palette", 6, 6,
                       (Fl_Callback *)checkPalette);
  y1 += 96 + 8;
  swatch = new Widget(right, 8, y1, 96, 48,
                      "Color Swatch", 0, 0,
                      0);
  y1 += 48 + 8;
  hexcolor = new InputText(right, 40, y1, 64, 24, "Hex:",
                           (Fl_Callback *)checkHexColor);
  hexcolor->maximum_size(6);
  hexcolor->textsize(14);
  y1 += 24 + 8;
  // satval overlaps the hue color wheel
  hue = new Widget(right, 8, y1, 96, 96,
                   "Hue", 1, 1,
                   (Fl_Callback *)checkHue);
  satval = new Widget(right, 32, y1 + 24, 48, 48,
                      "Saturation/Value", 1, 1,
                      (Fl_Callback *)checkSatVal);
  y1 += 96 + 8;
  trans = new Widget(right, 8, y1, 96, 24,
                     "Transparency", File::themePath("transparency.png"), 1, 24,
                     (Fl_Callback *)checkTrans);
  y1 += 24 + 8;
  blend = new Fl_Choice(8, y1, 96, 24, "");
  blend->tooltip("Blending Mode");
  blend->textsize(10);
  blend->resize(right->x() + 8, right->y() + y1, 96, 24);
  blend->add("Normal");
  blend->add("Normal (No Alpha)");
  blend->add("Darken");
  blend->add("Lighten");
  blend->add("Colorize");
  blend->add("Colorize (Highlights)");
  blend->add("Colorize (Midtones)");
  blend->add("Colorize (Shadows)");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->add("Smooth (Color Only)");
  blend->add("Sharpen");
  blend->add("Palette Colors");
  blend->value(0);
  blend->callback((Fl_Callback *)checkBlend);
  y1 += 24 + 8;
  right->resizable(0);
  right->end();

  // middle
  middle = new Fl_Group(160, top_right->h() + menubar->h(),
                        window->w() - 272, window->h() - (menubar->h() + top_right->h() + bottom->h() + status->h()));
  middle->box(FL_FLAT_BOX);
  view = new View(middle, 0, 0, middle->w(), middle->h(), "View");
  middle->resizable(view);
  middle->end();

  // container for left panels
  group_left = new Fl_Group(0, top_right->h() + menubar->h(),
                            160, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  group_left->add(tools);
  group_left->add(paint);
  group_left->add(getcolor);
  group_left->add(crop);
  group_left->add(offset);
  group_left->add(text);
  group_left->end();

  window->size_range(640, 480, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();

  // misc init
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

  drawPalette();
  tool->do_callback();
  checkZoom();
  checkCropValues(0, 0, 0, 0);
  checkOffsetValues(0, 0);

  // fix certain icons if using a light theme
  if(Project::theme == Project::THEME_LIGHT)
  {
    paint_size->bitmap->invert();
    paint_size->bitmap2->invert();
    paint_edge->bitmap->invert();
    paint_edge->bitmap2->invert();
    trans->bitmap->invert();
    trans->bitmap2->invert();
  }
}

void Gui::show()
{
  window->show();
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

void Gui::checkHexColor()
{
  unsigned int c;

  sscanf(hexcolor->value(), "%06x", &c);

  if(c > 0xFFFFFF)
    c = 0xFFFFFF;

  c |= 0xFF000000;

  updateColor(convertFormat((int)c, true));
  updateHexColor();
}

void Gui::updateHexColor()
{
  char hex_string[8];
  snprintf(hex_string, sizeof(hex_string),
       "%06x", (unsigned)convertFormat(Project::brush->color, true) & 0xFFFFFF);
  hexcolor->value(hex_string);
}

void Gui::updateColor(int c)
{
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);

  int h, s, v;

  Blend::rgbToHsv(r, g, b, &h, &s, &v);

  float angle = ((3.14159 * 2) / 1536) * h;
  int mx = 48 + 41 * std::cos(angle);
  int my = 48 + 41 * std::sin(angle);
  hue->var = mx + 96 * my;
  satval->var = (int)(s / 5.42) + 48 * (int)(v / 5.42);

  hue->do_callback();

  Project::brush->color = c;
  updateHexColor();
  updateSwatch();
}

void Gui::updateSwatch()
{
  for(int y = 0; y < swatch->bitmap->h; y++)
  {
    int *p = swatch->bitmap->row[y];
    for(int x = 0; x < swatch->bitmap->w; x++)
    {
      const int checker = ((x >> 4) & 1) ^ ((y >> 4) & 1)
                            ? 0xA0A0A0 : 0x606060;

      *p++ = blendFast(checker, Project::brush->color, Project::brush->trans);
    }
  }

  swatch->redraw();
}

void Gui::updateGetColor(int c)
{
  getcolor_color->bitmap->clear(c);
  getcolor_color->redraw();
}

void Gui::checkPalette(Widget *widget, void *var)
{
  Palette *palette = Project::palette.get();
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
  Project::palette->draw(palette);
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
  view->drawMain(true);
  view->redraw();
}

void Gui::checkGridX()
{
  gridx->limitValue(1, 256);
  view->gridx = atoi(gridx->value());
  view->drawMain(true);
}

void Gui::checkGridY()
{
  gridy->limitValue(1, 256);
  view->gridy = atoi(gridy->value());
  view->drawMain(true);
}

void Gui::checkPaintSize(Widget *, void *var)
{
  Brush *brush = Project::brush.get();

  int size = brush_sizes[*(int *)var];
  int shape = paint_shape->var;

  brush->make(shape, size);
  paint_brush->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR));

  for(int i = 0; i < brush->solid_count; i++)
  {
    paint_brush->bitmap->setpixelSolid(48 + brush->solidx[i],
                                       48 + brush->solidy[i],
                                       getFltkColor(FL_FOREGROUND_COLOR),
                                       0);
  }

  paint_brush->redraw();
}

void Gui::checkPaintShape(Widget *, void *)
{
  paint_size->do_callback();
}

void Gui::checkPaintStroke(Widget *, void *var)
{
  Project::stroke->type = *(int *)var;
}

void Gui::checkPaintEdge(Widget *, void *var)
{
  Project::brush->edge = *(int *)var;
}

void Gui::checkTool(Widget *, void *var)
{
  paint->hide();
  getcolor->hide();
  crop->hide();
  offset->hide();
  text->hide();
  fill->hide();

  switch(*(int *)var)
  {
    case Tool::PAINT:
      Project::setTool(Tool::PAINT);
      paint_brush->do_callback();
      paint_shape->do_callback();
      paint->show();
      updateInfo((char *)"Middle-click to navigate. Mouse wheel zooms. Esc cancels rendering.");
      break;
    case Tool::GETCOLOR:
      Project::setTool(Tool::GETCOLOR);
      getcolor->show();
      updateInfo((char *)"Click to grab a color from the image.");
      break;
    case Tool::CROP:
      Project::setTool(Tool::CROP);
      crop->show();
      updateInfo((char *)"Draw a box, then click inside box to move, outside to change size.");
      break;
    case Tool::OFFSET:
      Project::setTool(Tool::OFFSET);
      offset->show();
      updateInfo((char *)"Click and drag to change image offset.");
      break;
    case Tool::TEXT:
      Project::setTool(Tool::TEXT);
      text->show();
      updateInfo((char *)"Click to stamp text onto the image.");
      break;
    case Tool::FILL:
      Project::setTool(Tool::FILL);
      fill->show();
      updateInfo((char *)"Click to fill an area with the selected color.");
      break;
  }

  Project::tool->reset();
  view->drawMain(true);
}

void Gui::checkColor(Widget *, void *)
{
  int pos = hue->var;
  int mx = pos % 96;
  int my = pos / 96;

  float mouse_angle = atan2f(my - 48, mx - 48);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 48) * 5.43;
  int v = (satval->var / 48) * 5.43;

  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);
  Project::brush->trans = trans->var * 2.685;
  Project::brush->blend = blend->value();

  hue->bitmap->clear(blendFast(getFltkColor(FL_BACKGROUND_COLOR),
                               makeRgb(0, 0, 0), 192));
  satval->bitmap->clear(makeRgb(0, 0, 0));

  for(int i = 1; i < 1536; i++)
  {
    float angle = ((3.14159 * 2) / 1536) * i;
    int x1 = 48 + 44 * std::cos(angle);
    int y1 = 48 + 44 * std::sin(angle);
    int x2 = 48 + 38 * std::cos(angle);
    int y2 = 48 + 38 * std::sin(angle);

    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    hue->bitmap->line(x1, y1, x2, y2, makeRgb(r, g, b), 0);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(r, g, b), 0);
  }

  const int x1 = 48 + 41 * std::cos(mouse_angle);
  const int y1 = 48 + 41 * std::sin(mouse_angle);

  hue->bitmap->xorRect(x1 - 4, y1 - 4, x1 + 4, y1 + 4);

  for(int y = 0; y < 48; y++)
  {
    for(int x = 0; x < 48; x++)
    {
      Blend::hsvToRgb(h, x * 5.43, y * 5.43, &r, &g, &b);
      satval->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
    }
  }

  int x = (satval->var % 48);
  int y = (satval->var / 48);

  if(x < 4)
    x = 4;
  if(y < 4)
    y = 4;
  if(x > 43)
    x = 43;
  if(y > 43)
    y = 43;

  satval->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);

  hue->redraw();
  satval->redraw();

  updateSwatch();
  updateHexColor();
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
  Clone::wrap = *(int *)var;
}

void Gui::checkClone(Widget *, void *var)
{
  Clone::active = *(int *)var;
}

void Gui::checkMirror(Widget *, void *var)
{
  Clone::mirror = *(int *)var;
}

void Gui::checkOrigin(Widget *, void *var)
{
  Project::stroke->origin = *(int *)var;
}

void Gui::checkConstrain(Widget *, void *var)
{
  Project::stroke->constrain = *(int *)var;
}

void Gui::checkCropDo()
{
  Project::tool->done(view);
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

void Gui::textStartOver()
{
  // start text tool over if font changed
  Project::tool->reset();
}

int Gui::getFontFace()
{
  int index = font_browse->value();
  if(index < 1)
    index = 1;

  return index - 1;
}

int Gui::getFontSize()
{
  font_size->limitValue(6, 512);

  return atoi(font_size->value());
}

const char *Gui::getTextInput()
{
  return text_input->value();
}

void Gui::paletteSort()
{
  Project::palette->sort();
  Project::palette->draw(palette);
}

void Gui::paletteDefault()
{
  Project::palette->setDefault();
  Project::palette->draw(palette);
}

void Gui::paletteBlackAndWhite()
{
  Project::palette->setBlackAndWhite();
  Project::palette->draw(palette);
}

void Gui::paletteWebSafe()
{
  Project::palette->setWebSafe();
  Project::palette->draw(palette);
}

void Gui::palette3LevelRGB()
{
  Project::palette->set3LevelRGB();
  Project::palette->draw(palette);
}

void Gui::palette4LevelRGB()
{
  Project::palette->set4LevelRGB();
  Project::palette->draw(palette);
}

void Gui::palette332()
{
  Project::palette->set332();
  Project::palette->draw(palette);
}

void Gui::checkClearToPaintColor()
{
  Undo::push();

  Bitmap *bmp = Project::bmp;

  bmp->rectfill(bmp->cl, bmp->ct, bmp->cr, bmp->cb, Project::brush->color, 0);
  view->drawMain(true);
}

void Gui::checkClearToBlack()
{
  Undo::push();

  Bitmap *bmp = Project::bmp;

  bmp->rectfill(bmp->cl, bmp->ct, bmp->cr, bmp->cb, makeRgb(0, 0, 0), 0);
  view->drawMain(true);
}

void Gui::checkClearToWhite()
{
  Undo::push();

  Bitmap *bmp = Project::bmp;

  bmp->rectfill(bmp->cl, bmp->ct, bmp->cr, bmp->cb, makeRgb(255, 255, 255), 0);
  view->drawMain(true);
}

void Gui::checkClearToTransparent()
{
  Undo::push();

  Bitmap *bmp = Project::bmp;

  for(int y = bmp->ct; y <= bmp->cb; y++)
    for(int x = bmp->cl; x <= bmp->cr; x++)
      *(bmp->row[y] + x) = 0x00808080;

  view->drawMain(true);
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

void Gui::checkPaintMode()
{
  if(paint_mode->value() == 1)
    Project::brush->aa = 1;
  else
    Project::brush->aa = 0;
}

int Gui::getPaintMode()
{
  return paint_mode->value();
}

int Gui::getTextSmooth()
{
  return text_smooth->value();
}

void Gui::showProgress(float step)
{
  view->rendering = true;
  progress_value = 0;
  progress_step = 100.0 / (step / 50);
  // keep progress bar on right side in case window was resized
  progress->resize(status->x() + window->w() - 256 - 8, status->y() + 4, 256, 16);
  info->hide();
  progress->show();
}

int Gui::updateProgress(const int y)
{
  // user cancelled operation
  if(Fl::get_key(FL_Escape))
  {
    hideProgress();
    Gui::getView()->drawMain(true);
    return -1;
  }

  if(!(y % 50))
  {
    view->drawMain(true);
    progress->value(progress_value);
    char percent[16];
    sprintf(percent, "%d%%", (int)progress_value);
    progress->label(percent);
    Fl::check();
    progress_value += progress_step;
  }

  return 0;
}

void Gui::hideProgress()
{
    view->drawMain(true);
    progress->value(0);
    progress->label("");
    progress->redraw();
    progress->hide();
    info->show();
    view->rendering = false;
}

void Gui::updateCoords(char *s)
{
  coords->copy_label(s);
  coords->redraw();
}

void Gui::updateInfo(char *s)
{
  info->copy_label(s);
  info->redraw();
}

