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
#include "DitherMatrix.H"
#include "FX.H"
#include "File.H"
#include "Group.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
#include "InputInt.H"
#include "InputText.H"
#include "Map.H"
#include "Palette.H"
#include "Project.H"
#include "Render.H"
#include "Separator.H"
#include "StaticText.H"
#include "Stroke.H"
#include "ToggleButton.H"
#include "Tool.H"
#include "Transform.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

class MainWin;

namespace
{
  // window
  MainWin *window;

  // main menu
  Fl_Menu_Bar *menubar;

  // containers
  Fl_Group *group_left;
  Fl_Group *group_right;

  // panels
  Group *top;
  Group *tools;
  Group *paint;
  Group *selection;
  Group *getcolor;
  Group *offset;
  Group *text;
  Group *fill;
  Group *group_palette;
  Group *colors;
  Group *bottom;
  Group *status;
  Fl_Group *middle;

  // status
  Fl_Progress *progress;
  Fl_Box *coords;
  Fl_Box *info;

  // top
  ToggleButton *zoom_fit;
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  StaticText *zoom;
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
  Widget *paint_dither_pattern;
  CheckBox *paint_dither_relative;
  Fl_Choice *paint_mode;

  Widget *getcolor_color;

  StaticText *selection_x;
  StaticText *selection_y;
  StaticText *selection_w;
  StaticText *selection_h;
  Fl_Button *selection_crop;
  Fl_Button *selection_select;
  Fl_Button *selection_reset;

  StaticText *offset_x;
  StaticText *offset_y;

  Fl_Hold_Browser *font_browse;
  InputInt *font_size;
  Fl_Input *text_input;
  CheckBox *text_smooth;

  InputInt *fill_range;

  // colors
  Widget *palette;
  Widget *swatch;
  Widget *hue;
  Widget *satval;
  InputText *hexcolor;
  Widget *trans;
  Fl_Choice *blend;
  Fl_Choice *alpha_mask;

  // bottom
  ToggleButton *wrap;
  ToggleButton *clone;
  Widget *mirror;
  ToggleButton *origin;
  ToggleButton *constrain;

  // view
  View *view;

  // progress indicator related
  float progress_value = 0;
  float progress_step = 0;

  // tables
  const int brush_sizes[16] =
  {
    1, 2, 3, 4, 8, 12, 16, 24,
    32, 40, 48, 56, 64, 72, 80, 88
  };

  // quit program
  void quit()
  {
    if(Dialog::choice("Quit", "Are You Sure?"))
      exit(0);
  }

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
      if(Dialog::choice("Quit", "Are You Sure?"))
        widget->hide();
    }
  }
}

// custom class to control window behavior
class MainWin : public Fl_Double_Window
{
public:
  MainWin(int w, int h, const char *label)
  : Fl_Double_Window(w, h, label)
  {
  }

  ~MainWin()
  {
  }
  
  int handle(int event)
  {
    View *view = Gui::getView();
    bool shift, ctrl;

    switch(event)
    {
      case FL_FOCUS:
        return 1;
      case FL_UNFOCUS:
        return 1;
      case FL_KEYBOARD:
        // give focus to the main menu
        if(Fl::event_alt() > 0)
        {
          Gui::getMenuBar()->take_focus();
          return 0;
        }

        shift = Fl::event_shift() ? true : false;
        ctrl = Fl::event_ctrl() ? true : false;

        // cancel current rendering operation
        if(Fl::event_key() == FL_Escape)
        {
          Project::tool->reset();
          view->drawMain(true);
          break;
        }

        // misc keys
        switch(Fl::event_key())
        {
          case FL_Right:
            view->scroll(0, 64);
            break;
          case FL_Left:
            view->scroll(1, 64);
            break;
          case FL_Down:
            view->scroll(2, 64);
            break;
          case FL_Up:
            view->scroll(3, 64);
            break;
          case 'z':
            if(ctrl && shift)
              Undo::popRedo();
            else if(ctrl)
              Undo::pop();
            break;
          case 'e':
            if(ctrl)
              Dialog::editor();
            break;
        }

        return 1;
    }

    return Fl_Double_Window::handle(event);
  }
};

// initialize main gui
void Gui::init()
{
  int x1, y1;

  // main window
  window = new MainWin(800, 600, "Rendera");
  window->callback(closeCallback);

  // generate menu
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
    (Fl_Callback *)Undo::pop, 0, 0);
  menubar->add("&Edit/Redo (Shift+Ctrl+Z)", 0,
    (Fl_Callback *)Undo::popRedo, 0, FL_MENU_DIVIDER);
  menubar->add("&Edit/Clear/Transparent", 0,
    (Fl_Callback *)checkClearToTransparent, 0, 0);
  menubar->add("&Edit/Clear/Paint Color", 0,
    (Fl_Callback *)checkClearToPaintColor, 0, 0);
  menubar->add("&Edit/Clear/Black", 0,
    (Fl_Callback *)checkClearToBlack, 0, 0);
  menubar->add("&Edit/Clear/White", 0,
    (Fl_Callback *)checkClearToWhite, 0, 0);

  menubar->add("&Image/Flip &Horizontal", 0,
    (Fl_Callback *)Transform::flipHorizontal, 0, 0);
  menubar->add("&Image/Flip &Vertical", 0,
    (Fl_Callback *)Transform::flipVertical, 0, FL_MENU_DIVIDER);
  menubar->add("&Image/Resize...", 0,
    (Fl_Callback *)Transform::resize, 0, 0);
  menubar->add("&Image/Scale...", 0,
    (Fl_Callback *)Transform::scale, 0, 0);
  menubar->add("&Image/Rotate...", 0,
    (Fl_Callback *)Transform::rotate, 0, 0);

  menubar->add("&Selection/Flip &Horizontal", 0,
    (Fl_Callback *)checkSelectionFlipHorizontal, 0, 0);
  menubar->add("&Selection/Flip &Vertical", 0,
    (Fl_Callback *)checkSelectionFlipVertical, 0, 0);

  menubar->add("&Palette/&Open...", 0,
    (Fl_Callback *)File::loadPalette, 0, 0);
  menubar->add("&Palette/&Save...", 0,
    (Fl_Callback *)File::savePalette, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Create From Image...", 0,
    (Fl_Callback *)Dialog::makePalette, 0, 0);
  menubar->add("&Palette/&Dither Image...", 0,
    (Fl_Callback *)FX::ditherImage, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/Presets/Default", 0,
    (Fl_Callback *)paletteDefault, 0, 0);
  menubar->add("Palette/Presets/Black and White", 0,
    (Fl_Callback *)paletteBlackAndWhite, 0, 0);
  menubar->add("&Palette/Presets/Web Safe", 0,
    (Fl_Callback *)paletteWebSafe, 0, 0);
  menubar->add("&Palette/Presets/3-level RGB", 0,
    (Fl_Callback *)palette3LevelRGB, 0, 0);
  menubar->add("&Palette/Presets/4-level RGB", 0,
    (Fl_Callback *)palette4LevelRGB, 0, 0);
  menubar->add("&Palette/Presets/3-3-2", 0,
    (Fl_Callback *)palette332, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Editor... (Ctrl+E)", 0,
    (Fl_Callback *)Dialog::editor, 0, 0);

  menubar->add("F&X/Color/Normalize", 0,
    (Fl_Callback *)FX::normalize, 0, 0);
  menubar->add("F&X/Color/Equalize", 0,
    (Fl_Callback *)FX::equalize, 0, 0);
  menubar->add("F&X/Color/Value Stretch", 0,
    (Fl_Callback *)FX::valueStretch, 0, 0);
  menubar->add("F&X/Color/Saturate", 0,
    (Fl_Callback *)FX::saturate, 0, 0);
  menubar->add("F&X/Color/Rotate Hue...", 0,
    (Fl_Callback *)FX::rotateHue, 0, 0);
  menubar->add("F&X/Color/Invert", 0,
    (Fl_Callback *)FX::invert, 0, 0);
  menubar->add("F&X/Color/Invert Alpha", 0,
    (Fl_Callback *)FX::invertAlpha, 0, 0);
  menubar->add("F&X/Color/Desaturate", 0,
    (Fl_Callback *)FX::desaturate, 0, 0);
  menubar->add("F&X/Color/Colorize", 0,
    (Fl_Callback *)FX::colorize, 0, 0);
  menubar->add("F&X/Filter/Gaussian Blur...", 0,
    (Fl_Callback *)FX::gaussianBlur, 0, 0);
  menubar->add("F&X/Filter/Sharpen...", 0,
    (Fl_Callback *)FX::sharpen, 0, 0);
  menubar->add("F&X/Filter/Unsharp Mask...", 0,
    (Fl_Callback *)FX::unsharpMask, 0, 0);
  menubar->add("F&X/Filter/Convolution Matrix...", 0,
    (Fl_Callback *)FX::convolutionMatrix, 0, 0);
  menubar->add("F&X/Photo/Auto-Correct...", 0,
    (Fl_Callback *)FX::autoCorrect, 0, 0);
  menubar->add("F&X/Photo/Correction Matrix", 0,
    (Fl_Callback *)FX::correctionMatrix, 0, 0);
  menubar->add("F&X/Photo/Restore...", 0,
    (Fl_Callback *)FX::restore, 0, 0);
  menubar->add("F&X/Photo/Remove Dust...", 0,
    (Fl_Callback *)FX::removeDust, 0, 0);
  menubar->add("F&X/Artistic/Stained Glass...", 0,
    (Fl_Callback *)FX::stainedGlass, 0, 0);
  menubar->add("F&X/Artistic/Painting...", 0,
    (Fl_Callback *)FX::painting, 0, 0);
  menubar->add("F&X/FFT/Forward FFT", 0,
    (Fl_Callback *)FX::forwardFFT, 0, 0);
  menubar->add("F&X/FFT/Inverse FFT", 0,
    (Fl_Callback *)FX::inverseFFT, 0, 0);

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

  // top
  top = new Group(0, menubar->h(), window->w(), 40, "");
  x1 = 8;
  zoom_fit = new ToggleButton(top, x1, 8, 24, 24,
                              "Fit In Window", images_zoom_fit_png,
                              (Fl_Callback *)checkZoomFit);
  x1 += 24 + 8;
  zoom_one = new Button(top, x1, 8, 24, 24,
                        "Actual Size", images_zoom_one_png,
                        (Fl_Callback *)checkZoomOne);
  x1 += 24 + 8;
  zoom_in = new Button(top, x1, 8, 24, 24,
                       "Zoom In", images_zoom_in_png,
                       (Fl_Callback *)checkZoomIn);
  x1 += 24 + 8;
  zoom_out = new Button(top, x1, 8, 24, 24,
                        "Zoom Out", images_zoom_out_png,
                        (Fl_Callback *)checkZoomOut);
  x1 += 24 + 8;
  zoom = new StaticText(top, x1, 8, 56, 24, "");
  x1 += 56 + 6;
  new Separator(top, x1, 2, 2, 36, "");
  x1 += 8;
  grid = new ToggleButton(top, x1, 8, 24, 24,
                          "Show Grid", images_grid_png,
                          (Fl_Callback *)checkGrid);
  x1 += 24 + 48 + 8;
  gridx = new InputInt(top, x1, 8, 64, 24,
                       "Grid X:",
                       (Fl_Callback *)checkGridX, 1, 256);
  gridx->value("8");
  x1 += 64 + 48 + 8;
  gridy = new InputInt(top, x1, 8, 64, 24,
                       "Grid Y:",
                       (Fl_Callback *)checkGridY, 1, 256);
  gridy->value("8");
  top->resizable(0);
  top->end();

  // bottom
  bottom = new Group(160, window->h() - status->h() - 40, window->w() - 272 - 80, 40, "");
  x1 = 8;
  wrap = new ToggleButton(bottom, x1, 8, 24, 24,
                          "Wrap Edges", images_wrap_png,
                          (Fl_Callback *)checkWrap);
  x1 += 24 + 6;
  new Separator(bottom, x1, 2, 2, 36, "");
  x1 += 8;
  clone = new ToggleButton(bottom, x1, 8, 24, 24,
                           "Clone Enable (Shift+Click sets clone target!)",
                           images_clone_png,
                           (Fl_Callback *)checkClone);
  x1 += 24 + 8;
  mirror = new Widget(bottom, x1, 8, 96, 24,
                      "Clone Mirroring", images_mirror_png, 24, 24,
                      (Fl_Callback *)checkMirror);
  x1 += 96 + 6;
  new Separator(bottom, x1, 2, 2, 36, "");
  x1 += 8;
  origin = new ToggleButton(bottom, x1, 8, 24, 24,
                            "Start Shape From Center", images_origin_png,
                            (Fl_Callback *)checkOrigin);
  x1 += 24 + 8;
  constrain = new ToggleButton(bottom, x1, 8, 24, 24,
                              "Lock Shape Proportions",
                              images_constrain_png,
                              (Fl_Callback *)checkConstrain);
  bottom->resizable(0);
  bottom->end();

  // tools
  tools = new Group(0, top->h() + menubar->h(),
                    48, window->h() - (menubar->h() + top->h() + status->h()),
                    "Tools");
  y1 = 20;
  tool = new Widget(tools, 8, y1, 32, 192,
                    "Tools", images_tools_png, 32, 32,
                    (Fl_Callback *)checkTool);
  y1 += 96 + 8;
  tools->resizable(0);
  tools->end();

  // paint
  paint = new Group(48, top->h() + menubar->h(),
                    112, window->h() - top->h() - menubar->h() - status->h(),
                    "Painting");
  y1 = 20;
  paint_brush = new Widget(paint, 8, y1, 96, 96,
                           "Brush Preview", 0, 0, 0);
  paint_brush->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR));
  paint_brush->bitmap->setpixelSolid(48, 48, makeRgb(192, 192, 192), 0);
  y1 += 96 + 8;
  paint_size = new Widget(paint, 8, y1, 96, 24,
                          "Size", images_size_png, 6, 24,
                          (Fl_Callback *)checkPaintSize);
  y1 += 24 + 8;
  paint_stroke = new Widget(paint, 8, y1, 96, 48,
                            "Stroke", images_stroke_png, 24, 24,
                            (Fl_Callback *)checkPaintStroke);
  y1 += 48 + 8;
  paint_shape = new Widget(paint, 8, y1, 96, 24,
                           "Shape", images_shape_png, 24, 24,
                           (Fl_Callback *)checkPaintShape);
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
  paint_edge = new Widget(paint, 8, y1, 96, 24,
                          "Soft Edge", images_soft_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintEdge);
  paint_dither_pattern = new Widget(paint, 8, y1, 96, 48,
                                    "Dither Pattern", 24, 24,
                                    (Fl_Callback *)0);
  y1 += 48 + 8;
  paint_dither_relative = new CheckBox(paint, 12, y1, 16, 16, "Relative", 0);
  y1 += 16 + 8;
  paint->resizable(0);
  paint->end();

  // selection
  selection = new Group(48, top->h() + menubar->h(),
                   112, window->h() - top->h() - menubar->h() - status->h(),
                   "Selection");
  y1 = 20;
  new StaticText(selection, 8, y1, 32, 24, "X:");
  selection_x = new StaticText(selection, 24, y1, 72, 24, 0);
  y1 += 24 + 6;
  new StaticText(selection, 8, y1, 32, 24, "Y:");
  selection_y = new StaticText(selection, 24, y1, 72, 24, 0);
  y1 += 24 + 6;
  new StaticText(selection, 8, y1, 32, 24, "W:");
  selection_w = new StaticText(selection, 24, y1, 72, 24, 0);
  y1 += 24 + 6;
  new StaticText(selection, 8, y1, 32, 24, "H:");
  selection_h = new StaticText(selection, 24, y1, 72, 24, 0);
  y1 += 24 + 6;
  new Separator(selection, 2, y1, 110, 2, "");
  y1 += 8;
  selection_crop = new Fl_Button(selection->x() + 8, selection->y() + y1, 96, 32, "Crop");
  selection_crop->callback((Fl_Callback *)checkSelectionCrop);
  y1 += 32 + 8;
  selection_select = new Fl_Button(selection->x() + 8, selection->y() + y1, 96, 32, "Select");
  selection_select->callback((Fl_Callback *)checkSelectionSelect);
  y1 += 32 + 6;
  new Separator(selection, 2, y1, 110, 2, "");
  y1 += 8;
  selection_reset = new Fl_Button(selection->x() + 8, selection->y() + y1, 96, 32, "Reset");
  selection_reset->callback((Fl_Callback *)checkSelectionReset);
  y1 += 32 + 8;
  selection->resizable(0);
  selection->end();

  // getcolor
  getcolor = new Group(48, top->h() + menubar->h(),
                       112, window->h() - top->h() - menubar->h() - status->h(),
                       "GetColor");
  y1 = 20;
  getcolor_color = new Widget(getcolor, 8, y1, 96, 96, "Color", 0, 0, 0);
  getcolor->resizable(0);
  getcolor->end();

  // offset
  offset = new Group(48, top->h() + menubar->h(),
                     112, window->h() - top->h() - menubar->h() - status->h(),
                     "Offset");
  y1 = 20;
  new StaticText(offset, 8, y1, 32, 24, "X:");
  offset_x = new StaticText(offset, 24, y1, 72, 24, 0);
  y1 += 24 + 6;
  new StaticText(offset, 8, y1, 32, 24, "Y:");
  offset_y = new StaticText(offset, 24, y1, 72, 24, 0);
  y1 += 24 + 6;
  offset->resizable(0);
  offset->end();

  // text
  text = new Group(48, top->h() + menubar->h(),
                   112, window->h() - top->h() - menubar->h() - status->h(),
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
                           (Fl_Callback *)textStartOver, 4, 512);
  font_size->value("12");
  y1 += 24 + 8;

  text_input = new Fl_Input(8, y1, 96, 24, "");
  text_input->textsize(10);
  text_input->value("Text");
  text_input->resize(text->x() + 8, text->y() + y1, 96, 24);
  text_input->callback((Fl_Callback *)textStartOver);
  y1 += 24 + 8;

  text_smooth = new CheckBox(text, 12, y1, 16, 16, "Smooth", 0);
  text_smooth->center();
  text_smooth->value(1);

  text->resizable(0);
  text->end();

  // fill
  fill = new Group(48, top->h() + menubar->h(),
                   112, window->h() - top->h() - menubar->h() - status->h(),
                   "Fill");
  y1 = 40;
  fill_range = new InputInt(fill, 8, y1, 96, 24,
                            "Range:",
                            0, 0, 100);
  fill_range->align(FL_ALIGN_TOP);
  fill_range->value("0");
  fill->end();

  // group_palette
  group_palette = new Group(window->w() - 112 - 80, top->h() + menubar->h(),
                    80, window->h() - top->h() - menubar->h() - status->h(),
                    "Palette");
  y1 = 20;
  palette = new Widget(group_palette, 8, y1, 64, 256,
                       "Color Palette", 8, 8,
                       (Fl_Callback *)checkPalette);
  y1 += 96 + 8;

  group_palette->resizable(0);
  group_palette->end();

  // colors
  colors = new Group(window->w() - 112, top->h() + menubar->h(),
                    112, window->h() - top->h() - menubar->h() - status->h(),
                    "Colors");
  y1 = 20;
  swatch = new Widget(colors, 8, y1, 96, 48,
                      "Color Swatch", 0, 0,
                      0);
  y1 += 48 + 8;
  hexcolor = new InputText(colors, 40, y1, 64, 24, "Hex:",
                           (Fl_Callback *)checkHexColor);
  hexcolor->maximum_size(6);
  hexcolor->textsize(14);
  y1 += 24 + 8;
  // satval overlaps the hue color wheel
  hue = new Widget(colors, 8, y1, 96, 96,
                   "Hue", 1, 1,
                   (Fl_Callback *)checkHue);
  satval = new Widget(colors, 32, y1 + 24, 48, 48,
                      "Saturation/Value", 1, 1,
                      (Fl_Callback *)checkSatVal);
  y1 += 96 + 8;
  trans = new Widget(colors, 8, y1, 96, 24,
                     "Transparency", images_transparency_png, 1, 24,
                     (Fl_Callback *)checkTrans);
  y1 += 24 + 8;
  blend = new Fl_Choice(8, y1, 96, 24, "");
  blend->tooltip("Blending Mode");
  blend->textsize(10);
  blend->resize(colors->x() + 8, colors->y() + y1, 96, 24);
  blend->add("Normal");
  blend->add("Darken");
  blend->add("Lighten");
  blend->add("Colorize");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->add("Smooth (Color)");
  blend->add("Smooth (Luminosity)");
  blend->add("Sharpen");
  blend->value(0);
  blend->callback((Fl_Callback *)checkBlend);
  y1 += 24 + 8;
  alpha_mask = new Fl_Choice(8, y1, 96, 24, "");
  alpha_mask->tooltip("Masking Mode");
  alpha_mask->textsize(10);
  alpha_mask->resize(colors->x() + 8, colors->y() + y1, 96, 24);
  alpha_mask->add("None");
  alpha_mask->add("Alpha");
  alpha_mask->add("Reverse Alpha");
  alpha_mask->value(0);
  alpha_mask->callback((Fl_Callback *)checkAlphaMask);
  y1 += 16 + 8;

  colors->resizable(0);
  colors->end();

  // middle
  middle = new Fl_Group(160, top->h() + menubar->h(),
                        window->w() - 272 - 80, window->h() - (menubar->h() + top->h() + bottom->h() + status->h()));
  middle->box(FL_FLAT_BOX);
  view = new View(middle, 0, 0, middle->w(), middle->h(), "View");
  middle->resizable(view);
  middle->end();

  // container for left panels
  group_left = new Fl_Group(0, top->h() + menubar->h(),
                            160, window->h() - (menubar->h() + top->h() + bottom->h()));
  group_left->add(tools);
  group_left->add(paint);
  group_left->add(getcolor);
  group_left->add(selection);
  group_left->add(offset);
  group_left->add(text);
  group_left->end();

  // container for right panels
  group_right = new Fl_Group(window->w() - 112 - 80, top->h() + menubar->h(),
                            112 + 80, window->h() - (menubar->h() + top->h() + bottom->h()));
  group_right->add(group_palette);
  group_right->add(colors);

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
  checkSelectionValues(0, 0, 0, 0);
  checkOffsetValues(0, 0);
  checkPaintMode();

  // create dither pattern image from data in DitherMatrix.H
  paint_dither_pattern->bitmap->clear(makeRgb(192, 192, 192));

  for(int z = 0; z < 8; z++)
  {
    for(int y = 0; y < 4; y++)
    {
      const int yy = ((z / 4) * 4 + y) * 6;
      for(int x = 0; x < 4; x++)
      {
        const int xx = ((z & 3) * 4 + x) * 6;
        if(DitherMatrix::pattern[z][y][x] == 1)
          paint_dither_pattern->bitmap->rectfill(xx, yy, xx + 5, yy + 5,
                                         makeRgb(0, 0, 0), 0);
      }
    }
  }

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

// show the main program window (called after gui is constructed)
void Gui::show()
{
  window->show();
}

// draw checkmark next to a menu item
void Gui::setMenuItem(const char *s)
{
  Fl_Menu_Item *m;
  m = (Fl_Menu_Item *)menubar->find_item(s);

  if(m)
    m->set();
}

// remove checkmark from menu item
void Gui::clearMenuItem(const char *s)
{
  Fl_Menu_Item *m;
  m = (Fl_Menu_Item *)menubar->find_item(s);

  if(m)
    m->clear();
}

// begin callback functions
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
  char s[256];
  snprintf(s, sizeof(s), "%2.3f", view->zoom);
  zoom->copy_label(s);
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
  view->gridx = atoi(gridx->value());
  view->drawMain(true);
}

void Gui::checkGridY()
{
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
  selection->hide();
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
    case Tool::KNIFE:
      Project::setTool(Tool::KNIFE);
      selection->show();
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

  view->ignore_tool = true;
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

void Gui::checkAlphaMask()
{
  Project::brush->alpha_mask = alpha_mask->value();
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

void Gui::checkSelectionCrop()
{
  Project::tool->done(view, 0);
}

void Gui::checkSelectionSelect()
{
  Project::tool->done(view, 1);
}

void Gui::checkSelectionFlipHorizontal()
{
  Project::select_bmp->flipHorizontal();
}

void Gui::checkSelectionFlipVertical()
{
  Project::select_bmp->flipVertical();
}

void Gui::checkSelectionReset()
{
  Project::tool->done(view, 2);
}

void Gui::checkSelectionValues(int x, int y, int w, int h)
{
  char s[256];

  snprintf(s, sizeof(s), "(%d)", x);
  selection_x->copy_label(s);
  selection_x->redraw();

  snprintf(s, sizeof(s), "(%d)", y);
  selection_y->copy_label(s);
  selection_y->redraw();

  snprintf(s, sizeof(s), "(%d)", w);
  selection_w->copy_label(s);
  selection_w->redraw();

  snprintf(s, sizeof(s), "(%d)", h);
  selection_h->copy_label(s);
  selection_h->redraw();
}

void Gui::checkOffsetValues(int x, int y)
{
  char s[256];

  snprintf(s, sizeof(s), "(%d)", x);
  offset_x->copy_label(s);
  offset_x->redraw();

  snprintf(s, sizeof(s), "(%d)", y);
  offset_y->copy_label(s);
  offset_y->redraw();
}

// start text tool over if font changed
void Gui::textStartOver()
{
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
  return atoi(font_size->value());
}

const char *Gui::getTextInput()
{
  return text_input->value();
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

Fl_Double_Window *Gui::getWindow()
{
  return window;
}

Fl_Menu_Bar *Gui::getMenuBar()
{
  return menubar;
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
  Project::brush->aa = 0;
  paint_edge->hide();
  paint_dither_pattern->hide();
  paint_dither_relative->hide();

  switch(paint_mode->value())
  {
    case Render::SOLID:
      paint_dither_pattern->show();
      paint_dither_relative->show();
      break;
    case Render::ANTIALIASED:
      Project::brush->aa = 1;
      break;
    case Render::COARSE:
      paint_edge->show();
      break;
    case Render::FINE:
      paint_edge->show();
      break;
    case Render::BLUR:
      paint_edge->show();
      break;
    case Render::WATERCOLOR:
      paint_edge->show();
      break;
    case Render::CHALK:
      paint_edge->show();
      break;
  }
}

int Gui::getPaintMode()
{
  return paint_mode->value();
}

int Gui::getTextSmooth()
{
  return text_smooth->value();
}

int Gui::getFillRange()
{
  return atoi(fill_range->value());
}

int Gui::getDitherPattern()
{
  return paint_dither_pattern->var;
}

int Gui::getDitherRelative()
{
  return paint_dither_relative->value();
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
    progress->copy_label(percent);
    Fl::check();
    progress_value += progress_step;
  }

  return 0;
}

void Gui::hideProgress()
{
    view->drawMain(true);
    progress->value(0);
    progress->copy_label("");
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

