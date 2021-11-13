/*
Copyright (c) 2021 Joe Davisson.

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
#include <cstdio>
#include <typeinfo>

#include <FL/fl_draw.H>
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
#include "FX2.H"
#include "File.H"
#include "Gamma.H"
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
#include "RepeatButton.H"
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
  Fl_Group *left;
  Fl_Group *right;

  // panels
  Group *top;
  Group *tools;
  Group *paint;
  Group *selection;
  Group *getcolor;
  Group *offset;
  Group *text;
  Group *fill;
  Group *palette;
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
  Widget *paint_brush_preview;
  Widget *paint_size;
  InputInt *paint_size_value;
  Widget *paint_stroke;
  Widget *paint_shape;
  Widget *paint_coarse_edge;
  Widget *paint_fine_edge;
  Widget *paint_blurry_edge;
  Widget *paint_watercolor_edge;
  Widget *paint_chalk_edge;
  Widget *paint_texture_edge;
  Widget *paint_texture_marb;
  Widget *paint_texture_turb;
  Widget *paint_average_edge;
//  Widget *paint_dither_pattern;
//  CheckBox *paint_dither_relative;
  Fl_Choice *paint_mode;
  Widget *getcolor_color;
  InputInt *fill_feather;

  StaticText *selection_x;
  StaticText *selection_y;
  StaticText *selection_w;
  StaticText *selection_h;
  Fl_Button *selection_crop;
  Fl_Button *selection_select;
  Fl_Button *selection_reset;
  Button *selection_flip;
  Button *selection_mirror;
  Button *selection_rotate;

  StaticText *offset_x;
  StaticText *offset_y;
  RepeatButton *offset_up;
  RepeatButton *offset_left;
  RepeatButton *offset_right;
  RepeatButton *offset_down;
//  Button *offset_undo;

  Fl_Hold_Browser *font_browse;
  InputInt *font_size;
  Fl_Input *text_input;
  CheckBox *text_smooth;

  // palette
//  Palette *undo_palette;
//  bool begin_palette_undo = false;
  Widget *palette_swatches;
  Fl_Button *palette_editor;
//  Fl_Button *palette_insert;
//  Fl_Button *palette_delete;
//  Fl_Button *palette_replace;
//  Fl_Button *palette_undo;

  // colors
  Widget *swatch;
  Widget *hue;
  Widget *satval;
  InputText *hexcolor;
  Widget *trans;
  Fl_Choice *blend;
  Widget *range;
  Bitmap *range_buf;
//  CheckBox *alpha_mask;

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
    1, 2, 3, 4, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72
  };

  // quit program
  void quit()
  {
    if(Dialog::choice("Exit", "Exit Program?"))
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
      if(Dialog::choice("Exit", "Are You Sure?"))
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
        switch(/*key =*/ Fl::event_key())
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
/*
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
            trans->var = (key - '1') * 2;
            trans->do_callback();
            colors->redraw();
            break;
          case '9':
            trans->var = 15;
            trans->do_callback();
            colors->redraw();
            break;
*/
          case '1':
            zoom_fit->var = 0;
            zoom_fit->redraw();
            view->zoomOne();
            break;
          case 'f':
            zoom_fit->var ^= 1;
            zoom_fit->redraw();
            view->zoomFit(zoom_fit->var);
            break;
          case '+':
          case '=':
            view->zoomIn(view->w() / 2, view->h() / 2);
            break;
          case '-':
            view->zoomOut(view->w() / 2, view->h() / 2);
            break;
          case 'z':
            if(ctrl && shift)
              Undo::popRedo();
            else if(ctrl)
              Undo::pop();
            break;
          case 'e':
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
  int pos;

  // main window
  window = new MainWin(1024, 768, "Rendera");
  window->callback(closeCallback);

  // generate menu
  menubar = new Fl_Menu_Bar(0, 0, window->w(), 24);
  menubar->box(FL_THIN_UP_BOX);
  menubar->color(FL_INACTIVE_COLOR);

  menubar->add("&File/New...", 0,
    (Fl_Callback *)Dialog::newImage, 0, 0);
  menubar->add("&File/&Open...", 0,
    (Fl_Callback *)File::load, 0, 0);
  menubar->add("&File/&Save...", 0,
    (Fl_Callback *)File::save, 0, FL_MENU_DIVIDER);
  menubar->add("&File/E&xit...", 0,
    (Fl_Callback *)quit, 0, 0);

  menubar->add("&Edit/Undo (Ctrl+Z)", 0,
    (Fl_Callback *)Undo::pop, 0, 0);
  menubar->add("&Edit/Redo (Shift+Ctrl+Z)", 0,
    (Fl_Callback *)Undo::popRedo, 0);
  menubar->add("&Clear/&Black", 0,
    (Fl_Callback *)checkClearToBlack, 0, 0);
  menubar->add("&Clear/&White", 0,
    (Fl_Callback *)checkClearToWhite, 0, 0);
  menubar->add("&Clear/&Gray", 0,
    (Fl_Callback *)checkClearToGray, 0, 0);
  menubar->add("&Clear/&Paint Color", 0,
    (Fl_Callback *)checkClearToPaintColor, 0, 0);
  menubar->add("&Clear/&Transparent", 0,
    (Fl_Callback *)checkClearToTransparent, 0, 0);

  menubar->add("&Image/&Resize...", 0,
    (Fl_Callback *)Transform::resize, 0, 0);
  menubar->add("&Image/&Scale...", 0,
    (Fl_Callback *)Transform::scale, 0, FL_MENU_DIVIDER);
  menubar->add("&Image/Flip &Horizontal", 0,
    (Fl_Callback *)Transform::flipHorizontal, 0, 0);
  menubar->add("&Image/Flip &Vertical", 0,
    (Fl_Callback *)Transform::flipVertical, 0, FL_MENU_DIVIDER);
  menubar->add("&Image/Rotate/90 Degrees", 0,
    (Fl_Callback *)Transform::rotate90, 0, 0);
  menubar->add("&Image/Rotate/180 Degrees", 0,
    (Fl_Callback *)Transform::rotate180, 0, 0);
  menubar->add("&Image/Rotate/Arbitrary...", 0,
    (Fl_Callback *)Transform::rotateArbitrary, 0, 0);

  menubar->add("&Palette/&Open...", 0,
    (Fl_Callback *)File::loadPalette, 0, 0);
  menubar->add("&Palette/&Save...", 0,
    (Fl_Callback *)File::savePalette, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Create...", 0,
    (Fl_Callback *)Dialog::makePalette, 0, 0);
  menubar->add("&Palette/&Apply...", 0,
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
  menubar->add("&Palette/&Editor... (E)", 0,
    (Fl_Callback *)Dialog::editor, 0, 0);

//  menubar->add("F&X/Test", 0,
//    (Fl_Callback *)FX::test, 0, 0);
  menubar->add("F&X/Color/Normalize", 0,
    (Fl_Callback *)FX::normalize, 0, 0);
  menubar->add("F&X/Color/Equalize", 0,
    (Fl_Callback *)FX::equalize, 0, 0);
  menubar->add("F&X/Color/Value Stretch", 0,
    (Fl_Callback *)FX::valueStretch, 0, 0);
  menubar->add("F&X/Color/Color Stretch", 0,
    (Fl_Callback *)FX::saturate, 0, 0);
  menubar->add("F&X/Color/Rotate Hue...", 0,
    (Fl_Callback *)FX::rotateHue, 0, 0);
  menubar->add("F&X/Color/Desaturate", 0,
    (Fl_Callback *)FX::desaturate, 0, 0);
  menubar->add("F&X/Color/Colorize", 0,
    (Fl_Callback *)FX::colorize, 0, 0);
  menubar->add("F&X/Color/Palette Colors", 0,
    (Fl_Callback *)FX::paletteColors, 0, 0);
  menubar->add("F&X/Color/Invert", 0,
    (Fl_Callback *)FX::invert, 0, 0);
  menubar->add("F&X/Color/Invert Alpha", 0,
    (Fl_Callback *)FX::invertAlpha, 0, 0);

  menubar->add("F&X/Filters/Gaussian Blur...", 0,
    (Fl_Callback *)FX::gaussianBlur, 0, 0);
  menubar->add("F&X/Filters/Sharpen...", 0,
    (Fl_Callback *)FX::sharpen, 0, 0);
  menubar->add("F&X/Filters/Unsharp Mask...", 0,
    (Fl_Callback *)FX::unsharpMask, 0, 0);
  menubar->add("F&X/Filters/Box Filters...", 0,
    (Fl_Callback *)FX::convolutionMatrix, 0, 0);
  menubar->add("F&X/Filters/Sobel...", 0,
    (Fl_Callback *)FX::sobel, 0, 0);
  menubar->add("F&X/Filters/Bloom...", 0,
    (Fl_Callback *)FX::bloom, 0, 0);

  menubar->add("F&X/Photo/Restore...", 0,
    (Fl_Callback *)FX::restore, 0, 0);
  menubar->add("F&X/Photo/Side Absorptions", 0,
    (Fl_Callback *)FX::sideAbsorptions, 0, 0);
  menubar->add("F&X/Photo/Remove Dust...", 0,
    (Fl_Callback *)FX::removeDust, 0, 0);

  menubar->add("F&X/Artistic/Stained Glass...", 0,
    (Fl_Callback *)FX::stainedGlass, 0, 0);
  menubar->add("F&X/Artistic/Painting...", 0,
    (Fl_Callback *)FX::painting, 0, 0);
  menubar->add("F&X/Artistic/Marble...", 0,
    (Fl_Callback *)FX2::marble, 0, 0);

  menubar->add("F&X/FFT/Forward FFT", 0,
    (Fl_Callback *)FX2::forwardFFT, 0, 0);
  menubar->add("F&X/FFT/Inverse FFT", 0,
    (Fl_Callback *)FX2::inverseFFT, 0, 0);

  menubar->add("&Help/&About...", 0,
    (Fl_Callback *)Dialog::about, 0, 0);

  // initialize undo palette
//  undo_palette = new Palette();

  // status
  status = new Group(0, window->h() - 24, window->w(), 24, "");
  pos = 8;
  coords = new Fl_Box(FL_FLAT_BOX, pos, 4, 96, 16, "");

  coords->resize(status->x() + pos, status->y() + 4, 96, 16);
  coords->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  coords->copy_label("(0, 0)");
  pos += 96 + 6;
  new Separator(status, pos, 4, 2, 18, "");
  pos += 8;
  info = new Fl_Box(FL_FLAT_BOX, pos, 4, window->w() - pos, 16, "");
  info->resize(status->x() + pos, status->y() + 4, window->w() - pos, 16);
  info->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  info->copy_label("Welcome to Rendera!");
  progress = new Fl_Progress(pos, window->w() - 256 - 8, 256, 16);
  progress->resize(status->x() + window->w() - 256 - 8, status->y() + 4, 256, 16);
  progress->minimum(0);
  progress->maximum(100);
  progress->color(0x44444400);
  progress->selection_color(0xBBBBBB00);
  progress->labelcolor(0xFFFFFF00);
  progress->hide();
  status->resizable(0);
  status->end();

  // top
  top = new Group(0, menubar->h(), window->w(), 40, "");
  pos = 8;
  zoom_fit = new ToggleButton(top, pos, 8, 24, 24,
                              "Fit In Window (F)", images_zoom_fit_png,
                              (Fl_Callback *)checkZoomFit);
  pos += 24 + 8;
  zoom_one = new Button(top, pos, 8, 24, 24,
                        "Actual Size (1)", images_zoom_one_png,
                        (Fl_Callback *)checkZoomOne);
  pos += 24 + 8;
  zoom_in = new Button(top, pos, 8, 24, 24,
                       "Zoom In (+)", images_zoom_in_png,
                       (Fl_Callback *)checkZoomIn);
  pos += 24 + 8;
  zoom_out = new Button(top, pos, 8, 24, 24,
                        "Zoom Out (-)", images_zoom_out_png,
                        (Fl_Callback *)checkZoomOut);
  pos += 24 + 8;
  zoom = new StaticText(top, pos, 8, 56, 24, "");
  pos += 56 + 6;
  new Separator(top, pos, 4, 2, 34, "");
  pos += 8;
  grid = new ToggleButton(top, pos, 8, 24, 24,
                          "Show Grid", images_grid_png,
                          (Fl_Callback *)checkGrid);
  pos += 50;
  gridx = new InputInt(top, pos, 8, 64, 24,
                       "X:",
                       (Fl_Callback *)checkGridX, 1, 256);
  gridx->value("8");
  pos += 92;
  gridy = new InputInt(top, pos, 8, 64, 24,
                       "Y:",
                       (Fl_Callback *)checkGridY, 1, 256);
  gridy->value("8");
  top->resizable(0);
  top->end();

  // bottom
  bottom = new Group(160, window->h() - status->h() - 40, window->w() - 272 - 80, 40, "");
  pos = 8;
  wrap = new ToggleButton(bottom, pos, 8, 24, 24,
                          "Wrap Edges", images_wrap_png,
                          (Fl_Callback *)checkWrap);
  pos += 24 + 6;
  new Separator(bottom, pos, 4, 2, 34, "");
  pos += 8;
  clone = new ToggleButton(bottom, pos, 8, 24, 24,
                           "Clone (Shift+Click to set target)",
                           images_clone_png,
                           (Fl_Callback *)checkClone);
  pos += 24 + 8;
  mirror = new Widget(bottom, pos, 8, 96, 24,
                      "Mirror", images_mirror_png, 24, 24,
                      (Fl_Callback *)checkMirror);
  pos += 96 + 6;
  new Separator(bottom, pos, 4, 2, 34, "");
  pos += 8;
  origin = new ToggleButton(bottom, pos, 8, 24, 24,
                            "Start From Center", images_origin_png,
                            (Fl_Callback *)checkOrigin);
  pos += 24 + 8;
  constrain = new ToggleButton(bottom, pos, 8, 24, 24,
                              "Lock Proportions",
                              images_constrain_png,
                              (Fl_Callback *)checkConstrain);
  bottom->resizable(0);
  bottom->end();

  // tools
  tools = new Group(0, top->h() + menubar->h(),
                    48, window->h() - (menubar->h() + top->h() + status->h()),
                    "Tools");
  pos = 28;
  tool = new Widget(tools, 8, pos, 32, 192,
                    "Tools", images_tools_png, 32, 32,
                    (Fl_Callback *)checkTool);
  pos += 96 + 8;
  tools->resizable(0);
  tools->end();

  // paint
  paint = new Group(48, top->h() + menubar->h(),
                    112, window->h() - top->h() - menubar->h() - status->h(),
                    "Paint");
  pos = 28;
  paint_brush_preview = new Widget(paint, 8, pos, 96, 96 + 20,
                           "Preview", 0, 0, 0);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  pos += 96;
  paint_size_value = new InputInt(paint, 8, pos, 96, 24,
                       "", (Fl_Callback *)checkPaintSizeValue, 1, 72);
//  paint_size_value->inc.labelsize(11);
//  paint_size_value->dec.labelsize(11);
//                       (Fl_Callback *));
//  paint_brush_label = new Fl_Box(FL_NO_BOX, paint_brush->x(), paint_brush->y(), paint_brush->w(), paint_brush->h(), "test");
//  paint_brush_label->align(FL_ALIGN_RIGHT | FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE);
//  paint_brush->bitmap->setpixelSolid(48, 48, makeRgb(192, 192, 192), 0);
  pos += 24 + 8;
  paint_size = new Widget(paint, 8, pos, 96, 24,
                          "Size", images_size_png, 6, 24,
                          (Fl_Callback *)checkPaintSize);
  pos += 24 + 8;
  paint_stroke = new Widget(paint, 8, pos, 96, 48,
                            "Stroke", images_stroke_png, 24, 24,
                            (Fl_Callback *)checkPaintStroke);
  pos += 48 + 8;
  paint_shape = new Widget(paint, 8, pos, 96, 24,
                           "Shape", images_shape_png, 6, 24,
                           (Fl_Callback *)checkPaintShape);
  pos += 24 + 8;
  new Separator(paint, 4, pos, 106, 2, "");
  pos += 8;
  paint_mode = new Fl_Choice(8, pos, 96, 24, "");
  paint_mode->tooltip("Mode");
  paint_mode->textsize(10);
  paint_mode->resize(paint->x() + 8, paint->y() + pos, 96, 24);
  paint_mode->add("Solid");
  paint_mode->add("Antialiased");
  paint_mode->add("Coarse");
  paint_mode->add("Fine");
  paint_mode->add("Blurred");
  paint_mode->add("Watercolor");
  paint_mode->add("Chalk");
  paint_mode->add("Texture");
  paint_mode->add("Average");
  paint_mode->value(0);
  paint_mode->callback((Fl_Callback *)checkPaintMode);
  pos += 24 + 8;
  paint_coarse_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintCoarseEdge);
  paint_fine_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintFineEdge);
  paint_blurry_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintBlurryEdge);
  paint_watercolor_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_watercolor_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintWatercolorEdge);
  paint_chalk_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_chalk_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintChalkEdge);
  paint_texture_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintTextureEdge);
  paint_texture_marb = new Widget(paint, 8, pos + 32, 96, 16,
                          "Marbleize", images_marbleize_png, 12, 16,
                          (Fl_Callback *)checkPaintTextureMarb);
  paint_texture_turb = new Widget(paint, 8, pos + 56, 96, 16,
                          "Turbulence", images_turbulence_png, 12, 16,
                          (Fl_Callback *)checkPaintTextureTurb);
  paint_average_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)checkPaintAverageEdge);
  pos += 24 + 8;
//  paint_dither_pattern = new Widget(paint, 8, pos, 96, 48,
//                                    "Dither Pattern", 24, 24,
//                                    (Fl_Callback *)0);
//  pos += 48 + 8;
//  paint_dither_relative = new CheckBox(paint, 12, pos, 16, 16, "Relative", 0);
//  paint_dither_relative->center();
  pos += 16 + 8;
  paint->resizable(0);
  paint->end();

  // selection
  selection = new Group(48, top->h() + menubar->h(),
                   112, window->h() - top->h() - menubar->h() - status->h(),
                   "Selection");
  pos = 28;
  new StaticText(selection, 8, pos, 32, 24, "x:");
  selection_x = new StaticText(selection, 24, pos, 72, 24, 0);
  pos += 24;
  new StaticText(selection, 8, pos, 32, 24, "y:");
  selection_y = new StaticText(selection, 24, pos, 72, 24, 0);
  pos += 24;
  new StaticText(selection, 8, pos, 32, 24, "w:");
  selection_w = new StaticText(selection, 24, pos, 72, 24, 0);
  pos += 24;
  new StaticText(selection, 8, pos, 32, 24, "h:");
  selection_h = new StaticText(selection, 24, pos, 72, 24, 0);
  pos += 24;
  new Separator(selection, 4, pos, 106, 2, "");
  pos += 8;
  selection_reset = new Fl_Button(selection->x() + 8, selection->y() + pos, 96, 32, "Reset");
  selection_reset->callback((Fl_Callback *)checkSelectionReset);
  pos += 32 + 8;
  new Separator(selection, 4, pos, 106, 2, "");
  pos += 8;
  selection_crop = new Fl_Button(selection->x() + 8, selection->y() + pos, 96, 32, "Crop");
  selection_crop->callback((Fl_Callback *)checkSelectionCrop);
  pos += 32 + 8;
  new Separator(selection, 4, pos, 106, 2, "");
  pos += 8;
  selection_select = new Fl_Button(selection->x() + 8, selection->y() + pos, 96, 32, "Copy");
  selection_select->callback((Fl_Callback *)checkSelectionSelect);
  pos += 32 + 6;
  selection_mirror = new Button(selection, 8, pos, 30, 30, "Mirror", images_select_mirror_png, (Fl_Callback *)checkSelectionFlipHorizontal);
  selection_flip = new Button(selection, 8 + 33, pos, 30, 30, "Flip", images_select_flip_png, (Fl_Callback *)checkSelectionFlipVertical);
  selection_rotate = new Button(selection, 8 + 66, pos, 30, 30, "Rotate", images_select_rotate_png, (Fl_Callback *)checkSelectionRotate90);
  pos += 30 + 8;
  selection->resizable(0);
  selection->end();

  // getcolor
  getcolor = new Group(48, top->h() + menubar->h(),
                       112, window->h() - top->h() - menubar->h() - status->h(),
                       "Get Color");
  pos = 28;
  getcolor_color = new Widget(getcolor, 8, pos, 96, 96, 0, 0, 0, 0);
  getcolor->resizable(0);
  getcolor->end();

  // offset
  offset = new Group(48, top->h() + menubar->h(),
                     112, window->h() - top->h() - menubar->h() - status->h(),
                     "Offset");
  pos = 28;
  new StaticText(offset, 8, pos, 32, 24, "x:");
  offset_x = new StaticText(offset, 24, pos, 72, 24, 0);
  pos += 24;
  new StaticText(offset, 8, pos, 32, 24, "y:");
  offset_y = new StaticText(offset, 24, pos, 72, 24, 0);
  pos += 24;

  new Separator(offset, 4, pos, 106, 2, "");
  pos += 8;
  offset_up = new RepeatButton(offset, 44, pos, 24, 24, "", images_up_png, (Fl_Callback *)checkOffsetUp);
  pos += 12;
  offset_left = new RepeatButton(offset, 16, pos, 24, 24, "", images_left_png, (Fl_Callback *)checkOffsetLeft);
  offset_right = new RepeatButton(offset, 72, pos, 24, 24, "", images_right_png, (Fl_Callback *)checkOffsetRight);
  pos += 16;
  offset_down = new RepeatButton(offset, 44, pos, 24, 24, "", images_down_png, (Fl_Callback *)checkOffsetDown);
  pos += 24;
  new StaticText(offset, 8, pos, 96, 24, "Nudge");

  offset->resizable(0);
  offset->end();

  // text
  text = new Group(48, top->h() + menubar->h(),
                   112, window->h() - top->h() - menubar->h() - status->h(),
                   "Text");
  pos = 28;
  // add font names
  font_browse = new Fl_Hold_Browser(8, pos, 96, 192);
  font_browse->textsize(9);
  font_browse->resize(text->x() + 8, text->y() + pos, 96, 192);

  for(int i = 0; i < Fl::set_fonts(0); i++)
  {
    int t = 0;
    const char *name = Fl::get_font_name((Fl_Font)i, &t);
    font_browse->add(name);
  }

  font_browse->value(0);
  font_browse->callback((Fl_Callback *)changedFontSize);
  pos += 192 + 8;

  // font size
  font_size = new InputInt(text, 40, pos, 64, 24, "Size:",
                           (Fl_Callback *)changedFontSize, 1, 256);
  font_size->value("24");
  pos += 24 + 8;

  text_input = new Fl_Input(8, pos, 96, 24, "");
  text_input->textsize(10);
  text_input->value("Text");
  text_input->resize(text->x() + 8, text->y() + pos, 96, 24);
  text_input->callback((Fl_Callback *)changedFontSize);
  pos += 24 + 8;

  text_smooth = new CheckBox(text, 8, pos, 16, 16, "Antialiased", 0);
  text_smooth->labelsize(13);
  text_smooth->center();
  text_smooth->value(1);

  text->resizable(0);
  text->end();

  // fill
  fill = new Group(48, top->h() + menubar->h(),
                   112, window->h() - top->h() - menubar->h() - status->h(),
                   "Fill");
  pos = 28 + 8;

  fill_feather = new InputInt(fill, 8, pos, 96, 24, "Feather (0-255)", 0, 0, 255);
  fill_feather->align(FL_ALIGN_BOTTOM);
  fill_feather->value("0");
  pos += 24 + 8;

  fill->resizable(0);
  fill->end();

  // palette
  palette = new Group(window->w() - 112 - 80, top->h() + menubar->h(),
                    80, window->h() - top->h() - menubar->h() - status->h(),
                    "Palette");
  pos = 28;
//  palette_swatches_border = new Fl_Box(FL_FLAT_BOX, palette->x() + 7, palette->y() + pos - 1, 66, 258, "");
//  palette_swatches_border->color(FL_BLACK);

  palette_swatches = new Widget(palette, 8, pos, 64, 256,
                       0, 8, 8,
                       (Fl_Callback *)checkPaletteSwatches);
/*
  palette = new Group(window->w() - 112 - 80, top->h() + menubar->h(),
                    80, window->h() - top->h() - menubar->h() - status->h(),
                    "Palette");
  pos = 20;
  palette_swatches = new Widget(palette, 8, pos, 64, 256,
                       0, 8, 8,
                       (Fl_Callback *)checkPaletteSwatches);
*/
  pos += 256 + 8;
  palette_editor = new Fl_Button(palette->x() + 8, palette->y() + pos, 64, 24, "Editor...");
  palette_editor->callback((Fl_Callback *)Dialog::editor);
  palette_editor->labelsize(12);

/*  palette_insert = new Fl_Button(palette->x() + 8, palette->y() + pos,
                                 28, 24, "+"); 
  palette_insert->tooltip("Insert");
  palette_insert->callback((Fl_Callback *)checkPaletteInsert);

  palette_delete = new Fl_Button(palette->x() + 44, palette->y() + pos,
                                 28, 24, "-"); 
  palette_delete->tooltip("Delete");
  palette_delete->callback((Fl_Callback *)checkPaletteDelete);

  pos += 24 + 8;

  palette_replace = new Fl_Button(palette->x() + 8, palette->y() + pos,
                                 64, 24, "Replace");
  palette_replace->callback((Fl_Callback *)checkPaletteReplace);
  palette_replace->labelsize(12);

  pos += 24 + 8;

  palette_undo = new Fl_Button(palette->x() + 8, palette->y() + pos,
                                 64, 24, "Undo");
  palette_undo->callback((Fl_Callback *)checkPaletteUndo);
  palette_undo->labelsize(12);
*/

  palette->resizable(0);
  palette->end();

  // colors
  colors = new Group(window->w() - 112, top->h() + menubar->h(),
                    112, window->h() - top->h() - menubar->h() - status->h(),
                    "Colors");
  pos = 28;
  swatch = new Widget(colors, 8, pos, 96, 48, "Paint Color", 0, 0, 0);
  pos += 48 + 8;
  hexcolor = new InputText(colors, 40, pos, 64, 24, "Hex:",
                           (Fl_Callback *)checkHexColor);
  hexcolor->maximum_size(6);
  hexcolor->textfont(FL_COURIER);
  hexcolor->textsize(14);
  pos += 24 + 8;
  // satval overlaps the hue color wheel
  hue = new Widget(colors, 8, pos, 96, 96, 0, 1, 1, (Fl_Callback *)checkColor);
  satval = new Widget(colors, 32, pos + 24, 48, 48, 0, 1, 1, (Fl_Callback *)checkColor);
  pos += 96 + 8;
  trans = new Widget(colors, 8, pos, 96, 24,
                     "Transparency", images_transparency_png, 6, 24,
                     (Fl_Callback *)checkColor);
  pos += 24 + 8;
  blend = new Fl_Choice(8, pos, 96, 24, "");
  blend->tooltip("Blending Mode");
  blend->textsize(10);
  blend->resize(colors->x() + 8, colors->y() + pos, 96, 24);
  blend->add("Normal");
  blend->add("Lighten");
  blend->add("Darken");
  blend->add("Colorize");
  blend->add("Luminosity");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->add("Sharpen");
  blend->value(0);
  blend->callback((Fl_Callback *)checkColor);
  pos += 24 + 8;
  new Separator(colors, 4, pos, 106, 2, "");
  pos += 8;
  range = new Widget(colors, 8, pos, 96, 192, 0, 1, 1, (Fl_Callback *)checkRange);
  range_buf = new Bitmap(96, 192);
//  alpha_mask = new CheckBox(colors, 8, pos, 96, 24, "Alpha Mask", (Fl_Callback *)checkAlphaMask);
//  alpha_mask->labelsize(13);
//  pos += 24 + 8;
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
  left = new Fl_Group(0, top->h() + menubar->h(),
                            64 + 96, window->h() - (menubar->h() + top->h() + bottom->h()));
  left->add(tools);
  left->add(paint);
  left->add(getcolor);
  left->add(selection);
  left->add(offset);
  left->add(text);
  left->end();

  // container for right panels
  right = new Fl_Group(window->w() - 112 - 80, top->h() + menubar->h(),
                            112 + 80, window->h() - (menubar->h() + top->h() + bottom->h()));
  right->add(palette);
  right->add(colors);

  window->size_range(640, 480, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();

  // misc init
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

//  Project::brush->edge = 3;
//  paint_edge->var = 3;
//  Project::brush->size = 4;

  updateColor(Project::palette->data[palette_swatches->var]);
  //updateRange();
  //range->do_callback();
  drawPalette();
  tool->do_callback();
  checkZoom();
  checkSelectionValues(0, 0, 0, 0);
  checkOffsetValues(0, 0);
  checkPaintMode();
  paint_size->var = 0;
  paint_size->do_callback();
  paint_coarse_edge->var = 3;
  paint_coarse_edge->do_callback();
  paint_fine_edge->var = 3;
  paint_fine_edge->do_callback();
  paint_watercolor_edge->var = 3;
  paint_watercolor_edge->do_callback();
  paint_chalk_edge->var = 2;
  paint_chalk_edge->do_callback();
  paint_average_edge->var = 2;
  paint_average_edge->do_callback();

  // create dither pattern image from data in DitherMatrix.H
/*
  paint_dither_pattern->bitmap->clear(makeRgb(0, 0, 0));

  for(int z = 0; z < 8; z++)
  {
    for(int y = 0; y < 8; y++)
    {
      const int yy = (z / 4) * 24 + y * 3;
      for(int x = 0; x < 8; x++)
      {
        const int xx = (z % 4) * 24 + x * 3;
        if(DitherMatrix::pattern[z][(y & 3)][(x & 3)] == 1)
          paint_dither_pattern->bitmap->rectfill(xx, yy, xx + 2, yy + 2,
                                         makeRgb(208, 208, 208), 0);
      }
    }
  }
*/
/*
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
*/
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
  swatch->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR));

  for(int y = 1; y < swatch->bitmap->h - 1; y++)
  {
    int *p = swatch->bitmap->row[y] - 1;
    for(int x = 0; x < swatch->bitmap->w; x++)
    {
      const int checker = ((x >> 4) & 1) ^ ((y >> 4) & 1)
                            ? 0xA0A0A0 : 0x606060;

      *p++ = blendFast(checker, Project::brush->color, Project::brush->trans);
    }
  }

//  swatch->bitmap->rectfill(0, 0, swatch->bitmap->w / 2 - 1, swatch->bitmap->h - 1, Project::brush->color, 0);
  swatch->bitmap->rect(0, 0, swatch->bitmap->w - 1, swatch->bitmap->h - 1,
                       makeRgb(0, 0, 0), 0);
  swatch->redraw();
}

void Gui::updateRange()
{
  int c = Project::brush->color;
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);
  int h, s, v;

  Blend::rgbToHsv(r, g, b, &h, &s, &v);

  for(int y = 0; y < 192; y++)
  {
    const int hh = (h + y) % 1536;

    for(int x = 0; x < 96; x++)
    {
      int vv = v + x;

      vv = clamp(vv, 255);

      Blend::hsvToRgb(hh, s, vv, &r, &g, &b);
      //range_buf->setpixel(x, y, makeRgb(r, g, b));
      range_buf->setpixel(x, y, Blend::keepLum(makeRgb(r, g, b), x * 2.69));
    }
  }

  range_buf->rect(0, 0, range->bitmap->w - 1, range->bitmap->h - 1,
                       makeRgb(0, 0, 0), 0);
  range_buf->blit(range->bitmap, 0, 0, 0, 0, range_buf->w, range_buf->h);

  int pos = range->var;
  int mx = pos % 96;
  int my = pos / 96;

  range->bitmap->rect(mx - 6, my - 6, mx + 6, my + 6, makeRgb(0, 0, 0), 192);
  range->bitmap->rect(mx - 5, my - 5, mx + 5, my + 5, makeRgb(0, 0, 0), 96);
  range->bitmap->xorRect(mx - 4, my - 4, mx + 4, my + 4);

  range->redraw();
}

void Gui::updateGetColor(int c)
{
  getcolor_color->bitmap->clear(c);
  getcolor_color->bitmap->rect(0, 0, getcolor_color->bitmap->w - 1, getcolor_color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
  getcolor_color->redraw();
}

void Gui::checkPaletteSwatches(Widget *widget, void *var)
{
  Palette *pal = Project::palette.get();
  int pos = *(int *)var;

  if(pos > pal->max - 1)
  {
    pos = pal->max - 1;
    widget->var = pos;
  }

  int step = widget->stepx;
  int div = widget->w() / step;

  int x = pos % div;
  int y = pos / div;

  if(y > (pal->max - 1) / div)
  {
    y = (pal->max - 1) / div;
    pos = x + div * y;
    x = pos % div;
    y = pos / div;
    widget->var = pos;
  }

  if(pos > pal->max - 1)
  {
    pos = pal->max - 1;
    x = pos % div;
    y = pos / div;
    widget->var = pos;
  }

  int c = widget->bitmap->getpixel(x * step + 2, y * step + 2);
  pal->draw(widget);
  updateColor(c);
}

void Gui::setPaletteIndex(int var)
{
  palette_swatches->var = var;
  Project::palette->draw(palette_swatches);
}

int Gui::getPaletteIndex()
{
  return palette_swatches->var;
}

/*
void Gui::checkPaletteInsert(Widget *widget, void *var)
{
  Project::palette->copy(undo_palette);
  begin_palette_undo = true;
  Project::palette->insertColor(Project::brush->color, palette_swatches->var);
  Project::palette->fillTable();
  Project::palette->draw(palette_swatches);
  palette_swatches->do_callback();
}

void Gui::checkPaletteDelete(Widget *widget, void *var)
{
  Project::palette->copy(undo_palette);
  begin_palette_undo = true;
  Project::palette->deleteColor(palette_swatches->var);
  Project::palette->fillTable();
  Project::palette->draw(palette_swatches);

  if(palette_swatches->var > Project::palette->max - 1)
    palette_swatches->var = Project::palette->max - 1;

  palette_swatches->do_callback();
}

void Gui::checkPaletteReplace(Widget *widget, void *var)
{
  Project::palette->copy(undo_palette);
  begin_palette_undo = true;
  Project::palette->replaceColor(Project::brush->color, palette_swatches->var);
  Project::palette->fillTable();
  Project::palette->draw(palette_swatches);
  palette_swatches->do_callback();
}

void Gui::checkPaletteUndo(Widget *widget, void *var)
{
  if(begin_palette_undo)
  {
    begin_palette_undo = false;
    undo_palette->copy(Project::palette.get());
    Project::palette->fillTable();
    Project::palette->draw(palette_swatches);
    Gui::drawPalette();
    palette_swatches->do_callback();
  }
}
*/

void Gui::drawPalette()
{
  Project::palette->draw(palette_swatches);
  palette_swatches->var = 0;
  palette_swatches->redraw();
//  palette_swatches->do_callback();
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

  if(view->zoom < 1)
    snprintf(s, sizeof(s), "%0.3fx", view->zoom);
  else
    snprintf(s, sizeof(s), "%2.1fx", view->zoom);
    
  zoom->copy_label(s);
  zoom->redraw();
}

void Gui::checkGrid(ToggleButton *, void *var)
{
  view->grid = *(int *)var;
//  Project::tool->reset();
  view->drawMain(true);
  view->redraw();

  if(Project::tool->isActive())
  {
    Project::tool->redraw(view);
  }
}

void Gui::checkGridX()
{
  view->gridx = atoi(gridx->value());
  view->drawMain(true);
  view->redraw();
}

void Gui::checkGridY()
{
  view->gridy = atoi(gridy->value());
  view->drawMain(true);
  view->redraw();
}

void Gui::changePaintSize(int size)
{
  Brush *brush = Project::brush.get();
  float round = (float)(15 - paint_shape->var) / 15;

  brush->make(size, round);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 21,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);

  for(int i = 0; i < brush->solid_count; i++)
  {
    paint_brush_preview->bitmap->setpixelSolid(48 + brush->solidx[i],
                                       48 + brush->solidy[i],
                                       convertFormat(getFltkColor(FL_FOREGROUND_COLOR), true),
                                       0);
  }

  paint_brush_preview->redraw();
  paint_size_value->redraw();
}

void Gui::checkPaintSize(Widget *, void *var)
{
  int size = brush_sizes[*(int *)var];
  changePaintSize(size);
  char s[16];
  sprintf(s, "%d", (int)size);
  paint_size_value->value(s);
  paint_size_value->redraw();
}

void Gui::checkPaintSizeValue(Widget *, void *var)
{
  int size;
  sscanf(paint_size_value->value(), "%d", &size);
  changePaintSize(size);
}

void Gui::checkPaintShape(Widget *, void *)
{
//  paint_size->do_callback();
  changePaintSize(Project::brush->size);
}

void Gui::checkPaintStroke(Widget *, void *var)
{
  Project::stroke->type = *(int *)var;
}

void Gui::checkPaintCoarseEdge(Widget *, void *var)
{
  Project::brush->coarse_edge = *(int *)var;
}

void Gui::checkPaintFineEdge(Widget *, void *var)
{
  Project::brush->fine_edge = *(int *)var;
}

void Gui::checkPaintBlurryEdge(Widget *, void *var)
{
  Project::brush->blurry_edge = *(int *)var;
}

void Gui::checkPaintWatercolorEdge(Widget *, void *var)
{
  Project::brush->watercolor_edge = *(int *)var;
}

void Gui::checkPaintChalkEdge(Widget *, void *var)
{
  Project::brush->chalk_edge = *(int *)var;
}

void Gui::checkPaintTextureEdge(Widget *, void *var)
{
  Project::brush->texture_edge = *(int *)var;
}

void Gui::checkPaintTextureMarb(Widget *, void *var)
{
  Project::brush->texture_marb = *(int *)var;
}

void Gui::checkPaintTextureTurb(Widget *, void *var)
{
  Project::brush->texture_turb = *(int *)var;
}

void Gui::checkPaintAverageEdge(Widget *, void *var)
{
  Project::brush->average_edge = *(int *)var;
}

void Gui::checkTool(Widget *, void *var)
{
  int tool = *(int *)var;

  if(tool != Tool::PAINT)
    paint->hide();
  if(tool != Tool::GETCOLOR)
    getcolor->hide();
  if(tool != Tool::KNIFE)
    selection->hide();
  if(tool != Tool::OFFSET)
    offset->hide();
  if(tool != Tool::TEXT)
    text->hide();
  if(tool != Tool::FILL)
    fill->hide();

  Project::tool->reset();

  switch(tool)
  {
    case Tool::PAINT:
      Project::setTool(Tool::PAINT);
      paint_brush_preview->do_callback();
      paint_shape->do_callback();
      paint->show();
      updateInfo((char *)"Middle-click to navigate. Mouse wheel zooms. Esc cancels rendering.");
      
      break;
    case Tool::GETCOLOR:
      Project::setTool(Tool::GETCOLOR);
      getcolor->show();
      updateInfo((char *)"Click to select a color from the image.");
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

/*
  if(Project::tool->isActive())
    Project::tool->redraw(view);
  else
    view->drawMain(true);
*/
  view->ignore_tool = true;
  view->drawMain(true);
}

void Gui::checkColor(Widget *widget, void *)
{
  int pos = hue->var;
  int mx = pos % 96;
  int my = pos / 96;

  if(widget == hue)
  {
    if(((mx - 48) * (mx - 48) + (my - 48) * (my - 48)) < (20 * 20))
    {
      satval->redraw();
      return;
    }
  }

  float mouse_angle = atan2f(my - 48, mx - 48);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 48) * 5.43;
  int v = (satval->var / 48) * 5.43;

  int r, g, b;

  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);
//  Project::brush->trans = std::pow((double)trans->var, 2.02);
//  Project::brush->trans = std::pow((double)trans->var, 2.04);
  Project::brush->trans = trans->var * 16.8;
//  Project::brush->trans = Gamma::unfix(trans->var * 4096);
  Project::brush->blend = blend->value();

  // hue circle
  hue->bitmap->clear(blendFast(convertFormat(getFltkColor(FL_BACKGROUND_COLOR),true), makeRgb(0, 0, 0), 192));

  for(int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((3.14159 * 2) / 1536) * i;
    int x1 = 48 + 46 * std::cos(angle);
    int y1 = 48 + 46 * std::sin(angle);
    int x2 = 48 + 36 * std::cos(angle);
    int y2 = 48 + 36 * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 252);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 252);
  }

  for(int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((3.14159 * 2) / 1536) * i;
    int x1 = 48 + 45 * std::cos(angle);
    int y1 = 48 + 45 * std::sin(angle);
    int x2 = 48 + 37 * std::cos(angle);
    int y2 = 48 + 37 * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 252);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 252);
  }

  for(int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((3.14159 * 2) / 1536) * i;
    int x1 = 48 + 44 * std::cos(angle);
    int y1 = 48 + 44 * std::sin(angle);
    int x2 = 48 + 38 * std::cos(angle);
    int y2 = 48 + 38 * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(r, g, b), 0);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(r, g, b), 0);
  }

  const int x1 = 48 + 41 * std::cos(mouse_angle);
  const int y1 = 48 + 41 * std::sin(mouse_angle);

  Blend::hsvToRgb(h, 255, 255, &r, &g, &b);
  hue->bitmap->rect(x1 - 6, y1 - 6, x1 + 6, y1 + 6, makeRgb(0, 0, 0), 192);
  hue->bitmap->rect(x1 - 5, y1 - 5, x1 + 5, y1 + 5, makeRgb(0, 0, 0), 96);
  hue->bitmap->xorRect(x1 - 4, y1 - 4, x1 + 4, y1 + 4);
  hue->bitmap->rectfill(x1 - 3, y1 - 3, x1 + 3, y1 + 3, makeRgb(r, g, b), 0);

  // saturation/value
  hue->bitmap->rect(24 - 2, 24 - 2, 71 + 2, 71 + 2, makeRgb(0, 0, 0), 192);
  hue->bitmap->rect(24 - 1, 24 - 1, 71 + 1, 71 + 1, makeRgb(0, 0, 0), 96);
  hue->bitmap->rect(0, 0, 95, 95, makeRgb(0, 0, 0), 0);

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

  satval->bitmap->rect(x - 6, y - 6, x + 6, y + 6, makeRgb(0, 0, 0), 192);
  satval->bitmap->rect(x - 5, y - 5, x + 5, y + 5, makeRgb(0, 0, 0), 96);
  satval->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);

  hue->redraw();
  satval->redraw();

  updateSwatch();
  updateRange();
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

void Gui::checkRange(Widget *widget, void *)
{
  int pos = range->var;
  int mx = pos % 96;
  int my = pos / 96;

  if(mx < 1)
    mx = 1;

  if(mx > 94)
     mx = 94;

  if(my < 1)
    my = 1;

  if(my > 190)
     my = 190;

  Project::brush->color = range_buf->getpixel(mx, my);
  //updateRange();

//  int pos = range->var;
//  int mx = pos % 96;
//  int my = pos / 96;

  range_buf->blit(range->bitmap, 0, 0, 0, 0, range_buf->w, range_buf->h);

  range->bitmap->rect(mx - 6, my - 6, mx + 6, my + 6, makeRgb(0, 0, 0), 192);
  range->bitmap->rect(mx - 5, my - 5, mx + 5, my + 5, makeRgb(0, 0, 0), 96);
  range->bitmap->xorRect(mx - 4, my - 4, mx + 4, my + 4);

  updateSwatch();
}

/*
void Gui::checkAlphaMask()
{
  Project::brush->alpha_mask = alpha_mask->value();
  Gui::getView()->drawMain(true);
}
*/

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

void Gui::checkSelectionRotate90()
{
  int w = Project::select_bmp->w;
  int h = Project::select_bmp->h;

  // make copy
  Bitmap temp(w, h);
  Project::select_bmp->blit(&temp, 0, 0, 0, 0, w, h);

  // create rotated image
  delete Project::select_bmp;
  Project::select_bmp = new Bitmap(h, w);

  int *p = &temp.data[0];

  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
//       *(Project::bmp->row[w - 1 - x] + y) = *p++;   
       *(Project::select_bmp->row[x] + h - 1 - y) = *p++;
    }
  }
}

void Gui::checkSelectionRotate180()
{
  Project::select_bmp->rotate180();
}

void Gui::checkSelectionReset()
{
  Project::tool->done(view, 2);
}

void Gui::checkSelectionValues(int x, int y, int w, int h)
{
  char s[256];

  snprintf(s, sizeof(s), "%d", x);
  selection_x->copy_label(s);
  selection_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  selection_y->copy_label(s);
  selection_y->redraw();

  snprintf(s, sizeof(s), "%d", w);
  selection_w->copy_label(s);
  selection_w->redraw();

  snprintf(s, sizeof(s), "%d", h);
  selection_h->copy_label(s);
  selection_h->redraw();
}

void Gui::checkOffsetValues(int x, int y)
{
  char s[256];

  snprintf(s, sizeof(s), "%d", x);
  offset_x->copy_label(s);
  offset_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  offset_y->copy_label(s);
  offset_y->redraw();
}

void Gui::checkOffsetLeft(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgx = -1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::checkOffsetRight(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgx = 1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::checkOffsetUp(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgy = -1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::checkOffsetDown(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgy = 1;
  Project::tool->drag(view);
  Project::tool->release(view);
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

void Gui::changedFontSize(InputInt *input, void *var)
{
  input->redraw();
}

const char *Gui::getTextInput()
{
  return text_input->value();
}

void Gui::paletteDefault()
{
  Project::palette->setDefault();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
}

void Gui::paletteBlackAndWhite()
{
  Project::palette->setBlackAndWhite();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
}

void Gui::paletteWebSafe()
{
  Project::palette->setWebSafe();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
}

void Gui::palette3LevelRGB()
{
  Project::palette->set3LevelRGB();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
}

void Gui::palette4LevelRGB()
{
  Project::palette->set4LevelRGB();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
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

void Gui::checkClearToGray()
{
  Undo::push();

  Bitmap *bmp = Project::bmp;

  for(int y = bmp->ct; y <= bmp->cb; y++)
    for(int x = bmp->cl; x <= bmp->cr; x++)
      *(bmp->row[y] + x) = 0xFF808080;

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
  paint_coarse_edge->hide();
  paint_fine_edge->hide();
  paint_blurry_edge->hide();
  paint_watercolor_edge->hide();
  paint_chalk_edge->hide();
  paint_texture_edge->hide();
  paint_texture_marb->hide();
  paint_texture_turb->hide();
  paint_average_edge->hide();
//  paint_dither_pattern->hide();
//  paint_dither_relative->hide();

  switch(paint_mode->value())
  {
    case Render::SOLID:
//      paint_dither_pattern->show();
//      paint_dither_relative->show();
      break;
    case Render::ANTIALIASED:
      Project::brush->aa = 1;
      break;
    case Render::COARSE:
      paint_coarse_edge->show();
      break;
    case Render::FINE:
      paint_fine_edge->show();
      break;
    case Render::BLURRY:
      paint_blurry_edge->show();
      break;
    case Render::WATERCOLOR:
      paint_watercolor_edge->show();
      break;
    case Render::CHALK:
      paint_chalk_edge->show();
      break;
    case Render::TEXTURE:
      paint_texture_edge->show();
      paint_texture_marb->show();
      paint_texture_turb->show();
      break;
    case Render::AVERAGE:
      paint_average_edge->show();
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

int Gui::getFillFeather()
{
  return atoi(fill_feather->value());
}

/*
int Gui::getDitherPattern()
{
  return paint_dither_pattern->var;
}

int Gui::getDitherRelative()
{
  return paint_dither_relative->value();
}
*/

void Gui::resetTool()
{
  Project::tool->reset();
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
    progress->value(progress_value);
    char percent[16];
    sprintf(percent, "%d%%", (int)progress_value);
    progress->copy_label(percent);
    Fl::check();
    progress_value += progress_step;
    view->drawMain(true);
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

