/*
Copyright (c) 2024 Joe Davisson.

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
  Group *files;
  Group *bottom;
  Group *status;
  Fl_Group *middle;

  // status
  Fl_Progress *progress;
  Fl_Box *coords;
  Fl_Box *info;

  // top
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  StaticText *zoom;
  ToggleButton *grid;
  ToggleButton *gridsnap;
  InputInt *gridx;
  InputInt *gridy;
  Fl_Choice *aspect;
  Fl_Choice *view_mode;

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
  Fl_Choice *paint_mode;

  Widget *getcolor_color;

  InputInt *fill_range;
  InputInt *fill_feather;
  CheckBox *fill_color_only;
  Fl_Button *fill_reset;

  StaticText *selection_x;
  StaticText *selection_y;
  StaticText *selection_w;
  StaticText *selection_h;
  Fl_Button *selection_reset;
  Fl_Button *selection_copy;
  CheckBox *selection_alpha;
  Button *selection_flip;
  Button *selection_mirror;
  Button *selection_rotate;
  Fl_Button *selection_paste;
  Fl_Button *selection_crop;

  StaticText *offset_x;
  StaticText *offset_y;
  RepeatButton *offset_up;
  RepeatButton *offset_left;
  RepeatButton *offset_right;
  RepeatButton *offset_down;

  Fl_Hold_Browser *font_browse;
  InputInt *font_size;
  Fl_Input *text_input;
  CheckBox *text_smooth;

  // palette
  Widget *palette_swatches;
  Fl_Button *palette_editor;

  // colors
  Widget *swatch;
  Widget *hue;
  Widget *satval;
  InputText *hexcolor;
  InputInt *trans_input;
  Widget *trans;
  Fl_Choice *blend;

  // files
  Fl_Hold_Browser *file_browse;
  Fl_Input *file_rename;
  Button *file_close;
  Button *file_move_up;
  Button *file_move_down;
  Fl_Box *file_mem;
  Fl_Group *file_button_group;

  // bottom
  ToggleButton *clone;
  ToggleButton *origin;
  ToggleButton *constrain;

  // view
  View *view;

  // height of rightmost panels
  int left_height = 0;
  int right_height = 0;

  // progress indicator related
  float progress_value = 0;
  float progress_step = 0;
  int progress_interval = 50;
  bool progress_enable = true;

  // tables
  const int brush_sizes[16] =
  {
    1, 2, 3, 4, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72
  };

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
          break;
        }

        // inhibit use of most keys while rendering
        if (view->rendering)
          return 0;

        // misc keys
        switch (/*key =*/ Fl::event_key())
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
          case FL_Delete:
            Gui::imagesCloseFile();
            break;
          case '1':
            view->zoomOne();
            break;
          case '+':
          case '=':
            view->zoomIn(view->w() / 2, view->h() / 2);
            break;
          case '-':
            view->zoomOut(view->w() / 2, view->h() / 2);
            break;
          case 'z':
            if (ctrl && shift)
              Project::undo->popRedo();
            else if (ctrl)
              Project::undo->pop();
            break;
          case 'c':
            if (ctrl)
              Gui::selectCopy();
            break;
          case 'v':
            if (ctrl)
              Gui::selectPaste();
            break;
          case 'e':
            Editor::begin();
            break;
          default:
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
    (Fl_Callback *)paletteDefault, 0, 0);
  menubar->add("Palette/Presets/Black and White", 0,
    (Fl_Callback *)paletteBlackAndWhite, 0, 0);
  menubar->add("Palette/Presets/Grays", 0,
    (Fl_Callback *)paletteGrays, 0, 0);
  menubar->add("Palette/Presets/Two Bits", 0,
    (Fl_Callback *)paletteTwoBits, 0, 0);
  menubar->add("Palette/Presets/C64", 0,
    (Fl_Callback *)paletteC64, 0, 0);
  menubar->add("&Palette/Presets/Web Safe", 0,
    (Fl_Callback *)paletteWebSafe, 0, 0);
  menubar->add("&Palette/Presets/3-level RGB", 0,
    (Fl_Callback *)palette3LevelRGB, 0, 0);
  menubar->add("&Palette/Presets/4-level RGB", 0,
    (Fl_Callback *)palette4LevelRGB, 0, 0);
  menubar->add("&Palette/&Editor... (E)", 0,
    (Fl_Callback *)Editor::begin, 0, 0);

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
  progress->selection_color(0xbbbbbb00);
  progress->labelcolor(0xffffff00);
  progress->hide();

  status->resizable(0);
  status->end();

  // top
  top = new Group(0, menubar->h(), window->w(), 40, "");
  pos = 8;

  zoom_one = new Button(top, pos, 8, 24, 24,
                        "Actual Size (1)", images_zoom_one_png,
                        (Fl_Callback *)zoomOne);
  pos += 24 + 8;

  zoom_in = new Button(top, pos, 8, 24, 24,
                       "Zoom In (+)", images_zoom_in_png,
                       (Fl_Callback *)zoomIn);
  pos += 24 + 8;

  zoom_out = new Button(top, pos, 8, 24, 24,
                        "Zoom Out (-)", images_zoom_out_png,
                        (Fl_Callback *)zoomOut);
  pos += 24 + 8;

  zoom = new StaticText(top, pos, 8, 56, 24, "");
  pos += 56 + 6;

  new Separator(top, pos, 4, 2, 34, "");
  pos += 8;

  grid = new ToggleButton(top, pos, 8, 24, 24,
                          "Show Grid", images_grid_png,
                          (Fl_Callback *)gridEnable);
  pos += 24 + 8;

  gridsnap = new ToggleButton(top, pos, 8, 24, 24,
                          "Snap to Grid", images_gridsnap_png,
                          (Fl_Callback *)gridSnap);
  pos += 50;

  gridx = new InputInt(top, pos, 8, 64, 24,
                       "X:",
                       (Fl_Callback *)gridX, 1, 256);
  gridx->value("8");
  pos += 92;

  gridy = new InputInt(top, pos, 8, 64, 24,
                       "Y:",
                       (Fl_Callback *)gridY, 1, 256);
  gridy->value("8");
  pos += 64 + 8;

  new Separator(top, pos, 4, 2, 34, "");

  pos += 8;

  aspect = new Fl_Choice(pos, 8, 96, 24, "");
  aspect->tooltip("Aspect Ratio");
  aspect->textsize(10);
  aspect->resize(top->x() + pos, top->y() + 8, 96, 24);
  aspect->add("Normal (1:1)");
  aspect->add("Wide (2:1)");
  aspect->add("Tall (1:2)");
  aspect->value(0);
  aspect->callback((Fl_Callback *)aspectMode);

  pos += 96 + 8;

  new Separator(top, pos, 4, 2, 34, "");

  pos += 8;

  view_mode = new Fl_Choice(pos, 8, 104, 24, "");
  view_mode->tooltip("View Mode");
  view_mode->textsize(10);
  view_mode->resize(top->x() + pos, top->y() + 8, 104, 24);
  view_mode->add("Full Color");
  view_mode->add("Palette Colors");
  view_mode->value(0);
  view_mode->callback((Fl_Callback *)viewMode);

  top->resizable(0);
  top->end();

  // bottom
  bottom = new Group(160, window->h() - status->h() - 40,
                     window->w() - 304 - 80, 40, "");
  pos = 8;

  clone = new ToggleButton(bottom, pos, 8, 24, 24,
                           "Clone (Ctrl+Click to set target)",
                           images_clone_png,
                           (Fl_Callback *)cloneEnable);
  pos += 24 + 8;

  new Separator(bottom, pos, 4, 2, 34, "");
  pos += 8;

  origin = new ToggleButton(bottom, pos, 8, 24, 24,
                            "Start From Center", images_origin_png,
                            (Fl_Callback *)originEnable);
  pos += 24 + 8;

  constrain = new ToggleButton(bottom, pos, 8, 24, 24,
                              "Lock Proportions",
                              images_constrain_png,
                              (Fl_Callback *)constrainEnable);
  pos += 24 + 8;

  bottom->resizable(0);
  bottom->end();

  left_height = window->h() - top->h() - menubar->h() - status->h();

  // tools
  tools = new Group(0, top->h() + menubar->h(),
                    48, left_height,
                    "Tools");
  pos = 28;

  tool = new Widget(tools, 8, pos, 32, 6 * 32,
                    "Tools", images_tools_png, 32, 32,
                    (Fl_Callback *)toolChange);
  pos += 96 + 8;

  tools->resizable(0);
  tools->end();

  // paint
  paint = new Group(48, top->h() + menubar->h(),
                    112, left_height,
                    "Paint");
  pos = 28;

  paint_brush_preview = new Widget(paint, 8, pos, 96, 96 + 20,
                           "Preview", 0, 0, 0);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  pos += 96;

  paint_size_value = new InputInt(paint, 8, pos, 96, 24,
                       "", (Fl_Callback *)paintSizeValue, 1, 72);
  pos += 24 + 8;

  paint_size = new Widget(paint, 8, pos, 96, 24,
                          "Size", images_size_png, 6, 24,
                          (Fl_Callback *)paintSize);
  pos += 24 + 8;

  paint_stroke = new Widget(paint, 8, pos, 96, 48,
                            "Stroke", images_stroke_png, 24, 24,
                            (Fl_Callback *)paintStroke);
  pos += 48 + 8;

  paint_shape = new Widget(paint, 8, pos, 96, 24,
                           "Shape", images_shape_png, 6, 24,
                           (Fl_Callback *)paintShape);
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
  paint_mode->add("Blur");
  paint_mode->add("Watercolor");
  paint_mode->add("Chalk");
  paint_mode->add("Texture");
  paint_mode->add("Average");
  paint_mode->value(0);
  paint_mode->callback((Fl_Callback *)paintMode);
  pos += 24 + 8;

  paint_coarse_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)paintCoarseEdge);
  paint_fine_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)paintFineEdge);
  paint_blurry_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)paintBlurryEdge);
  paint_watercolor_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_watercolor_edge_png, 12, 24,
                          (Fl_Callback *)paintWatercolorEdge);
  paint_chalk_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_chalk_edge_png, 12, 24,
                          (Fl_Callback *)paintChalkEdge);
  paint_texture_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)paintTextureEdge);
  paint_texture_marb = new Widget(paint, 8, pos + 32, 96, 16,
                          "Marbleize", images_marbleize_png, 12, 16,
                          (Fl_Callback *)paintTextureMarb);
  paint_texture_turb = new Widget(paint, 8, pos + 56, 96, 16,
                          "Turbulence", images_turbulence_png, 12, 16,
                          (Fl_Callback *)paintTextureTurb);
  paint_average_edge = new Widget(paint, 8, pos, 96, 24,
                          "Edge", images_edge_png, 12, 24,
                          (Fl_Callback *)paintAverageEdge);
  pos += 24 + 8;

  paint->resizable(0);
  paint->end();

  // selection
  selection = new Group(48, top->h() + menubar->h(),
                   112, left_height,
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
  selection_reset->callback((Fl_Callback *)selectReset);
  pos += 32 + 8;

  new Separator(selection, 4, pos, 106, 2, "");
  pos += 8;

  selection_alpha = new CheckBox(selection, 8, pos, 16, 16, "Alpha Mask", 0);
  selection_alpha->labelsize(13);
  selection_alpha->center();
  selection_alpha->value(1);
  pos += 16 + 8;

  selection_mirror = new Button(selection, 8, pos, 30, 30, "Mirror", images_select_mirror_png, (Fl_Callback *)selectFlipX);
  selection_flip = new Button(selection, 8 + 33, pos, 30, 30, "Flip", images_select_flip_png, (Fl_Callback *)selectFlipY);
  selection_rotate = new Button(selection, 8 + 66, pos, 30, 30, "Rotate", images_select_rotate_png, (Fl_Callback *)selectRotate90);
  pos += 30 + 8;

  new Separator(selection, 4, pos, 106, 2, "");
  pos += 8;

  selection_copy = new Fl_Button(selection->x() + 8, selection->y() + pos, 96, 32, "Copy");
  selection_copy->tooltip("Control-C");
  selection_copy->callback((Fl_Callback *)selectCopy);
  selection_copy->deactivate();
  pos += 32 + 8;

  selection_paste = new Fl_Button(selection->x() + 8, selection->y() + pos, 96, 32, "Paste");
  selection_paste->tooltip("Control-V");
  selection_paste->callback((Fl_Callback *)selectPaste);
  selection_paste->deactivate();
  pos += 32 + 8;

  new Separator(selection, 4, pos, 106, 2, "");
  pos += 8;

  selection_crop = new Fl_Button(selection->x() + 8, selection->y() + pos, 96, 32, "Crop");
  selection_crop->callback((Fl_Callback *)selectCrop);
  pos += 32 + 8;

  selection->resizable(0);
  selection_crop->deactivate();
  selection->end();

  // getcolor
  getcolor = new Group(48, top->h() + menubar->h(),
                       112, left_height,
                       "Get Color");
  pos = 28;

  getcolor_color = new Widget(getcolor, 8, pos, 96, 96, "Selected Color", 0, 0, 0);
  getcolor_color->align(FL_ALIGN_CENTER | FL_ALIGN_BOTTOM);
  getcolor_color->labelsize(12);
  pos += 96 + 24;

  getcolor->resizable(0);
  getcolor->end();

  // offset
  offset = new Group(48, top->h() + menubar->h(),
                     112, left_height,
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

  offset_up = new RepeatButton(offset, 44, pos, 24, 24, "", images_up_png, (Fl_Callback *)offsetUp);
  pos += 12;

  offset_left = new RepeatButton(offset, 16, pos, 24, 24, "", images_left_png, (Fl_Callback *)offsetLeft);
  offset_right = new RepeatButton(offset, 72, pos, 24, 24, "", images_right_png, (Fl_Callback *)offsetRight);
  pos += 16;

  offset_down = new RepeatButton(offset, 44, pos, 24, 24, "", images_down_png, (Fl_Callback *)offsetDown);
  pos += 24;

  new StaticText(offset, 8, pos, 96, 24, "Nudge");

  offset->resizable(0);
  offset->end();

  // text
  text = new Group(48, top->h() + menubar->h(),
                   112, left_height,
                   "Text");
  pos = 28;

  // add font names
  font_browse = new Fl_Hold_Browser(8, pos, 96, 192);
  font_browse->textsize(9);
  font_browse->resize(text->x() + 8, text->y() + pos, 96, 192);

  for (int i = 0; i < Fl::set_fonts(0); i++)
  {
    int t = 0;
    const char *name = Fl::get_font_name((Fl_Font)i, &t);
    font_browse->add(name);
  }

  font_browse->value(0);
  font_browse->callback((Fl_Callback *)textChangedSize);
  pos += 192 + 8;

  // font size
  font_size = new InputInt(text, 40, pos, 64, 24, "Size:",
                           (Fl_Callback *)textChangedSize, 1, 256);
  font_size->value("24");
  pos += 24 + 8;

  text_input = new Fl_Input(8, pos, 96, 24, "");
  text_input->textsize(10);
  text_input->value("Text");
  text_input->resize(text->x() + 8, text->y() + pos, 96, 24);
  text_input->callback((Fl_Callback *)textChangedSize);
  pos += 24 + 8;

  text_smooth = new CheckBox(text, 8, pos, 16, 16, "Antialiased", 0);
  text_smooth->labelsize(13);
  text_smooth->center();
  text_smooth->value(1);

  text->resizable(0);
  text->end();

  // fill
  fill = new Group(48, top->h() + menubar->h(),
                   112, left_height,
                   "Fill");
  pos = 28 + 8;

  fill_range = new InputInt(fill, 8, pos, 96, 24, "Range (0-31)", 0, 0, 31);
  fill_range->align(FL_ALIGN_BOTTOM);
  fill_range->value("0");
  pos += 24 + 24;

  fill_feather = new InputInt(fill, 8, pos, 96, 24, "Feather (0-255)", 0, 0, 255);
  fill_feather->align(FL_ALIGN_BOTTOM);
  fill_feather->value("0");
  pos += 24 + 24;

  fill_color_only = new CheckBox(fill, 8, pos, 16, 16, "Color Only", 0);
  fill_color_only->labelsize(13);
  fill_color_only->center();
  fill_color_only->value(0);
  pos += 24;

  new Separator(fill, 4, pos, 106, 2, "");
  pos += 8;

  fill_reset = new Fl_Button(fill->x() + 8, fill->y() + pos, 96, 32, "Reset");
  fill_reset->callback((Fl_Callback *)fillReset);
  pos += 32 + 8;

  fill->resizable(0);
  fill->end();

  // palette
  palette = new Group(window->w() - 80, top->h() + menubar->h(),
                    80, window->h() - top->h() - menubar->h() - status->h(),
                    "Palette");
  pos = 28;

  palette_swatches = new Widget(palette, 8, pos, 64, 256,
                       0, 8, 8,
                       (Fl_Callback *)paletteSwatches);
  pos += 256 + 8;

  palette_editor = new Fl_Button(palette->x() + 8, palette->y() + pos, 64, 24, "Editor...");
  palette_editor->callback((Fl_Callback *)Editor::begin);
  palette_editor->labelsize(12);

  palette->resizable(0);
  palette->end();

  // colors
  colors = new Group(window->w() - 144 - 80, top->h() + menubar->h(),
                    144, window->h() - top->h() - menubar->h() - status->h(),
                    "Colors");
  pos = 28;

  swatch = new Widget(colors, 8, pos, 128, 48, "Paint Color", 0, 0, 0);
  pos += 48 + 8;

  hexcolor = new InputText(colors, 40, pos, 96, 24, "Hex:",
                           (Fl_Callback *)colorHexInput);
  hexcolor->maximum_size(6);
  hexcolor->textfont(FL_COURIER);
  hexcolor->textsize(14);
  pos += 24 + 8;

  // satval overlaps the hue color wheel
  hue = new Widget(colors, 8, pos, 128, 128, 0, 1, 1, (Fl_Callback *)colorChange);
  satval = new Widget(colors, 40, pos + 32, 64, 64, 0, 1, 1, (Fl_Callback *)colorChange);
  pos += 128 + 8;

  new Separator(colors, 4, pos, 138, 2, "");
  pos += 8;

  trans_input = new InputInt(colors, 8, pos, 128, 24,
                       "", (Fl_Callback *)colorTransInput, 0, 255);
  trans_input->value("0");
  pos += 24 + 8;

  trans = new Widget(colors, 8, pos, 128, 24,
                     "Transparency", images_transparency_png, 4, 24,
                     (Fl_Callback *)colorTrans);
  pos += 24 + 8;

  new Separator(colors, 4, pos, 138, 2, "");
  pos += 8;

  blend = new Fl_Choice(8, pos, 128, 24, "");
  blend->tooltip("Blending Mode");
  blend->textsize(10);
  blend->resize(colors->x() + 8, colors->y() + pos, 128, 24);
  blend->add("Normal");
  blend->add("Non-Linear");
  blend->add("Lighten");
  blend->add("Darken");
  blend->add("Colorize");
  blend->add("Luminosity");
  blend->add("Alpha Add");
  blend->add("Alpha Subtract");
  blend->add("Smooth");
  blend->value(0);
  blend->callback((Fl_Callback *)colorChange);
  pos += 24 + 8;

  colors->resizable(0);
  colors->end();
  right_height = pos;

  // files
  files = new Group(window->w() - colors->w() - palette->w(),
                    top->h() + menubar->h() + pos,
                    colors->w() + palette->w(),
                    window->h() - top->h() - menubar->h() - status->h() - right_height, "Images");
  pos = 28;

  file_browse = new Fl_Hold_Browser(8, pos, 168, 200);
  file_browse->textsize(12);
  file_browse->resize(files->x() + 8, files->y() + pos, 168, 200);
  file_browse->callback((Fl_Callback *)imagesBrowse);

  file_button_group = new Fl_Group(files->x() + 184, files->y() + 28,
                                48, 112, "");

  file_close = new Button(file_button_group, 0, 0, 32, 32,
                          "Close File (Delete)", images_close_png,
                          (Fl_Callback *)imagesCloseFile);

  file_move_up = new Button(file_button_group, 0, 40, 32, 32,
                            "Move Up", images_up_large_png,
                            (Fl_Callback *)imagesMoveUp);

  file_move_down = new Button(file_button_group, 0, 80, 32, 32,
                              "Move Down", images_down_large_png,
                              (Fl_Callback *)imagesMoveDown);

  file_button_group->box(FL_NO_BOX);
  file_button_group->resizable(0);
  file_button_group->end();

  pos += file_browse->h() + 8;

  file_rename = new Fl_Input(8, pos, 168, 24, "");
  file_rename->textsize(12);
  file_rename->value("");
  file_rename->when(FL_WHEN_ENTER_KEY);
  file_rename->resize(files->x() + 8, files->y() + pos, 168, 24);
  file_rename->callback((Fl_Callback *)imagesRename);
  pos += 24 + 8;

  new Separator(files, 4, pos, 216, 2, "");
  pos += 8;

  file_mem = new Fl_Box(FL_FLAT_BOX, files->x() + 8, files->y() + pos, 144, 32, "");

  file_mem->labelsize(10);
  file_mem->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);

  files->resizable(file_browse);
  files->end();

  // middle
  middle = new Fl_Group(160, top->h() + menubar->h(),
                        window->w() - 304 - 80, window->h() - (menubar->h() + top->h() + bottom->h() + status->h()));
  middle->box(FL_FLAT_BOX);
  view = new View(middle, 0, 0, middle->w(), middle->h(), "View");
  middle->resizable(view);
  middle->end();

  // container for left panels
  left = new Fl_Group(0, top->h() + menubar->h(),
                            64 + 96, left_height);
  left->add(tools);
  left->add(paint);
  left->add(getcolor);
  left->add(selection);
  left->add(offset);
  left->add(text);
  left->end();

  // container for right panels
  right = new Fl_Group(window->w() - 144 - 80,
                       top->h() + menubar->h(),
                       144 + 80,
                       right_height);
//                       window->h() - top->h() - menubar->h());
  right->add(palette);
  right->add(colors);

  // resize these panels
  colors->resize(colors->x(), colors->y(), colors->w(), right_height);
  palette->resize(palette->x(), palette->y(), palette->w(), right_height);

  window->size_range(1024, 768, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();

  // misc init
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

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
  int mx = 64 + 56 * std::cos(angle);
  int my = 64 + 56 * std::sin(angle);
  hue->var = mx + 128 * my;
  satval->var = (int)(s / 4.05) + 64 * (int)(v / 4.05);

  hue->do_callback();

  Project::brush->color = c;
  colorHexUpdate();
  colorSwatch();
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
  colorSwatch();
}

void Gui::colorSwatch()
{
  swatch->bitmap->clear(getFltkColor(FL_BACKGROUND2_COLOR));

  for (int y = 1; y < swatch->bitmap->h - 1; y++)
  {
    int *p = swatch->bitmap->row[y] - 1;
    for (int x = 0; x < swatch->bitmap->w; x++)
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

void Gui::getcolorUpdate(int c)
{
  getcolor_color->bitmap->clear(c);
  getcolor_color->bitmap->rect(0, 0, getcolor_color->bitmap->w - 1, getcolor_color->bitmap->h - 1, makeRgb(0, 0, 0), 0);
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

void Gui::paletteIndex(int var)
{
  palette_swatches->var = var;
  Project::palette->draw(palette_swatches);
}

int Gui::getPaletteIndex()
{
  return palette_swatches->var;
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
    snprintf(s, sizeof(s), "1/%1.0f", 1.0 / view->zoom);
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
  view->redraw();
}

void Gui::gridY()
{
  view->gridy = atoi(gridy->value());
  view->drawMain(true);
  view->redraw();
}

void Gui::paintChangeSize(int size)
{
  Brush *brush = Project::brush;
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

  for (int i = 0; i < brush->solid_count; i++)
  {
    paint_brush_preview->bitmap->setpixelSolid(48 + brush->solidx[i],
                                       48 + brush->solidy[i],
                                       convertFormat(getFltkColor(FL_FOREGROUND_COLOR), true),
                                       0);
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

void Gui::paintSizeValue(Widget *, void *var)
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
  int mx = pos % 128;
  int my = pos / 128;

  if (widget == hue)
  {
    if (((mx - 64) * (mx - 64) + (my - 64) * (my - 64)) < (30 * 30))
    {
      satval->redraw();
      return;
    }
  }

  float mouse_angle = atan2f(my - 64, mx - 64);
  int h = ((int)(mouse_angle * 244.46) + 1536) % 1536;
  int s = (satval->var % 64) * 4.05;
  int v = (satval->var / 64) * 4.05;

  int r, g, b;

  //joe
  Blend::hsvToRgb(h, s, v, &r, &g, &b);
  //Blend::wheelToRgb(h, s, v, &r, &g, &b);
  Project::brush->color = makeRgb(r, g, b);
  Project::brush->blend = blend->value();

  // hue circle
  hue->bitmap->clear(blendFast(convertFormat(getFltkColor(FL_BACKGROUND_COLOR), true), makeRgb(0, 0, 0), 192));

  for (int i = 1; i < 1536; i++)
  {
    //joe
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 64 + 62 * std::cos(angle);
    int y1 = 64 + 62 * std::sin(angle);
    int x2 = 64 + 50 * std::cos(angle);
    int y2 = 64 + 50 * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 252);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 252);
  }

  for (int i = 1; i < 1536; i++)
  {
    //joe
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 64 + 61 * std::cos(angle);
    int y1 = 64 + 61 * std::sin(angle);
    int x2 = 64 + 51 * std::cos(angle);
    int y2 = 64 + 51 * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(0, 0, 0), 252);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(0, 0, 0), 252);
  }

  for (int i = 1; i < 1536; i++)
  {
    //joe
    Blend::hsvToRgb(i, 255, 255, &r, &g, &b);
    //Blend::wheelToRgb(i, 255, 255, &r, &g, &b);

    float angle = ((M_PI * 2) / 1536) * i;
    int x1 = 64 + 60 * std::cos(angle);
    int y1 = 64 + 60 * std::sin(angle);
    int x2 = 64 + 52 * std::cos(angle);
    int y2 = 64 + 52 * std::sin(angle);

    hue->bitmap->line(x1, y1, x2, y2, makeRgb(r, g, b), 0);
    hue->bitmap->line(x1 + 1, y1, x2 + 1, y2, makeRgb(r, g, b), 0);
  }

  const int x1 = 64 + 56 * std::cos(mouse_angle);
  const int y1 = 64 + 56 * std::sin(mouse_angle);

  //joe
  Blend::hsvToRgb(h, 255, 255, &r, &g, &b);
  //Blend::wheelToRgb(h, 255, 255, &r, &g, &b);
  hue->bitmap->rect(x1 - 6, y1 - 6, x1 + 6, y1 + 6, makeRgb(0, 0, 0), 192);
  hue->bitmap->rect(x1 - 5, y1 - 5, x1 + 5, y1 + 5, makeRgb(0, 0, 0), 96);
  hue->bitmap->xorRect(x1 - 4, y1 - 4, x1 + 4, y1 + 4);
  hue->bitmap->rectfill(x1 - 3, y1 - 3, x1 + 3, y1 + 3, makeRgb(r, g, b), 0);

  // saturation/value
  hue->bitmap->rect(32 - 2, 32 - 2, 95 + 2, 95 + 2, makeRgb(0, 0, 0), 192);
  hue->bitmap->rect(32 - 1, 32 - 1, 95 + 1, 95 + 1, makeRgb(0, 0, 0), 96);
  hue->bitmap->rect(0, 0, 127, 127, makeRgb(0, 0, 0), 0);

  for (int y = 0; y < 64; y++)
  {
    for (int x = 0; x < 64; x++)
    {
      Blend::hsvToRgb(h, x * 4.05, y * 4.05, &r, &g, &b);
      //Blend::wheelToRgb(h, x * 4.05, y * 4.05, &r, &g, &b);
      satval->bitmap->setpixelSolid(x, y, makeRgb(r, g, b), 0);
    }
  }

  int x = (satval->var % 64);
  int y = (satval->var / 64);

  if (x < 4)
    x = 4;
  if (y < 4)
    y = 4;
  if (x > 59)
    x = 59;
  if (y > 59)
    y = 59;

  satval->bitmap->rect(x - 6, y - 6, x + 6, y + 6, makeRgb(0, 0, 0), 192);
  satval->bitmap->rect(x - 5, y - 5, x + 5, y + 5, makeRgb(0, 0, 0), 96);
  satval->bitmap->xorRect(x - 4, y - 4, x + 4, y + 4);

  hue->redraw();
  satval->redraw();

  colorSwatch();
  colorHexUpdate();
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
  trans->redraw();
  colorSwatch();
  Project::tool->redraw(view);
}

void Gui::colorTrans(Widget *, void *)
{
//  Project::brush->trans = trans->var * 16.8;
  Project::brush->trans = trans->var * 8.23;
  char s[16];
  snprintf(s, sizeof(s), "%d", Project::brush->trans);
  trans_input->value(s);
  trans_input->redraw();
  colorSwatch();
  Project::tool->redraw(view);
//  colorChange(0, 0);
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

int Gui::getSelectAlpha()
{
  return selection_alpha->value();
}

void Gui::selectFlipX()
{
  Project::select_bmp->flipHorizontal();
}

void Gui::selectFlipY()
{
  Project::select_bmp->flipVertical();
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
}

void Gui::selectRotate180()
{
  Project::select_bmp->rotate180();
}

void Gui::selectReset()
{
  Project::tool->reset();
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

int Gui::getTextFontFace()
{
  int index = font_browse->value();

  if (index < 1)
    index = 1;

  return index - 1;
}

int Gui::getTextFontSize()
{
  return atoi(font_size->value());
}

void Gui::textChangedSize(InputInt *input, void *var)
{
  input->redraw();
}

const char *Gui::textInput()
{
  return text_input->value();
}

int Gui::getTool()
{
  return tool->var;
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

void Gui::paletteGrays()
{
  Project::palette->setGrays();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
}

void Gui::paletteTwoBits()
{
  Project::palette->setTwoBits();
  palette_swatches->var = 0;
  Project::palette->draw(palette_swatches);
}

void Gui::paletteC64()
{
  Project::palette->setC64();
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

void Gui::aspectMode()
{
  view->changeAspect(aspect->value());
}

void Gui::viewMode()
{
  view->changeViewMode(view_mode->value());
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

View *Gui::getView()
{
  return view;
}

int Gui::getClone()
{
  return clone->var;
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

int Gui::getPaintMode()
{
  return paint_mode->value();
}

int Gui::getTextSmooth()
{
  return text_smooth->value();
}

int Gui::getFillColorOnly()
{
  return fill_color_only->value();
}

int Gui::getFillFeather()
{
  return atoi(fill_feather->value());
}

int Gui::getFillRange()
{
  return atoi(fill_range->value());
}

void Gui::fillReset()
{
  fill_range->value("0");
  fill_feather->value("0");
}

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

