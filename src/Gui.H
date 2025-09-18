/*
Copyright (c) 2025 Joe Davisson.

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

#ifndef GUI_H
#define GUI_H

class ColorOptions;
class FillOptions;
class ImagesOptions;
class OffsetOptions;
class PaintOptions;
class PickerOptions;
class SelectionOptions;
class TextOptions;
class ToolOptions;
class ViewOptions;

class Group;
class View;

class Fl_Box;
class Fl_Double_Window;
class Fl_Group;
class Fl_Menu_Bar;
class Fl_Progress;

class Gui
{
public:
  enum
  {
    SPACING = 15,
    TOP_HEIGHT = 56,
    TOOLS_WIDTH = 64,
    OPTIONS_WIDTH = 176,
    COLORS_WIDTH = 208,
    IMAGES_WIDTH = 176,
    STATUS_HEIGHT = 32,
    TOTAL_WIDTH = (TOOLS_WIDTH + OPTIONS_WIDTH + COLORS_WIDTH + IMAGES_WIDTH)
  };

  static void closeFile();
  static Fl_Menu_Bar *getMenuBar();
  static View *getView();
  static Fl_Double_Window *getWindow();
  static Fl_Group *getStatus();

  static void init();
  static void show();

  static void clearToBlack();
  static void clearToColor();
  static void clearToGray();
  static void clearToTrans();
  static void clearToWhite();

  static void duplicate();

  static void menuCheckItem(const char *);
  static void menuClearItem(const char *);

  static void paletteSortByValue();
  static void paletteSortByHue();
  static void paletteNormalize();
  static void paletteSetDefault();
  static void paletteSetBlackAndWhite();
  static void paletteSetGrays();
  static void paletteSetTwoBits();
  static void paletteSetC64();
  static void paletteSetVCS();
  static void paletteSetWebSafe();
  static void paletteSet3LevelRGB();
  static void paletteSet4LevelRGB();
  static void paletteSet332();

  static void mouseTimer();

  static void selectFromImage();
  static void selectToImage();

  static void statusCoords(char *);
  static void statusInfo(char *);

  static void toggleTextPreview();

  static void updateMemInfo();

  static View *view;

  static Fl_Box *coords;
  static Fl_Box *info;

  static Fl_Progress *progress;

  static ViewOptions *top;
  static PaintOptions *paint;
  static PickerOptions *picker;
  static ImagesOptions *images;
  static OffsetOptions *offset;
  static SelectionOptions *selection;
  static TextOptions *text;
  static FillOptions *fill;
  static ToolOptions *tools;
  static ColorOptions *colors;

private:
  Gui() { }
  ~Gui() { }
};

#endif

