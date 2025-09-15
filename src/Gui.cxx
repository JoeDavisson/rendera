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
#include "ColorOptions.H"
#include "Dialog.H"
#include "Editor.H"
#include "ExportData.H"
#include "FX/FX.H"
#include "File.H"
#include "FillOptions.H"
#include "Gamma.H"
#include "GetColorOptions.H"
#include "Group.H"
#include "Gui.H"
#include "Images.H"
#include "ImagesOptions.H"
#include "Inline.H"
#include "InputInt.H"
#include "InputText.H"
#include "Map.H"
#include "OffsetOptions.H"
#include "Palette.H"
#include "PaintOptions.H"
#include "Project.H"
#include "Render.H"
#include "RepeatButton.H"
#include "Separator.H"
#include "Selection.H"
#include "SelectionOptions.H"
#include "StaticText.H"
#include "Stroke.H"
#include "Text.H"
#include "TextOptions.H"
#include "ToggleButton.H"
#include "Tool.H"
#include "ToolsOptions.H"
#include "Transform.H"
#include "Undo.H"
#include "View.H"
#include "ViewOptions.H"
#include "Widget.H"

class MainWin;

// view
View *Gui::view;
Fl_Box *Gui::coords;
Fl_Box *Gui::info;

// progress
Fl_Progress *Gui::progress;

// groups
ViewOptions *Gui::top;
PaintOptions *Gui::paint;
GetColorOptions *Gui::getcolor;
ImagesOptions *Gui::images;
OffsetOptions *Gui::offset;
SelectionOptions *Gui::selection;
TextOptions *Gui::text;
FillOptions *Gui::fill;
ToolsOptions *Gui::tools;
ColorOptions *Gui::colors;

namespace
{
  // window
  MainWin *window;

  // main menu
  Fl_Menu_Bar *menubar;

  // containers
  Fl_Group *left;
  Fl_Group *middle;
  Fl_Group *right;

  // status
  Group *status;

  // height of rightmost panels
  int left_height = 0;
  int right_height = 0;

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
            Gui::closeFile();
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
              Gui::selection->selectCopy();
            return 1;
          case 'v':
            if (ctrl)
              Gui::selection->selectPaste();
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
    (Fl_Callback *)closeFile, 0, FL_MENU_DIVIDER);
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
    (Fl_Callback *)duplicate, 0, FL_MENU_DIVIDER);
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
  menubar->add("&Palette/&Sort/Hue", 0,
    (Fl_Callback *)paletteSortByHue, 0, 0);
  menubar->add("&Palette/&Sort/Value", 0,
    (Fl_Callback *)paletteSortByValue, 0, 0);
  menubar->add("&Palette/&Normalize", 0,
    (Fl_Callback *)paletteNormalize, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Editor... (E)", 0,
    (Fl_Callback *)Editor::begin, 0, FL_MENU_DIVIDER);
  menubar->add("&Palette/&Undo", 0,
    (Fl_Callback *)Editor::pop, 0, 0);
  menubar->add("&Palette/&Redo", 0,
    (Fl_Callback *)Editor::popRedo, 0, 0);

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
  top = new ViewOptions(0, menubar->h(), window->w(), TOP_HEIGHT, "");

  left_height = window->h() - top->h() - menubar->h() - status->h();

  // tools
  tools = new ToolsOptions(0, top->h() + menubar->h(),
                           64, left_height,
                           "Tools");

  // paint
  paint = new PaintOptions(TOOLS_WIDTH, top->h() + menubar->h(),
                    OPTIONS_WIDTH, left_height,
                    "Paint");

  // selection
  selection = new SelectionOptions(TOOLS_WIDTH, top->h() + menubar->h(),
                                   OPTIONS_WIDTH, left_height,
                                  "Selection");

  // getcolor
  getcolor = new GetColorOptions(TOOLS_WIDTH, top->h() + menubar->h(),
                                 OPTIONS_WIDTH, left_height,
                                 "Get Color");

  // offset
  offset = new OffsetOptions(TOOLS_WIDTH, top->h() + menubar->h(),
                             OPTIONS_WIDTH, left_height,
                             "Offset");

  // text
  text = new TextOptions(TOOLS_WIDTH, top->h() + menubar->h(),
                         OPTIONS_WIDTH, left_height,
                         "Text");

  // fill
  fill = new FillOptions(TOOLS_WIDTH, top->h() + menubar->h(),
                         OPTIONS_WIDTH, left_height,
                         "Fill");

  // colors
  colors = new ColorOptions(window->w() - COLORS_WIDTH - IMAGES_WIDTH,
                            top->h() + menubar->h(),
                            COLORS_WIDTH,
                            window->h() - top->h() - menubar->h() - status->h(),
                           "Colors");

  right_height = left_height;

  // images
  images = new ImagesOptions(window->w() - IMAGES_WIDTH,
                         top->h() + menubar->h(),
                         IMAGES_WIDTH,
                         window->h() - top->h() - menubar->h() - status->h(),
                        "Images");

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
  right = new Fl_Group(window->w() - COLORS_WIDTH - IMAGES_WIDTH,
                       top->h() + menubar->h(),
                       COLORS_WIDTH + IMAGES_WIDTH,
                       right_height);

  right->add(images);
  right->add(colors);

  // resize these panels
  colors->resize(colors->x(), colors->y(), colors->w(), right_height);
  images->resize(images->x(), images->y(), images->w(), right_height);

  window->size_range(1024, 828, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();

  // misc init
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

  paletteSetDefault();
  tools->init();
  top->zoomLevel();
  images->imagesAddFile("new");
  colors->colorUpdate(Project::palette->data[0]);
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

// callback functions

// sort into grays, low-sat colors, hi-sat colors
void Gui::paletteSortByHue()
{
  Editor::push();
  Project::palette->sortByHue();
  colors->changePalette(Project::palette);
}

void Gui::paletteSortByValue()
{
  Editor::push();
  Project::palette->sortByValue();
  colors->changePalette(Project::palette);
}

void Gui::paletteNormalize()
{
  Editor::push();
  Project::palette->normalize();
  colors->changePalette(Project::palette);
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
  colors->changePalette(pal);
}

void Gui::paletteSetBlackAndWhite()
{
  Palette *pal = Project::palette;

  pal->data[0] = makeRgb(0, 0, 0);
  pal->data[1] = makeRgb(255, 255, 255);

  pal->max = 2;
  pal->fillTable();
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
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
  colors->changePalette(pal);
}

// limit mouse framerate
void Gui::mouseTimer()
{
  view->mouse_timer_ready = true;
  Fl::repeat_timeout(1.0 / 125, (Fl_Timeout_Handler)Gui::mouseTimer);
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

// joe
//  tool->var = Tool::SELECT;
//  toolChange(tool, (void *)&tool->var);
}

void Gui::selectToImage()
{
  Bitmap *select_bmp = Project::select_bmp;
  Bitmap *temp = new Bitmap(select_bmp->w, select_bmp->h);
  
  select_bmp->blit(temp, 0, 0, 0, 0, temp->w, temp->h);

  if (Project::newImageFromBitmap(temp) != -1)
    images->imagesAddFile("new_from_selection");
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

void Gui::closeFile()
{
  images->imagesCloseFile();
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

Fl_Double_Window *Gui::getWindow()
{
  return window;
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

void Gui::updateMemInfo()
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

  images->imagesMemLabel(s);

  Fl::repeat_timeout(1.0, (Fl_Timeout_Handler)Gui::updateMemInfo);
}

void Gui::duplicate()
{
  images->imagesDuplicate();
}

