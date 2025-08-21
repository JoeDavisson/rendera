/*
Copyright (c) 2025 Joe Davisson.

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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <typeinfo>
#include <vector>

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
#include "Editor.H"
#include "ExportData.H"
#include "FX/FX.H"
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
#include "Selection.H"
#include "StaticText.H"
#include "Stroke.H"
#include "ToggleButton.H"
#include "Text.H"
#include "Tool.H"
#include "Transform.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

#define TOP_HEIGHT 56
#define TOOLS_WIDTH 64
#define OPTIONS_WIDTH 176
#define COLORS_WIDTH 208
#define FILES_WIDTH 176
#define IMAGES_WIDTH 256
#define IMAGES_HEIGHT 384
#define STATUS_HEIGHT 32
#define TOTAL_WIDTH (TOOLS_WIDTH + OPTIONS_WIDTH + COLORS_WIDTH + FILES_WIDTH)

class MainWin;

// top
Button *Gui::zoom_one;
Button *Gui::zoom_in;
Button *Gui::zoom_out;
StaticText *Gui::zoom;
ToggleButton *Gui::grid;
ToggleButton *Gui::gridsnap;
InputInt *Gui::gridx;
InputInt *Gui::gridy;
Fl_Choice *Gui::aspect;
CheckBox *Gui::filter;

// tools
Widget *Gui::tool;
ToggleButton *Gui::clone;
ToggleButton *Gui::origin;
ToggleButton *Gui::constrain;

// options
Widget *Gui::paint_brush_preview;
Widget *Gui::paint_size;
InputInt *Gui::paint_size_value;
Widget *Gui::paint_stroke;
Widget *Gui::paint_shape;
Widget *Gui::paint_coarse_edge;
Widget *Gui::paint_fine_edge;
Widget *Gui::paint_blurry_edge;
Widget *Gui::paint_watercolor_edge;
Widget *Gui::paint_chalk_edge;
Widget *Gui::paint_texture_edge;
Widget *Gui::paint_texture_marb;
Widget *Gui::paint_texture_turb;
Widget *Gui::paint_average_edge;
Fl_Choice *Gui::paint_mode;

Widget *Gui::getcolor_color;
CheckBox *Gui::getcolor_best;

InputInt *Gui::fill_range;
InputInt *Gui::fill_feather;
CheckBox *Gui::fill_color_only;
Fl_Button *Gui::fill_reset;

StaticText *Gui::selection_x;
StaticText *Gui::selection_y;
StaticText *Gui::selection_w;
StaticText *Gui::selection_h;
Fl_Button *Gui::selection_reset;
Fl_Button *Gui::selection_copy;
CheckBox *Gui::selection_alpha;
Button *Gui::selection_flip;
Button *Gui::selection_mirror;
Button *Gui::selection_rotate;
Fl_Button *Gui::selection_paste;
Fl_Button *Gui::selection_crop;

StaticText *Gui::offset_x;
StaticText *Gui::offset_y;
RepeatButton *Gui::offset_up;
RepeatButton *Gui::offset_left;
RepeatButton *Gui::offset_right;
RepeatButton *Gui::offset_down;

Fl_Hold_Browser *Gui::font_browse;
InputInt *Gui::font_size;
InputInt *Gui::font_angle;
Fl_Input *Gui::text_input;
CheckBox *Gui::text_smooth;

// colors
Widget *Gui::hue;
Widget *Gui::satval;
InputText *Gui::hexcolor;
InputInt *Gui::trans_input;
Widget *Gui::trans;
Fl_Choice *Gui::blend;
Widget *Gui::palette_swatches;

// files
Fl_Hold_Browser *Gui::file_browse;
Fl_Input *Gui::file_rename;
Button *Gui::file_close;
Button *Gui::file_move_up;
Button *Gui::file_move_down;
Fl_Box *Gui::file_mem;

// view
View *Gui::view;
Fl_Box *Gui::coords;
Fl_Box *Gui::info;

// progress
Fl_Progress *Gui::progress;

static bool sort_value_cb(const int c1, const int c2)
{
  return getl(c1) < getl(c2);
}

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
  Group *colors;
  Group *files;
  Group *status;
  Fl_Group *middle;

  // status
//  Fl_Progress *progress;
//  Fl_Box *coords;
//  Fl_Box *info;

  // height of rightmost panels
  int left_height = 0;
  int right_height = 0;

  // progress indicator related
/*
  float progress_value = 0;
  float progress_step = 0;
  int progress_interval = 50;
  bool progress_enable = true;
*/

  // tables
  const int brush_sizes[16] =
  {
    1, 2, 3, 4, 6, 8, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100
  };

  // space between widgets
  const int gap = 15;

  // quit program
  void quit()
  {
    if (Dialog::choice("Exit", "Exit Program?"))
      exit(0);
  }

  // prevent escape from closing main window
  void closeCallback(Fl_Widget *widget, void *)
  {
    if ((Fl::event() == FL_KEYDOWN || Fl::event() == FL_SHORTCUT)
       && Fl::event_key() == FL_Escape)
    {
      return;
    }
      else
    {
      // hide any open windows so FLTK ends the program
      if (Dialog::choice("Exit", "Are You Sure?"))
      {
        widget->hide();
      }
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

    switch (event)
    {
      case FL_FOCUS:
        return 1;
      case FL_UNFOCUS:
        return 1;
      case FL_KEYBOARD:
        // give focus to the main menu
        if (Fl::event_alt() > 0)
        {
          Gui::getMenuBar()->take_focus();
          return 0;
        }

        shift = Fl::event_shift() ? true : false;
        ctrl = Fl::event_ctrl() ? true : false;

        // cancel current rendering operation
        if (Fl::event_key() == FL_Escape)
        {
          Project::tool->reset();
          view->drawMain(true);
          return 1;
        }

        // inhibit use of keys while rendering
        if (view->rendering)
          return 0;

        // misc keys
        switch (Fl::event_key())
        {
          case FL_Right:
            view->scroll(0, 64);
            return 1;
          case FL_Left:
            view->scroll(1, 64);
            return 1;
          case FL_Down:
            view->scroll(2, 64);
            return 1;
          case FL_Up:
            view->scroll(3, 64);
            return 1;
          case FL_Delete:
            Gui::imagesCloseFile();
            return 1;
          case '1':
            view->zoomOne();
            return 1;
          case '+':
          case '=':
            if (ctrl == false)
            {
              view->zoomIn(view->w() / 2, view->h() / 2);
              return 1;
            }
              else
            {
              return Fl_Double_Window::handle(event);
            }
          case '-':
            if (ctrl == false)
            {
              view->zoomOut(view->w() / 2, view->h() / 2);
              return 1;
            }
              else
            {
              return Fl_Double_Window::handle(event);
            }
          case 'z':
            if (ctrl && shift)
              Project::undo->popRedo();
            else if (ctrl)
              Project::undo->pop();
            return 1;
          case 'c':
            if (ctrl)
              Gui::selectCopy();
            return 1;
          case 'v':
            if (ctrl)
              Gui::selectPaste();
            return 1;
          case 'e':
            Editor::begin();
            return 1;
        }
    }

    return Fl_Double_Window::handle(event);
  }
};

// initialize main gui
void Gui::init()
{
  int pos;

  // main window
  FL_NORMAL_SIZE = 18;
  window = new MainWin(1280, 828, "Rendera");
  window->callback(closeCallback);
  window->xclass("Rendera");

  // generate menu
  menubar = new Fl_Menu_Bar(0, 0, window->w(), 36);
  menubar->box(FL_THIN_UP_BOX);
  menubar->color(FL_INACTIVE_COLOR);

  menubar->add("&File/&New...", 0,
    (Fl_Callback *)Dialog::newImage, 0, 0);
  menubar->add("&File/&Open...", 0,
    (Fl_Callback *)File::load, 0, 0);
  menubar->add("&File/&Save...", 0,
    (Fl_Callback *)File::save, 0, FL_MENU_DIVIDER);
  menubar->add("&File/&Close...", 0,
    (Fl_Callback *)imagesCloseFile, 0, FL_MENU_DIVIDER);
  menubar->add("&File/Export &Data...", 0,
    (Fl_Callback *)ExportData::save, 0, FL_MENU_DIVIDER);
  menubar->add("&File/E&xit...", 0,
    (Fl_Callback *)quit, 0, 0);

  menubar->add("&Edit/Undo (Ctrl+Z)", 0,
    (Fl_Callback *)Project::pop, 0, 0);
  menubar->add("&Edit/Redo (Shift+Ctrl+Z)", 0,
    (Fl_Callback *)Project::popRedo, 0);
  menubar->add("&Clear/&Black", 0,
    (Fl_Callback *)clearToBlack, 0, 0);
  menubar->add("&Clear/&White", 0,
    (Fl_Callback *)clearToWhite, 0, 0);
  menubar->add("&Clear/&Gray", 0,
    (Fl_Callback *)clearToGray, 0, 0);
  menubar->add("&Clear/&Paint Color", 0,
    (Fl_Callback *)clearToColor, 0, 0);
  menubar->add("&Clear/&Transparent", 0,
    (Fl_Callback *)clearToTrans, 0, 0);

  menubar->add("&Image/&Resize...", 0,
    (Fl_Callback *)Transform::resize, 0, 0);
  menubar->add("&Image/&Scale...", 0,
    (Fl_Callback *)Transform::scale, 0, FL_MENU_DIVIDER);
  menubar->add("&Image/&Duplicate", 0,
    (Fl_Callback *)imagesDuplicate, 0, FL_MENU_DIVIDER);
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

  menubar->add("&Selection/&Open...", 0,
    (Fl_Callback *)File::loadSelection, 0, 0);
  menubar->add("&Selection/&Save...", 0,
    (Fl_Callback *)File::saveSelection, 0, FL_MENU_DIVIDER);
  menubar->add("&Selection/&Image to Selection", 0,
    (Fl_Callback *)selectFromImage, 0, 0);
  menubar->add("&Selection/Selection to &New Image", 0,
    (Fl_Callback *)selectToImage, 0, 0);

  menubar->add("&Palette/&Open...", 0,
    (Fl_Callback *)File::loadPalette, 0, 0);
  menubar->add("&Palette/&Save...", 0,
    (Fl_Callback *)File::savePalette, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Create...", 0,
    (Fl_Callback *)Dialog::makePalette, 0, 0);
  menubar->add("&Palette/&Apply...", 0,
    (Fl_Callback *)Dither::begin, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/Presets/Default", 0,
    (Fl_Callback *)paletteSetDefault, 0, 0);
  menubar->add("Palette/Presets/Black and White", 0,
    (Fl_Callback *)paletteSetBlackAndWhite, 0, 0);
  menubar->add("Palette/Presets/Grays", 0,
    (Fl_Callback *)paletteSetGrays, 0, 0);
  menubar->add("Palette/Presets/Two Bits", 0,
    (Fl_Callback *)paletteSetTwoBits, 0, 0);
  menubar->add("Palette/Presets/C64", 0,
    (Fl_Callback *)paletteSetC64, 0, 0);
  menubar->add("Palette/Presets/VCS", 0,
    (Fl_Callback *)paletteSetVCS, 0, 0);
  menubar->add("&Palette/Presets/Web Safe", 0,
    (Fl_Callback *)paletteSetWebSafe, 0, 0);
  menubar->add("&Palette/Presets/3-level RGB", 0,
    (Fl_Callback *)paletteSet3LevelRGB, 0, 0);
  menubar->add("&Palette/Presets/4-level RGB", 0,
    (Fl_Callback *)paletteSet4LevelRGB, 0, 0);
  menubar->add("&Palette/Presets/332", 0,
    (Fl_Callback *)paletteSet332, 0, 0);
  menubar->add("&Palette/&Editor... (E)", 0,
    (Fl_Callback *)Editor::begin, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Sort", 0,
    (Fl_Callback *)paletteSortHue, 0, 0);

//  menubar->add("F&X/Color/Test", 0,
//    (Fl_Callback *)Test::begin, 0, 0);
  menubar->add("F&X/Color/Normalize", 0,
    (Fl_Callback *)Normalize::begin, 0, 0);
  menubar->add("F&X/Color/Equalize", 0,
    (Fl_Callback *)Equalize::begin, 0, 0);
  menubar->add("F&X/Color/Value Stretch", 0,
    (Fl_Callback *)ValueStretch::begin, 0, 0);
  menubar->add("F&X/Color/Saturate", 0,
    (Fl_Callback *)Saturate::begin, 0, 0);
  menubar->add("F&X/Color/Rotate Hue...", 0,
    (Fl_Callback *)RotateHue::begin, 0, 0);
  menubar->add("F&X/Color/Desaturate", 0,
    (Fl_Callback *)Desaturate::begin, 0, 0);
  menubar->add("F&X/Color/Colorize", 0,
    (Fl_Callback *)Colorize::begin, 0);
  menubar->add("F&X/Color/Palette Colors", 0,
   (Fl_Callback *)PaletteColors::begin, 0, 0);
  menubar->add("F&X/Color/Invert", 0,
    (Fl_Callback *)Invert::begin, 0, 0);

  menubar->add("F&X/Alpha/Invert", 0,
    (Fl_Callback *)AlphaInvert::begin, 0, 0);
  menubar->add("F&X/Alpha/Clear", 0,
    (Fl_Callback *)AlphaClear::begin, 0, 0);
  menubar->add("F&X/Alpha/Blend to Paint Color", 0,
    (Fl_Callback *)AlphaColor::begin, 0, 0);

  menubar->add("F&X/Filters/Gaussian Blur...", 0,
    (Fl_Callback *)GaussianBlur::begin, 0, 0);
  menubar->add("F&X/Filters/Sharpen...", 0,
    (Fl_Callback *)Sharpen::begin, 0, 0);
  menubar->add("F&X/Filters/Unsharp Mask...", 0,
    (Fl_Callback *)UnsharpMask::begin, 0, 0);
  menubar->add("F&X/Filters/Box Filters...", 0,
    (Fl_Callback *)BoxFilters::begin, 0, 0);
  menubar->add("F&X/Filters/Sobel...", 0,
    (Fl_Callback *)Sobel::begin, 0, 0);
  menubar->add("F&X/Filters/Bloom...", 0,
    (Fl_Callback *)Bloom::begin, 0, 0);
  menubar->add("F&X/Filters/Randomize", 0,
    (Fl_Callback *)Randomize::begin, 0, 0);

  menubar->add("F&X/Photo/Restore...", 0,
    (Fl_Callback *)Restore::begin, 0, 0);
  menubar->add("F&X/Photo/Side Absorptions", 0,
    (Fl_Callback *)SideAbsorptions::begin, 0, 0);
  menubar->add("F&X/Photo/Remove Dust...", 0,
    (Fl_Callback *)RemoveDust::begin, 0, 0);

  menubar->add("F&X/Artistic/Stained Glass...", 0,
    (Fl_Callback *)StainedGlass::begin, 0, 0);
  menubar->add("F&X/Artistic/Painting...", 0,
    (Fl_Callback *)Painting::begin, 0, 0);
  menubar->add("F&X/Artistic/Marble...", 0,
    (Fl_Callback *)Marble::begin, 0, 0);

  menubar->add("&Help/&About...", 0,
    (Fl_Callback *)Dialog::about, 0, 0);

  // status
  status = new Group(0, window->h() - STATUS_HEIGHT, window->w(), STATUS_HEIGHT, "");
  pos = 8;

  coords = new Fl_Box(FL_FLAT_BOX, pos, 4, 96, 24, "");
  coords->resize(status->x() + pos, status->y() + 4, 96, 24);
  coords->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  coords->copy_label("(0, 0)");
  pos += 96 + 6;

  new Separator(status, pos, 0, STATUS_HEIGHT, Separator::VERTICAL, "");
  pos += 8;

  info = new Fl_Box(FL_FLAT_BOX, pos, 4, window->w() - pos, 24, "");
  info->resize(status->x() + pos, status->y() + 4, window->w() - pos, 24);
  info->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  info->copy_label("Welcome to Rendera!");

  progress = new Fl_Progress(window->w() - 256 - 8, pos, 256, 24);
  progress->resize(status->x() + window->w() - 256 - 8, status->y() + 4, 256, 24);
  progress->minimum(0);
  progress->maximum(100);
  progress->color(0x44444400);
  progress->selection_color(0xbbbbbb00);
  progress->labelcolor(0xffffff00);
  progress->hide();

  status->resizable(0);
  status->end();

  // top
  top = new Group(0, menubar->h(), window->w(), TOP_HEIGHT, "");
  pos = gap;

  zoom_one = new Button(top, pos, 8, 40, 40,
                        "Actual Size (1)", images_zoom_one_png,
                        (Fl_Callback *)zoomOne);
  pos += 40 + gap;

  zoom_in = new Button(top, pos, 8, 40, 40,
                       "Zoom In (+)", images_zoom_in_png,
                       (Fl_Callback *)zoomIn);
  pos += 40 + gap;

  zoom_out = new Button(top, pos, 8, 40, 40,
                        "Zoom Out (-)", images_zoom_out_png,
                        (Fl_Callback *)zoomOut);
  pos += 40 + gap;

  zoom = new StaticText(top, pos, 8, 64, 40, "");
  zoom->labelsize(20);
  pos += 64 + gap;

  new Separator(top, pos, 0, TOP_HEIGHT, Separator::VERTICAL, "");
  pos += 4 + gap;

  grid = new ToggleButton(top, pos, 8, 40, 40,
                          "Show Grid", images_grid_png,
                          (Fl_Callback *)gridEnable);
  pos += 40 + gap;

  gridsnap = new ToggleButton(top, pos, 8, 40, 40,
                          "Snap to Grid", images_gridsnap_png,
                          (Fl_Callback *)gridSnap);
  pos += 96;

  gridx = new InputInt(top, pos, 8, 104, 40,
                       "X:",
                       (Fl_Callback *)gridX, 1, 256);
  gridx->labelsize(18);
  gridx->textsize(18);
  gridx->value("8");
  pos += 104 + 48;

  gridy = new InputInt(top, pos, 8, 104, 40,
                       "Y:",
                       (Fl_Callback *)gridY, 1, 256);
  gridy->labelsize(18);
  gridy->textsize(18);
  gridy->value("8");
  pos += 104 + gap;

  new Separator(top, pos, 0, TOP_HEIGHT, Separator::VERTICAL, "");
  pos += 4 + gap;

  aspect = new Fl_Choice(pos, 8, 160, 40, "");
  aspect->tooltip("Aspect Ratio\n(simulates non-square displays)");
  aspect->textsize(10);
  aspect->resize(top->x() + pos, top->y() + 8, 160, 40);
  aspect->add("Normal (1:1)");
  aspect->add("Wide (2:1)");
  aspect->add("Tall (1:2)");
  aspect->value(0);
  aspect->callback((Fl_Callback *)aspectMode);
  aspect->textsize(16);
  pos += 160 + gap;

  new Separator(top, pos, 0, TOP_HEIGHT, Separator::VERTICAL, "");
  pos += 4 + gap;

  filter = new CheckBox(top, pos, 8, 48, 40,
                            "Filter",
                            (Fl_Callback *)filterToggle);
  filter->tooltip("Filter display\nwhen zoomed out");
  filter->value(0);
  pos += 48 + gap;

  top->resizable(0);
  top->end();

  left_height = window->h() - top->h() - menubar->h() - status->h();

  // tools
  tools = new Group(0, top->h() + menubar->h(),
                    64, left_height,
                    "Tools");

  pos = Group::title_height + gap;

  tool = new Widget(tools, 8, pos, 48, 6 * 48,
                    "Tools", images_tools_png, 48, 48,
                    (Fl_Callback *)toolChange);

  pos += 6 * 48 + gap;

  new Separator(tools, 0, pos, TOOLS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  clone = new ToggleButton(tools, 8, pos, 48, 48,
                           "Clone (Ctrl+Click to set target)",
                           images_clone_png,
                           (Fl_Callback *)cloneEnable);

  pos += 48 + 8;

  origin = new ToggleButton(tools, 8, pos, 48, 48,
                            "Start From Center", images_origin_png,
                            (Fl_Callback *)originEnable);

  pos += 48 + 8;

  constrain = new ToggleButton(tools, 8, pos, 48, 48,
                              "Lock Proportions",
                              images_constrain_png,
                              (Fl_Callback *)constrainEnable);

  tools->resizable(0);
  tools->end();

  // paint
  paint = new Group(TOOLS_WIDTH, top->h() + menubar->h(),
                    OPTIONS_WIDTH, left_height,
                    "Paint");
  pos = Group::title_height + gap;

  paint_brush_preview = new Widget(paint, 8, pos, 160, 160,
                           "Brush Preview", 0, 0, 0);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  pos += 160 + 8;

  paint_size_value = new InputInt(paint, 8, pos, 160, 32,
                       "", (Fl_Callback *)paintSizeValue, 1, Brush::max_size);
  paint_size_value->textsize(16);
  pos += 32 + 8;

  paint_size = new Widget(paint, 8, pos, 160, 32,
                          "Brush Size", images_size_png, 10, 32,
                          (Fl_Callback *)paintSize);
  pos += 32 + 8;

  paint_shape = new Widget(paint, 8, pos, 160, 40,
                           "Shape Adjust", images_shape_png, 10, 40,
                           (Fl_Callback *)paintShape);
  pos += 40 + gap;

  new Separator(paint, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  paint_stroke = new Widget(paint, 8, pos, 160, 80,
                            "Brushstroke Type", images_stroke_png, 40, 40,
                            (Fl_Callback *)paintStroke);

  pos += 80 + gap;

  new Separator(paint, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  paint_mode = new Fl_Choice(8, pos, 160, 32, "");
  paint_mode->tooltip("Rendering Mode");
  paint_mode->textsize(10);
  paint_mode->resize(paint->x() + 8, paint->y() + pos, 160, 32);
  paint_mode->add("Solid");
  paint_mode->add("Antialiased");
  paint_mode->add("Coarse");
  paint_mode->add("Fine");
  paint_mode->add("Blur");
  paint_mode->add("Watercolor");
  paint_mode->add("Chalk");
  paint_mode->add("Texture");
  paint_mode->add("Average");
  paint_mode->value(0);
  paint_mode->textsize(16);
  paint_mode->callback((Fl_Callback *)paintMode);
  pos += 32 + 8;

  paint_coarse_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)paintCoarseEdge);
  paint_fine_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)paintFineEdge);
  paint_blurry_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)paintBlurryEdge);
  paint_watercolor_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_watercolor_edge_png, 20, 32,
                          (Fl_Callback *)paintWatercolorEdge);
  paint_chalk_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_chalk_edge_png, 20, 32,
                          (Fl_Callback *)paintChalkEdge);
  paint_texture_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)paintTextureEdge);
  paint_texture_marb = new Widget(paint, 8, pos + 40, 160, 32,
                          "Marbleize", images_marbleize_png, 20, 32,
                          (Fl_Callback *)paintTextureMarb);
  paint_texture_turb = new Widget(paint, 8, pos + 80, 160, 32,
                          "Turbulence", images_turbulence_png, 20, 32,
                          (Fl_Callback *)paintTextureTurb);
  paint_average_edge = new Widget(paint, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)paintAverageEdge);

  pos += 32 * 4;

  paint->resizable(0);
  paint->end();

  // selection
  selection = new Group(TOOLS_WIDTH, top->h() + menubar->h(),
                   OPTIONS_WIDTH, left_height,
                   "Selection");
  pos = Group::title_height + 8;

  new StaticText(selection, 8, pos, 32, 32, "x:");
  selection_x = new StaticText(selection, 32, pos, 96, 32, 0);
  pos += 24;

  new StaticText(selection, 8, pos, 32, 32, "y:");
  selection_y = new StaticText(selection, 32, pos, 96, 32, 0);
  pos += 24;

  new StaticText(selection, 8, pos, 32, 32, "w:");
  selection_w = new StaticText(selection, 32, pos, 96, 32, 0);
  pos += 24;

  new StaticText(selection, 8, pos, 32, 32, "h:");
  selection_h = new StaticText(selection, 32, pos, 96, 32, 0);
  pos += 24 + gap;

  new Separator(selection, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  selection_reset = new Fl_Button(selection->x() + 8, selection->y() + pos, 160, 48, "Reset");
  selection_reset->callback((Fl_Callback *)selectReset);
  pos += 48 + gap;

  new Separator(selection, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  selection_alpha = new CheckBox(selection, 8, pos, 16, 16, "Alpha Mask",
                                 (Fl_Callback *)selectAlpha);
  selection_alpha->center();
  selection_alpha->value(1);
  pos += 24 + gap;

  selection_mirror = new Button(selection, 8, pos, 48, 48, "Mirror", images_select_mirror_png, (Fl_Callback *)selectFlipX);
  selection_flip = new Button(selection, 8 + 48 + 8, pos, 48, 48, "Flip", images_select_flip_png, (Fl_Callback *)selectFlipY);
  selection_rotate = new Button(selection, 8 + 48 + 8 + 48 + 8, pos, 48, 48, "Rotate", images_select_rotate_png, (Fl_Callback *)selectRotate90);
  pos += 48 + gap;

  new Separator(selection, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  selection_copy = new Fl_Button(selection->x() + 8, selection->y() + pos, 160, 40, "Copy");
  selection_copy->tooltip("Ctrl-C");
  selection_copy->callback((Fl_Callback *)selectCopy);
  selection_copy->deactivate();
  pos += 40 + gap;

  selection_paste = new Fl_Button(selection->x() + 8, selection->y() + pos, 160, 40, "Paste");
  selection_paste->tooltip("Ctrl-V");
  selection_paste->callback((Fl_Callback *)selectPaste);
  selection_paste->deactivate();
  pos += 40 + gap;

  new Separator(selection, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  selection_crop = new Fl_Button(selection->x() + 8, selection->y() + pos, 160, 48, "Crop");
  selection_crop->callback((Fl_Callback *)selectCrop);
  pos += 48 + gap;

  selection->resizable(0);
  selection_crop->deactivate();
  selection->end();

  // getcolor
  getcolor = new Group(TOOLS_WIDTH, top->h() + menubar->h(),
                       OPTIONS_WIDTH, left_height,
                       "Get Color");
  pos = Group::title_height + gap;

  getcolor_color = new Widget(getcolor, 8, pos, 160, 160, "Selected Color", 0, 0, 0);
  getcolor_color->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  pos += 160 + gap + 16;

  new Separator(getcolor, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  getcolor_best = new CheckBox(getcolor, 8, pos, 16, 16, "Best Match", 0);
  getcolor_best->tooltip("Use nearest color in palette");
  getcolor_best->center();
  getcolor_best->value(0);

  getcolor->resizable(0);
  getcolor->end();

  // offset
  offset = new Group(TOOLS_WIDTH, top->h() + menubar->h(),
                     OPTIONS_WIDTH, left_height,
                     "Offset");
  pos = Group::title_height + gap;

  new StaticText(offset, 32 + 8, pos, 32, 24, "x:");
  offset_x = new StaticText(offset, 32 + 24, pos, 72, 24, 0);
  pos += 24;

  new StaticText(offset, 32 + 8, pos, 32, 24, "y:");
  offset_y = new StaticText(offset, 32 + 24, pos, 72, 24, 0);
  pos += 24 + gap;

  new Separator(offset, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  offset_left = new RepeatButton(offset, 16, pos + 26, 40, 40, "", images_left_png, (Fl_Callback *)offsetLeft);
  offset_up = new RepeatButton(offset, 68, pos, 40, 40, "", images_up_png, (Fl_Callback *)offsetUp);
  offset_right = new RepeatButton(offset, 120, pos + 26, 40, 40, "", images_right_png, (Fl_Callback *)offsetRight);
  offset_down = new RepeatButton(offset, 68, pos + 52, 40, 40, "", images_down_png, (Fl_Callback *)offsetDown);
  pos += 92;

  new StaticText(offset, 8, pos, 160, 32, "Nudge");

  offset->resizable(0);
  offset->end();

  // text
  text = new Group(TOOLS_WIDTH, top->h() + menubar->h(),
                   OPTIONS_WIDTH, left_height,
                   "Text");
  pos = Group::title_height + gap;

  // add font names
  font_browse = new Fl_Hold_Browser(8, pos, 160, 384);
  font_browse->labelsize(16);
  font_browse->textsize(16);
  font_browse->resize(text->x() + 8, text->y() + pos, 160, 384);

  for (int i = 0; i < Fl::set_fonts("*"); i++)
  {
    const char *name = Fl::get_font_name((Fl_Font)i, 0);
    font_browse->add(name);
  }

  font_browse->value(1);
  font_browse->callback((Fl_Callback *)textChangedSize);
  pos += 384 + gap;

  // font size
  font_size = new InputInt(text, 64, pos, 96, 32, "Size:",
                           (Fl_Callback *)textChangedSize, 4, 500);
  font_size->value("48");
  pos += 32 + gap;

  font_angle = new InputInt(text, 64, pos, 96, 32, "Angle:",
                           (Fl_Callback *)textChangedSize, -359, 359);
  font_angle->value("0");
  pos += 32 + gap;

  text_input = new Fl_Input(8, pos, 160, 32, "");
  text_input->textsize(16);
  text_input->value("Text");
  text_input->resize(text->x() + 8, text->y() + pos, 160, 32);
  text_input->callback((Fl_Callback *)textChangedSize);
  pos += 32 + gap;

  text_smooth = new CheckBox(text, 8, pos, 16, 16, "Antialiased", 0);
  text_smooth->center();
  text_smooth->value(1);

  text->resizable(0);
  text->end();

  // fill
  fill = new Group(TOOLS_WIDTH, top->h() + menubar->h(),
                   OPTIONS_WIDTH, left_height,
                   "Fill");
  pos = Group::title_height + gap;

  fill_range = new InputInt(fill, 8, pos, 160, 32, "Range (0-31)", 0, 0, 31);
  fill_range->align(FL_ALIGN_BOTTOM);
  fill_range->value("0");
  pos += 32 + 32;

  fill_feather = new InputInt(fill, 8, pos, 160, 32, "Feather (0-255)", 0, 0, 255);
  fill_feather->align(FL_ALIGN_BOTTOM);
  fill_feather->value("0");
  pos += 32 + 32;

  fill_color_only = new CheckBox(fill, 8, pos, 16, 16, "Color Only", 0);
  fill_color_only->center();
  fill_color_only->value(0);
  pos += 32;

  new Separator(fill, 0, pos, OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  fill_reset = new Fl_Button(fill->x() + 8, fill->y() + pos, 160, 48, "Reset");
  fill_reset->callback((Fl_Callback *)fillReset);
  pos += 48 + gap;

  fill->resizable(0);
  fill->end();

  // colors
  colors = new Group(window->w() - COLORS_WIDTH - FILES_WIDTH,
                     top->h() + menubar->h(),
                     COLORS_WIDTH,
                     window->h() - top->h() - menubar->h() - status->h(),
                     "Colors");

  pos = Group::title_height + gap;

  // satval overlaps the hue color wheel
  hue = new Widget(colors, 8, pos, 192, 192, 0, 1, 1, (Fl_Callback *)colorChange);
  satval = new Widget(colors, 8 + 48, pos + 48, 96, 96, 0, 1, 1, (Fl_Callback *)colorChange);

  pos += 192 + 8;

  hexcolor = new InputText(colors, 56, pos, 136, 32, "Hex:",
                           (Fl_Callback *)colorHexInput);
  hexcolor->maximum_size(6);
  hexcolor->textfont(FL_COURIER);
  hexcolor->textsize(18);

  pos += 32 + gap;

  new Separator(colors, 0, pos, COLORS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  trans_input = new InputInt(colors, 8, pos, 192, 32,
                             "", (Fl_Callback *)colorTransInput, 0, 255);
  trans_input->value("0");
  pos += 32 + 8;

  trans = new Widget(colors, 8, pos, 192, 42, "Transparency", 6, 42,
                     (Fl_Callback *)colorTrans);

  pos += 42 + gap;

  new Separator(colors, 0, pos, COLORS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  blend = new Fl_Choice(8, pos, 192, 32, "");
  blend->tooltip("Blending Mode");
  blend->textsize(10);
  blend->resize(colors->x() + 8, colors->y() + pos, 192, 32);
  blend->add("Normal");
  blend->add("Gamma Correct");
  blend->add("Lighten");
  blend->add("Darken");
  blend->add("Colorize");
  blend->add("Luminosity");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->value(0);
  blend->callback((Fl_Callback *)colorChange);
  blend->textsize(16);
  pos += 32 + gap;

  new Separator(colors, 0, pos, COLORS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  palette_swatches = new Widget(colors, 8, pos, 192, 192,
                       0, 12, 12,
                       (Fl_Callback *)paletteSwatches);
  pos += 192 + gap;

  colors->resizable(0);
  colors->end();
  right_height = left_height;

  // files
  files = new Group(window->w() - FILES_WIDTH,
                    top->h() + menubar->h(),
                    FILES_WIDTH,
                    window->h() - top->h() - menubar->h() - status->h(),
                    "Images");
  pos = Group::title_height + gap;

  file_browse = new Fl_Hold_Browser(8, pos, 160, 256);
  file_browse->textsize(14);
  file_browse->resize(files->x() + 8, files->y() + pos, 160, 384);
  file_browse->callback((Fl_Callback *)imagesBrowse);

  pos += file_browse->h() + gap;

  file_close = new Button(files, 8, pos, 48, 48,
                          "Close File (Delete)", images_close_png,
                          (Fl_Callback *)imagesCloseFile);

  file_move_up = new Button(files, 8 + 48 + 8, pos, 48, 48,
                            "Move Up", images_up_large_png,
                            (Fl_Callback *)imagesMoveUp);

  file_move_down = new Button(files, 8 + 48 + 8 + 48 + 8, pos,
                              48, 48,
                              "Move Down", images_down_large_png,
                              (Fl_Callback *)imagesMoveDown);

  pos += 48 + gap;

  file_rename = new Fl_Input(8, pos, 160, 32, "");
  file_rename->value("");
  file_rename->when(FL_WHEN_ENTER_KEY);
  file_rename->resize(files->x() + 8, files->y() + pos, 160, 32);
  file_rename->callback((Fl_Callback *)imagesRename);
  pos += 32 + gap;

  new Separator(files, 0, pos, FILES_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + gap;

  file_mem = new Fl_Box(FL_FLAT_BOX,
                        files->x() + 8, files->y() + pos, 160, 64, "");

  file_mem->labelsize(14);
  file_mem->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);

  files->resizable(file_browse);
  files->end();

  // middle
  middle = new Fl_Group(TOOLS_WIDTH + OPTIONS_WIDTH,
                        top->h() + menubar->h(),
                        window->w() - TOTAL_WIDTH,
                        window->h() - (menubar->h() + top->h() + status->h()));
  middle->box(FL_FLAT_BOX);
  view = new View(middle, 0, 0, middle->w(), middle->h(), "View");
  middle->resizable(view);
  middle->end();

  // container for left panels
  left = new Fl_Group(0, top->h() + menubar->h(),
                            TOOLS_WIDTH + OPTIONS_WIDTH, left_height);
  left->add(tools);
  left->add(paint);
  left->add(getcolor);
  left->add(selection);
  left->add(offset);
  left->add(text);
  left->end();

  // container for right panels
  right = new Fl_Group(window->w() - COLORS_WIDTH - FILES_WIDTH,
                       top->h() + menubar->h(),
                       COLORS_WIDTH + FILES_WIDTH,
                       right_height);

  right->add(files);
  right->add(colors);

  // resize these panels
  colors->resize(colors->x(), colors->y(), colors->w(), right_height);
  files->resize(files->x(), files->y(), files->w(), right_height);

  window->size_range(1024, 828, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();

  // misc init
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

  paletteSetDefault();
  colorUpdate(Project::palette->data[palette_swatches->var]);
  paletteDraw();
  tool->do_callback();
  zoomLevel();
  selectValues(0, 0, 0, 0);
  offsetValues(0, 0);
  paintMode();

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

//  FX::enableProgress(true);

/*
  // fix certain icons if using a light theme
  if (Project::theme == Project::THEME_LIGHT)
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
void Gui::menuCheckItem(const char *s)
{
  Fl_Menu_Item *m;
  m = (Fl_Menu_Item *)menubar->find_item(s);

  if (m)
    m->set();
}

// remove checkmark from menu item
void Gui::menuClearItem(const char *s)
{
  Fl_Menu_Item *m;
  m = (Fl_Menu_Item *)menubar->find_item(s);

  if (m)
    m->clear();
}

// begin callback functions
void Gui::colorHexInput()
{
  unsigned int c;

  sscanf(hexcolor->value(), "%06x", &c);

  if (c > 0xffffff)
    c = 0xffffff;

  c |= 0xff000000;

  colorUpdate(convertFormat((int)c, true));
  colorHexUpdate();
}

void Gui::colorHexUpdate()
{
  char hex_string[8];
  snprintf(hex_string, sizeof(hex_string),
       "%06x", (unsigned)convertFormat(Project::brush->color, true) & 0xffffff);
  hexcolor->value(hex_string);
}

void Gui::colorUpdate(int c)
{
  int r = getr(c);
  int g = getg(c);
  int b = getb(c);

  int h, s, v;

  Blend::rgbToHsv(r, g, b, &h, &s, &v);

  float angle = ((M_PI * 2) / 1536) * h;
  int mx = 96 + 88 * std::cos(angle);
  int my = 96 + 88 * std::sin(angle);
  hue->var = mx + 192 * my;
  satval->var = (int)(s / 2.68) + 96 * (int)(v / 2.68);

  hue->do_callback();

  Project::brush->color = c;
  colorHexUpdate();
//  colorSwatch();
  colorTrans();
}

void Gui::transUpdate(int t)
{
  trans->var = t / 8;
  trans->redraw();
  Project::brush->trans = t;
  char s[16];
  snprintf(s, sizeof(s), "%d", Project::brush->trans);
  trans_input->value(s);
  trans_input->redraw();
  colorTrans();
}

void Gui::getcolorUpdate(int c)
{
  Palette *pal = Project::palette;
  const int index = palette_swatches->var;

  if (getcolor_best->value())
  {
    getcolor_color->bitmap->clear(pal->data[index]);
  }
    else
  {
    getcolor_color->bitmap->clear(c);
  }

  getcolor_color->bitmap->rect(0,
                               0,
                               getcolor_color->bitmap->w - 1,
                               getcolor_color->bitmap->h - 1,
                               makeRgb(0, 0, 0), 0);
  getcolor_color->redraw();
}

void Gui::paletteSwatches(Widget *widget, void *var)
{
  Palette *pal = Project::palette;
  int pos = *(int *)var;

  if (pos > pal->max - 1)
  {
    pos = pal->max - 1;
    widget->var = pos;
  }

  int step = widget->stepx;
  int div = widget->w() / step;

  int x = pos % div;
  int y = pos / div;

  if (y > (pal->max - 1) / div)
  {
    y = (pal->max - 1) / div;
    pos = x + div * y;
    x = pos % div;
    y = pos / div;
    widget->var = pos;
  }

  if (pos > pal->max - 1)
  {
    pos = pal->max - 1;
    x = pos % div;
    y = pos / div;
    widget->var = pos;
  }

  int c = widget->bitmap->getpixel(x * step + 2, y * step + 2);

  pal->draw(widget);
  colorUpdate(c);
}

void Gui::paletteSortValue()
{
  Palette *pal = Project::palette;

  std::sort(pal->data, pal->data + pal->max, sort_value_cb);
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

// sort into grays, low-sat colors, hi-sat colors
void Gui::paletteSortHue()
{
  Palette *pal = Project::palette;

  std::vector<int> bucket(25 * 256, 0);
  std::vector<int> count(25, 0);

  int h, s, v;

  for (int j = 0; j < pal->max; j++)
  {
    const int c = pal->data[j];
    const int r = getr(c);
    const int g = getg(c);
    const int b = getb(c);
    Blend::rgbToHsv(r, g, b, &h, &s, &v);

    if (s == 0)
    {
      bucket[count[0]++] = c;
    }
    else if (s < 128)
    {
      for (int i = 0; i < 12; i++)
      {
        if (h >= i * 128 && h < (i + 1) * 128)
        {
          bucket[(i + 1) * 256 + count[i + 1]++] = c;
        }
      }
    }
      else
    {
      for (int i = 0; i < 12; i++)
      {
        if (h >= i * 128 && h < (i + 1) * 128)
        {
          bucket[(i + 13) * 256 + count[i + 13]++] = c;
        }
      }
    }
  }

  for (int i = 0; i < 25; i++)
  {
    std::sort(&bucket[i * 256], &bucket[i * 256] + count[i], sort_value_cb);
  }

  int index = 0;

  for (int i = 0; i < 25; i++)
  {
    for (int j = 0; j < count[i]; j++)
    {
      pal->data[index++] = bucket[i * 256 + j];
    }
  }

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteNormalize()
{
  Palette *pal = Project::palette;

  // search for highest & lowest RGB values
  int r_high = 0;
  int g_high = 0;
  int b_high = 0;
  int r_low = 0xffffff;
  int g_low = 0xffffff;
  int b_low = 0xffffff;

  for (int i = 0; i < pal->max; i++)
  {
    rgba_type rgba = getRgba(pal->data[i]);

    const int r = rgba.r;
    const int g = rgba.g;
    const int b = rgba.b;

    if (r < r_low)
      r_low = r;
    if (r > r_high)
      r_high = r;
    if (g < g_low)
      g_low = g;
    if (g > g_high)
      g_high = g;
    if (b < b_low)
      b_low = b;
    if (b > b_high)
      b_high = b;
  }

  if (!(r_high - r_low))
    r_high++;
  if (!(g_high - g_low))
    g_high++;
  if (!(b_high - b_low))
    b_high++;

  // scale palette
  double r_scale = 255.0 / (r_high - r_low);
  double g_scale = 255.0 / (g_high - g_low);
  double b_scale = 255.0 / (b_high - b_low);

  for (int i = 0; i < pal->max; i++)
  {
    rgba_type rgba = getRgba(pal->data[i]);

    const int r = (rgba.r - r_low) * r_scale;
    const int g = (rgba.g - g_low) * g_scale;
    const int b = (rgba.b - b_low) * b_scale;

    pal->data[i] = makeRgb(r, g, b);
  }

  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSetDefault()
{
  static int hue[] =
  {
    0, 109, 192, 256, 328, 364, 512, 657,
    768, 864, 940, 1024, 1160, 1280, 1425
  }; 

  static int sat[] = { 192, 255, 144, 96 };
  static int val[] = { 128, 255, 255, 255 };

  Palette *pal = Project::palette;
  int index = 0;

  // grays
  pal->data[index++] = makeRgb(255, 255, 255);
  pal->data[index++] = makeRgb(160, 160, 160);
  pal->data[index++] = makeRgb(96, 96, 96);
  pal->data[index++] = makeRgb(0, 0, 0);

  //colors
  for (int h = 0; h < 15; h++)
  {
    for (int v = 3; v >= 0; v--)
    {
      int r, g, b;

      Blend::hsvToRgb(hue[h] / 32 * 32, sat[v], val[v], &r, &g, &b);
      pal->data[index++] = makeRgb(r, g, b);
    }
  }

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSetBlackAndWhite()
{
  Palette *pal = Project::palette;

  pal->data[0] = makeRgb(0, 0, 0);
  pal->data[1] = makeRgb(255, 255, 255);

  pal->max = 2;
  pal->fillTable();
}

void Gui::paletteSetGrays()
{
  Palette *pal = Project::palette;

  for (int i = 0; i < 16; i++)
  {
    pal->data[i] = makeRgb(i * 17, i * 17, i * 17);
  }

  pal->max = 16;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSetTwoBits()
{
  Palette *pal = Project::palette;
  pal->data[0] = makeRgb(0x00, 0x00, 0x00);
  pal->data[1] = makeRgb(0x4a, 0x4a, 0x4a);
  pal->data[2] = makeRgb(0x7b, 0x7b, 0x7b);
  pal->data[3] = makeRgb(0xb2, 0xb2, 0xb2);

  pal->max = 4;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

// from colodore.com
void Gui::paletteSetC64()
{
  Palette *pal = Project::palette;

  pal->data[0] = makeRgb(0, 0, 0);
  pal->data[1] = makeRgb(255, 255, 255);
  pal->data[2] = makeRgb(129, 51, 56);
  pal->data[3] = makeRgb(117, 206, 200);
  pal->data[4] = makeRgb(142, 60, 151);
  pal->data[5] = makeRgb(86, 172, 77);
  pal->data[6] = makeRgb(46, 44, 155);
  pal->data[7] = makeRgb(237, 241, 113);
  pal->data[8] = makeRgb(142, 80, 41);
  pal->data[9] = makeRgb(85, 56, 0);
  pal->data[10] = makeRgb(196, 108, 113);
  pal->data[11] = makeRgb(74, 74, 74);
  pal->data[12] = makeRgb(123, 123, 123);
  pal->data[13] = makeRgb(169, 255, 159);
  pal->data[14] = makeRgb(112, 109, 235);
  pal->data[15] = makeRgb(178, 178, 178);

  pal->max = 16;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSetVCS()
{
  Palette *pal = Project::palette;
  int index = 0;

  for (int x = 0; x < 8; x++)
  {
    for (int y = 0; y < 16; y++)
    {
      const float luma = 36 + x * 24;
      const float sat = 76 - luma / 16;
      const float hue = (float)y - 0.66;
      const float bias = 6.8;
      const float d = M_PI * hue / bias;
      const float c_blue = y == 0 ? 128 : 128 + sat * -std::cos(d);
      const float c_red = y == 0 ? 128 : 128 + sat * std::sin(d);

      int r = 0, g = 0, b = 0;

      if (y == 0)
        Blend::yccToRgb(x == 0 ? 0 : luma, c_blue, c_red, &r, &g, &b);
      else
        Blend::yccToRgb(luma, c_blue, c_red, &r, &g, &b);

      const int c = makeRgb(r, g, b);

      pal->data[index++] = c;
    }
  }

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSetWebSafe()
{
  Palette *pal = Project::palette;
  int index = 0;

  for (int b = 0; b < 6; b++)
  {
    for (int g = 0; g < 6; g++)
    {
      for (int r = 0; r < 6; r++)
      {
        pal->data[index++] = makeRgb(r * 51, g * 51, b * 51);
      }
    }
  }

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSet3LevelRGB()
{
  Palette *pal = Project::palette;
  int index = 0;

  for (int r = 0; r < 3; r++)
  {
    for (int g = 0; g < 3; g++)
    {
      for (int b = 0; b < 3; b++)
      {
        pal->data[index++] = makeRgb(std::min(r * 128, 255),
                                     std::min(g * 128, 255),
                                     std::min(b * 128, 255));
      }
    }
  }

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSet4LevelRGB()
{
  Palette *pal = Project::palette;
  int index = 0;

  for (int r = 0; r < 4; r++)
  {
    for (int g = 0; g < 4; g++)
    {
      for (int b = 0; b < 4; b++)
      {
        pal->data[index++] = makeRgb(r * 85, g * 85, b * 85);
      }
    }
  }

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteSet332()
{
  Palette *pal = Project::palette;
  int index = 0;

  for (int r = 0; r < 8; r++)
  {
    for (int g = 0; g < 8; g++)
    {
      for (int b = 0; b < 4; b++)
      {
        pal->data[index++] = makeRgb(r << 5, g << 5, b << 6);
      }
    }
  }

  pal->data[255] = makeRgb(255, 255, 255);

  pal->max = index;
  pal->fillTable();
  Gui::palette_swatches->var = 0;
  pal->draw(Gui::palette_swatches);
}

void Gui::paletteIndex(int var)
{
  palette_swatches->var = var;
  Project::palette->draw(palette_swatches);
}

void Gui::paletteDraw()
{
  Project::palette->draw(palette_swatches);
  palette_swatches->var = 0;
  palette_swatches->redraw();
}

void Gui::zoomIn(Button *, void *)
{
  view->zoomIn(view->w() / 2, view->h() / 2);
  zoomLevel();
}

void Gui::zoomOut(Button *, void *)
{
  view->zoomOut(view->w() / 2, view->h() / 2);
  zoomLevel();
}

void Gui::zoomOne(Button *, void *)
{
  view->zoomOne();
  zoomLevel();
}

void Gui::zoomLevel()
{
  char s[256];

  if (view->zoom < 1)
    snprintf(s, sizeof(s), "1/%1.0fx", 1.0 / view->zoom);
  else
    snprintf(s, sizeof(s), "%2.1fx", view->zoom);
    
  zoom->copy_label(s);
  zoom->redraw();
}

void Gui::gridEnable(ToggleButton *, void *var)
{
  view->grid = *(int *)var;
  view->drawMain(true);
  view->redraw();

  if (Project::tool->isActive())
  {
    Project::tool->redraw(view);
  }
}

void Gui::gridSnap(ToggleButton *, void *var)
{
  view->gridsnap = *(int *)var;
}

void Gui::gridX()
{
  view->gridx = atoi(gridx->value());
  view->drawMain(true);
}

void Gui::gridY()
{
  view->gridy = atoi(gridy->value());
  view->drawMain(true);
}

void Gui::paintChangeSize(int size)
{
  Brush *brush = Project::brush;
  float round = (float)(15 - paint_shape->var) / 15;

  brush->make(size, round);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);

  const double aspect = 128.0 / size;

  for (int i = 0; i < brush->solid_count; i++)
  {
    int temp_x = brush->solidx[i];
    int temp_y = brush->solidy[i];

    if (size > 128)
    {
      temp_x *= aspect;
      temp_y *= aspect;
    }

    temp_x += paint_brush_preview->w() / 2;
    temp_y += paint_brush_preview->h() / 2;

    paint_brush_preview->bitmap->setpixelSolid(temp_x, temp_y,
                     convertFormat(getFltkColor(FL_FOREGROUND_COLOR), true), 0);
  }

  paint_brush_preview->redraw();

  char s[32];
  snprintf(s, sizeof(s), "%d", (int)size);

  paint_size_value->value(s);
  paint_size_value->redraw();
}

void Gui::paintSize(Widget *, void *var)
{
  int size = brush_sizes[*(int *)var];
  paintChangeSize(size);
  char s[16];
  snprintf(s, sizeof(s), "%d", (int)size);
  paint_size_value->value(s);
  paint_size_value->redraw();
}

void Gui::paintSizeValue(Widget *, void *)
{
  int size;
  sscanf(paint_size_value->value(), "%d", &size);
  paintChangeSize(size);
}

void Gui::paintShape(Widget *, void *)
{
  paintChangeSize(Project::brush->size);
}

void Gui::paintStroke(Widget *, void *var)
{
  Project::stroke->type = *(int *)var;
}

void Gui::paintCoarseEdge(Widget *, void *var)
{
  Project::brush->coarse_edge = *(int *)var;
}

void Gui::paintFineEdge(Widget *, void *var)
{
  Project::brush->fine_edge = *(int *)var;
}

void Gui::paintBlurryEdge(Widget *, void *var)
{
  Project::brush->blurry_edge = *(int *)var;
}

void Gui::paintWatercolorEdge(Widget *, void *var)
{
  Project::brush->watercolor_edge = *(int *)var;
}

void Gui::paintChalkEdge(Widget *, void *var)
{
  Project::brush->chalk_edge = *(int *)var;
}

void Gui::paintTextureEdge(Widget *, void *var)
{
  Project::brush->texture_edge = *(int *)var;
}

void Gui::paintTextureMarb(Widget *, void *var)
{
  Project::brush->texture_marb = *(int *)var;
}

void Gui::paintTextureTurb(Widget *, void *var)
{
  Project::brush->texture_turb = *(int *)var;
}

void Gui::paintAverageEdge(Widget *, void *var)
{
  Project::brush->average_edge = *(int *)var;
}

void Gui::toolChange(Widget *, void *var)
{
  int tool = *(int *)var;

  if (tool != Tool::PAINT)
    paint->hide();
  if (tool != Tool::GETCOLOR)
    getcolor->hide();
  if (tool != Tool::SELECT)
    selection->hide();
  if (tool != Tool::OFFSET)
    offset->hide();
  if (tool != Tool::TEXT)
    text->hide();
  if (tool != Tool::FILL)
    fill->hide();

  Project::map->clear(0);
  view->drawMain(true);

  switch (tool)
  {
    case Tool::PAINT:
      Project::setTool(Tool::PAINT);
      Project::tool->reset();
      paint_brush_preview->do_callback();
      paint_shape->do_callback();
      paint->show();
      statusInfo((char *)"Middle-click to navigate. Mouse wheel zooms. Esc to cancel rendering.");
      
      break;
    case Tool::GETCOLOR:
      Project::setTool(Tool::GETCOLOR);
      Project::tool->reset();
      getcolor->show();
      statusInfo((char *)"Click to select a color from the image.");
      break;
    case Tool::SELECT:
      Project::setTool(Tool::SELECT);
      //  Project::tool->reset();
      Project::tool->redraw(view);
      selection->show();
      statusInfo((char *)"Draw a box, then click inside box to move, outside to change size.");
      break;
    case Tool::OFFSET:
      Project::setTool(Tool::OFFSET);
      Project::tool->reset();
      offset->show();
      statusInfo((char *)"Click and drag to change image offset.");
      break;
    case Tool::TEXT:
      Project::setTool(Tool::TEXT);
      Project::tool->reset();
      text->show();
      statusInfo((char *)"Click to stamp text onto the image.");
      break;
    case Tool::FILL:
      Project::setTool(Tool::FILL);
      Project::tool->reset();
      fill->show();
      statusInfo((char *)"Click to fill an area with the selected color. Blending modes ignored. Esc to cancel.");
      break;
  }
}

void Gui::colorChange(Widget *widget, void *)
{
  int pos = hue->var;
  int mx = pos % 192;
  int my = pos / 192;

  int dist = 68;
  int inner = dist + 6;
  int outer = dist + 25;
  int center = inner + (outer - inner) / 2;

  if (widget == hue)
  {
    const int md = ((mx - 96) * (mx - 96) + (my - 96) * (my - 96));

    if (md < ((inner - 32) * (inner - 32)))
    {
      hue->redraw();
      satval->redraw();
      return;
    }
  }

  float mouse_angle = atan2f(my - 96, mx - 96);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 96) * 2.69;
  int v = (satval->var / 96) * 2.69;

  int r, g, b;

  //joe
  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  //Blend::wheelToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);
  Project::brush->blend = blend->value();

  // hue circle
  hue->bitmap->clear(Blend::trans(convertFormat(getFltkColor(FL_BACKGROUND_COLOR), true), makeRgb(0, 0, 0), 192));

  for (int i = 1; i < 1536; i++)
  {
    //joe
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 96 + outer * std::cos(angle);
    int y1 = 96 + outer * std::sin(angle);
    int x2 = 96 + inner * std::cos(angle);
    int y2 = 96 + inner * std::sin(angle);
    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 252);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 252);
  }

  for (int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 96 + (outer - 1) * std::cos(angle);
    int y1 = 96 + (outer - 1) * std::sin(angle);
    int x2 = 96 + (inner + 1) * std::cos(angle);
    int y2 = 96 + (inner + 1) * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 248);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 248);
  }

  for (int i = 1; i < 1536; i++)
  {
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 96 + (outer - 3) * std::cos(angle);
    int y1 = 96 + (outer - 3) * std::sin(angle);
    int x2 = 96 + (inner + 3) * std::cos(angle);
    int y2 = 96 + (inner + 3) * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(r, g, b), 0);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(r, g, b), 0);
  }

  const int x1 = 96 + center * std::cos(mouse_angle);
  const int y1 = 96 + center * std::sin(mouse_angle);

  Blend::hsvToRgb(h, 255, 255, &r, &g, &b);
  //Blend::wheelToRgb(h, 255, 255, &r, &g, &b);
  hue->bitmap->rect(x1 - 10, y1 - 10, x1 + 10, y1 + 10, makeRgb(0, 0, 0), 192);
  hue->bitmap->rect(x1 - 9, y1 - 9, x1 + 9, y1 + 9, makeRgb(0, 0, 0), 96);
  hue->bitmap->xorRect(x1 - 8, y1 - 8, x1 + 8, y1 + 8);
  hue->bitmap->rectfill(x1 - 7, y1 - 7, x1 + 7, y1 + 7, makeRgb(r, g, b), 0);

  // saturation/value
  hue->bitmap->rect(48 - 1, 48 - 1, 48 + 95 + 1, 48 + 95 + 1, makeRgb(0, 0, 0), 192);

  for (int y = 0; y < 96; y++)
  {
    for (int x = 0; x < 96; x++)
    {
      Blend::hsvToRgb(h, x * 2.69, y * 2.69, &r, &g, &b);
      //Blend::wheelToRgb(h, x * 4.05, y * 4.05, &r, &g, &b);
      satval->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
    }
  }

  int x = (satval->var % 96);
  int y = (satval->var / 96);

  if (x < 9)
    x = 9;
  if (y < 9)
    y = 9;
  if (x > 86)
    x = 86;
  if (y > 86)
    y = 86;

  satval->bitmap->rect(x - 10, y - 10, x + 10, y + 10, makeRgb(0, 0, 0), 192);
  satval->bitmap->rect(x - 9, y - 9, x + 9, y + 9, makeRgb(0, 0, 0), 96);
  satval->bitmap->xorRect(x - 8, y - 8, x + 8, y + 8);

//  colorSwatch();
  colorTrans();
  colorHexUpdate();
  hue->redraw();
  satval->redraw();

}

void Gui::colorHue(Widget *, void *)
{
  colorChange(0, 0);
}

void Gui::colorSatVal(Widget *, void *)
{
  colorChange(0, 0);
}

void Gui::colorTransInput(Widget *, void *)
{
  Project::brush->trans = atoi(trans_input->value());
  trans->var = Project::brush->trans / 8.22;
//  colorTrans();
}

void Gui::colorTrans()
{
  int temp_trans = trans->var * 8;

  if (temp_trans >= 248)
    temp_trans = 255;

  Project::brush->trans = temp_trans;

  char s[16];
  snprintf(s, sizeof(s), "%d", Project::brush->trans);
  trans_input->value(s);
  trans_input->redraw();

  for (int y = 0; y < trans->bitmap->h; y++)
  {
    for (int x = 0; x < trans->bitmap->w; x++)
    {
      const int checker = ((x / 16) & 1) ^ (((y + 3) / 16) & 1) ? 0xff989898 : 0xff686868;
      trans->bitmap->setpixel(x, y, checker);
    }
  }

  const int c = Project::brush->color;
  const float mul = 255.0 / (trans->bitmap->w - 1);

  for (int x = 0; x < trans->bitmap->w; x++)
  {
    trans->bitmap->vline(0, x, trans->bitmap->h - 1, c, x * mul);
  }

  const int stepx = trans->stepx;
  const int pos = trans->var * stepx;

  trans->bitmap->xorRect(pos + 1, 1, pos + stepx - 2, trans->h() - 2);
  trans->bitmap->rect(pos, 0, pos + stepx - 1, trans->h() - 1, makeRgb(0, 0, 0), 0);
  trans->redraw();
  Project::tool->redraw(view);
}

void Gui::colorBlend(Widget *, void *)
{
  colorChange(0, 0);
}

void Gui::cloneEnable(Widget *, void *var)
{
  Clone::active = *(int *)var;
}

void Gui::originEnable(Widget *, void *var)
{
  Project::stroke->origin = *(int *)var;
}

void Gui::constrainEnable(Widget *, void *var)
{
  Project::stroke->constrain = *(int *)var;
}

// limit mouse framerate
void Gui::mouseTimer()
{
  view->mouse_timer_ready = true;
  Fl::repeat_timeout(1.0 / 125, (Fl_Timeout_Handler)Gui::mouseTimer);
}

void Gui::selectCopy()
{
  Project::tool->done(view, 0);
}

void Gui::selectCopyEnable(bool enable)
{
  if (enable == true)
    selection_copy->activate();
  else
    selection_copy->deactivate();

  selection_copy->redraw();
}

void Gui::selectPaste()
{
  Project::tool->done(view, 2);
}

void Gui::selectPasteEnable(bool enable)
{
  if (enable == true)
    selection_paste->activate();
  else
    selection_paste->deactivate();

  selection_paste->redraw();
}

void Gui::selectAlpha()
{
  Project::selection->redraw(view);
}

void Gui::selectCrop()
{
  Project::tool->done(view, 1);
}

void Gui::selectCropEnable(bool enable)
{
  if (enable == true)
    selection_crop->activate();
  else
    selection_crop->deactivate();

  selection_crop->redraw();
}

void Gui::selectFlipX()
{
  Project::select_bmp->flipHorizontal();
  Project::selection->redraw(view);
}

void Gui::selectFlipY()
{
  Project::select_bmp->flipVertical();
  Project::selection->redraw(view);
}

void Gui::selectRotate90()
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

  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
       *(Project::select_bmp->row[x] + h - 1 - y) = *p++;
    }
  }

  Project::selection->reload();
  Project::selection->redraw(view);
}

void Gui::selectRotate180()
{
  Project::select_bmp->rotate180();
  Project::selection->redraw(view);
}

void Gui::selectReset()
{
  Project::tool->reset();
  Project::selection->redraw(view);
}

void Gui::selectValues(int x, int y, int w, int h)
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

void Gui::selectFromImage()
{
  delete Project::select_bmp;

  Bitmap *bmp = Project::bmp;
  Bitmap *temp = new Bitmap(bmp->cw, bmp->ch);

  bmp->blit(temp, 0, 0, 0, 0, temp->w, temp->h);
  Project::select_bmp = temp;
  Project::selection->reload();
  Project::selection->redraw(view);

  tool->var = Tool::SELECT;
  toolChange(tool, (void *)&tool->var);
}

void Gui::selectToImage()
{
  Bitmap *select_bmp = Project::select_bmp;
  Bitmap *temp = new Bitmap(select_bmp->w, select_bmp->h);
  
  select_bmp->blit(temp, 0, 0, 0, 0, temp->w, temp->h);

  if (Project::newImageFromBitmap(temp) != -1)
    imagesAddFile("new_from_selection");
}

void Gui::offsetValues(int x, int y)
{
  char s[256];

  snprintf(s, sizeof(s), "%d", x);
  offset_x->copy_label(s);
  offset_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  offset_y->copy_label(s);
  offset_y->redraw();
}

void Gui::offsetLeft(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgx = view->gridsnap ? -view->gridx : -1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::offsetRight(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgx = view->gridsnap ? view->gridx : 1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::offsetUp(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgy = view->gridsnap ? -view->gridy : -1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::offsetDown(Widget *, void *)
{
  view->imgx = 0;
  view->imgy = 0;
  Project::tool->push(view);
  view->imgy = view->gridsnap ? view->gridy : 1;
  Project::tool->drag(view);
  Project::tool->release(view);
}

void Gui::aspectMode()
{
  view->changeAspect(aspect->value());
}

void Gui::clearToBlack()
{
  Project::undo->push();

  Bitmap *bmp = Project::bmp;

  bmp->rectfill(bmp->cl, bmp->ct, bmp->cr, bmp->cb, makeRgb(0, 0, 0), 0);
  view->drawMain(true);
}

void Gui::clearToColor()
{
  Project::undo->push();

  Bitmap *bmp = Project::bmp;

  bmp->rectfill(bmp->cl, bmp->ct, bmp->cr, bmp->cb, Project::brush->color, 0);
  view->drawMain(true);
}

void Gui::clearToGray()
{
  Project::undo->push();

  Bitmap *bmp = Project::bmp;

  for (int y = bmp->ct; y <= bmp->cb; y++)
    for (int x = bmp->cl; x <= bmp->cr; x++)
      *(bmp->row[y] + x) = 0xff808080;

  view->drawMain(true);
}

void Gui::clearToTrans()
{
  Project::undo->push();

  Bitmap *bmp = Project::bmp;

  for (int y = bmp->ct; y <= bmp->cb; y++)
    for (int x = bmp->cl; x <= bmp->cr; x++)
      *(bmp->row[y] + x) = 0x00808080;

  view->drawMain(true);
}

void Gui::clearToWhite()
{
  Project::undo->push();

  Bitmap *bmp = Project::bmp;

  bmp->rectfill(bmp->cl, bmp->ct, bmp->cr, bmp->cb, makeRgb(255, 255, 255), 0);
  view->drawMain(true);
}

void Gui::imagesBrowse()
{
  const int line = file_browse->value();

  if (line > 0)
  {
    Project::switchImage(line - 1);

    // restore coords/zoom saved during navigation
    view->ox = Project::ox_list[Project::current];
    view->oy = Project::oy_list[Project::current];
    view->zoom = Project::zoom_list[Project::current];
    view->drawMain(true);

    file_rename->value(file_browse->text(line));
  }
}

void Gui::imagesRename()
{
  const int line = file_browse->value();

  if (line > 0)
  {
    file_browse->text(line, file_rename->value());
    file_browse->redraw();
  }
}

void Gui::imagesAddFile(const char *name)
{
  file_browse->add(name, 0);
  file_browse->select(Project::current + 1);
  imagesBrowse();
}

void Gui::imagesCloseFile()
{
  if (Project::removeImage() == false)
    return;

  file_browse->remove(Project::current + 1);

  if (Project::current > 0)
    file_browse->select(Project::current, 1);
  else
    file_browse->select(Project::current + 1, 1);

  if (Project::current > 0)
    Project::switchImage(Project::current - 1);
  else
    Project::switchImage(Project::current);

  file_rename->value(file_browse->text(file_browse->value()));

  if (Project::last == 0)
  {
    imagesAddFile("new");
    Project::last = 1;
  }

  // restore coords/zoom saved during navigation
  view->ox = Project::ox_list[Project::current];
  view->oy = Project::oy_list[Project::current];
  view->zoom = Project::zoom_list[Project::current];
  view->drawMain(true);
}

void Gui::imagesUpdateMemInfo()
{
  if (Project::last < 1)
    return;

  char s[256];

  double mem = Project::getImageMemory() / 1000000;
  double max = Project::mem_max;

  bool mem_gb = false;
  bool max_gb = false;

  if (mem >= 1000)
  {
    mem /= 1000;
    mem_gb = true;
  }

  if (max >= 1000)
  {
    max /= 1000;
    max_gb = true;
  }

  const int undos =
    Project::undo_max - Project::undo_list[Project::current]->undo_current - 1;
  const int redos =
    Project::undo_max - Project::undo_list[Project::current]->redo_current - 1;

  snprintf(s, sizeof(s), "%.1lf %s / %.1lf %s used\n%d/%d undos, %d redos",
          mem, mem_gb ? "GB" : "MB", max, max_gb ? "GB" : "MB",
          undos, Project::undo_max, redos);

  file_mem->copy_label(s);

  Fl::repeat_timeout(1.0, (Fl_Timeout_Handler)Gui::imagesUpdateMemInfo);
}

void Gui::imagesDuplicate()
{
  const int current = Project::current;
  const int last = Project::last;
  Bitmap **bmp_list = Project::bmp_list;
  Bitmap *bmp = Project::bmp_list[current];

  if (Project::enoughMemory(bmp->w, bmp->h) == false)
    return;

  Project::newImage(bmp->cw, bmp->ch);
  bmp_list[current]->blit(bmp_list[last], 0, 0, 0, 0, bmp->w, bmp->h);

  imagesAddFile("new");
}

void Gui::imagesMoveUp()
{
  int temp = Project::current;

  if (Project::swapImage(temp, temp - 1) == true)
  {
    file_browse->swap(temp + 1, temp); 
    file_browse->select(temp);
    imagesBrowse();
  }
}

void Gui::imagesMoveDown()
{
  int temp = Project::current;

  if (Project::swapImage(temp, temp + 1) == true)
  {
    file_browse->swap(temp + 1, temp + 2); 
    file_browse->select(temp + 2);
    imagesBrowse();
  }
}

Fl_Double_Window *Gui::getWindow()
{
  return window;
}

Fl_Menu_Bar *Gui::getMenuBar()
{
  return menubar;
}

Fl_Group *Gui::getStatus()
{
  return status;
}

View *Gui::getView()
{
  return view;
}

void Gui::paintMode()
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

  switch (paint_mode->value())
  {
    case Render::SOLID:
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

void Gui::fillReset()
{
  fill_range->value("0");
  fill_feather->value("0");
}

void Gui::filterToggle()
{
  view->filter = filter->value() == 1 ? true : false;
  view->drawMain(true);
}

void Gui::textChangedSize(InputInt *input, void *)
{
  input->redraw();
  Project::tool->move(view);
}

/*
// use default interval
void Gui::progressShow(float step)
{
  if (progress_enable == false)
    return;

  if (step == 0)
    step = .001;

  view->rendering = true;
  progress_value = 0;
  progress_interval = 50;
  progress_step = 100.0 / (step / progress_interval);
  // keep progress bar on right side in case window was resized
  progress->resize(status->x() + window->w() - 256 - 8, status->y() + 4, 256, 16);
  info->hide();
  progress->show();
}

// custom interval
void Gui::progressShow(float step, int interval)
{
  if (progress_enable == false)
    return;

  if (step == 0)
    step = .001;

  if (interval < 1)
     interval = 1;

  view->rendering = true;
  progress_value = 0;
  progress_interval = interval;
  progress_step = 100.0 / (step / progress_interval);
  // keep progress bar on right side in case window was resized
  progress->resize(status->x() + window->w() - 256 - 8, status->y() + 4, 256, 16);
  info->hide();
  progress->show();
}

int Gui::progressUpdate(int y)
{
  if (progress_enable == false)
    return 0;

  // user cancelled operation
  if (Fl::get_key(FL_Escape))
  {
    progressHide();
    Gui::getView()->drawMain(true);
    return -1;
  }

  if (!(y % progress_interval))
  {
    progress->value(progress_value);
    char percent[16];
    snprintf(percent, sizeof(percent), "%d%%", (int)progress_value);
    progress->copy_label(percent);
    Fl::check();
    progress_value += progress_step;
    view->drawMain(true);
  }

  return 0;
}

void Gui::progressHide()
{
  if (progress_enable == false)
    return;

  view->drawMain(true);
  progress->value(0);
  progress->copy_label("");
  progress->redraw();
  progress->hide();
  info->show();
  view->rendering = false;
}

// hack to externally enable/disable progress indicator
// allows filters to be used internally
void Gui::progressEnable(bool state)
{
  progress_enable = state;
}
*/

void Gui::statusCoords(char *s)
{
  coords->copy_label(s);
  coords->redraw();
}

void Gui::statusInfo(char *s)
{
  info->copy_label(s);
  info->redraw();
}

