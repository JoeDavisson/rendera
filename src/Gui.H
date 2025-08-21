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

class Widget;
class Button;
class CheckBox;
class ToggleButton;
class InputInt;
class InputText;
class RepeatButton;
class StaticText;
class View;

class Fl_Box;
class Fl_Button;
class Fl_Choice;
class Fl_Double_Window;
class Fl_Group;
class Fl_Hold_Browser;
class Fl_Input;
class Fl_Menu_Bar;
class Fl_Progress;

class Gui
{
public:
  static Fl_Menu_Bar *getMenuBar();
  static View *getView();
  static Fl_Double_Window *getWindow();
  static Fl_Group *getStatus();

  static void init();
  static void show();

  static void aspectMode();

  static void clearToBlack();
  static void clearToColor();
  static void clearToGray();
  static void clearToTrans();
  static void clearToWhite();

  static void constrainEnable(Widget *, void *);
  static void cloneEnable(Widget *, void *);

  static void colorBlend(Widget *, void *);
  static void colorChange(Widget *, void *);
  static void colorHexInput();
  static void colorHexUpdate();
  static void colorHue(Widget *, void *);
  static void colorSatVal(Widget *, void *);
  static void colorTrans();
  static void colorTransInput(Widget *, void *);
  static void colorUpdate(int);
  static void transUpdate(int);

  static void fillReset();
  static void filterToggle();

  static void getcolorUpdate(int);

  static void gridEnable(ToggleButton *, void *);
  static void gridSnap(ToggleButton *, void *);
  static void gridX();
  static void gridY();

  static void imagesBrowse();
  static void imagesRename();
  static void imagesAddFile(const char *);
  static void imagesCloseFile();
  static void imagesUpdateMemInfo();
  static void imagesDuplicate();
  static void imagesMoveUp();
  static void imagesMoveDown();

  static void menuCheckItem(const char *);
  static void menuClearItem(const char *);

  static void offsetValues(int, int);
  static void offsetLeft(Widget *, void *);
  static void offsetRight(Widget *, void *);
  static void offsetUp(Widget *, void *);
  static void offsetDown(Widget *, void *);

  static void originEnable(Widget *, void *);

  static void paintChangeSize(int);
  static void paintMode();
  static void paintSize(Widget *, void *);
  static void paintSizeValue(Widget *, void *);
  static void paintShape(Widget *, void *);
  static void paintStroke(Widget *, void *);
  static void paintCoarseEdge(Widget *, void *);
  static void paintFineEdge(Widget *, void *);
  static void paintBlurryEdge(Widget *, void *);
  static void paintWatercolorEdge(Widget *, void *);
  static void paintChalkEdge(Widget *, void *);
  static void paintTextureEdge(Widget *, void *);
  static void paintTextureMarb(Widget *, void *);
  static void paintTextureTurb(Widget *, void *);
  static void paintAverageEdge(Widget *, void *);

  static void paletteSortValue();
  static void paletteSortHue();
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

  static void paletteDraw();
  static void paletteIndex(int);
  static void paletteSwatches(Widget *, void *);

/*
  static int progressUpdate(int);
  static void progressEnable(bool);
  static void progressHide();
  static void progressShow(float);
  static void progressShow(float, int);
*/

  static void mouseTimer();

  static void selectAlpha();
  static void selectCopy();
  static void selectCopyEnable(bool);
  static void selectPaste();
  static void selectPasteEnable(bool);
  static void selectCrop();
  static void selectCropEnable(bool);
  static void selectFlipX();
  static void selectFlipY();
  static void selectFromImage();
  static void selectReset();
  static void selectRotate180();
  static void selectRotate90();
  static void selectToImage();
  static void selectValues(int, int, int, int);

  static void statusCoords(char *);
  static void statusInfo(char *);

  static void textChangedSize(InputInt *, void *);
  static void toolChange(Widget *, void *);

  static void zoomIn(Button *, void *);
  static void zoomOut(Button *, void *);
  static void zoomOne(Button *, void *);
  static void zoomLevel();

  // top
  static Button *zoom_one;
  static Button *zoom_in;
  static Button *zoom_out;
  static StaticText *zoom;
  static ToggleButton *grid;
  static ToggleButton *gridsnap;
  static InputInt *gridx;
  static InputInt *gridy;
  static Fl_Choice *aspect;
  static CheckBox *filter;

  // tools
  static Widget *tool;
  static ToggleButton *clone;
  static ToggleButton *origin;
  static ToggleButton *constrain;

  // options
  static Widget *paint_brush_preview;
  static Widget *paint_size;
  static InputInt *paint_size_value;
  static Widget *paint_stroke;
  static Widget *paint_shape;
  static Widget *paint_coarse_edge;
  static Widget *paint_fine_edge;
  static Widget *paint_blurry_edge;
  static Widget *paint_watercolor_edge;
  static Widget *paint_chalk_edge;
  static Widget *paint_texture_edge;
  static Widget *paint_texture_marb;
  static Widget *paint_texture_turb;
  static Widget *paint_average_edge;
  static Fl_Choice *paint_mode;

  static Widget *getcolor_color;
  static CheckBox *getcolor_best;

  static InputInt *fill_range;
  static InputInt *fill_feather;
  static CheckBox *fill_color_only;
  static Fl_Button *fill_reset;

  static StaticText *selection_x;
  static StaticText *selection_y;
  static StaticText *selection_w;
  static StaticText *selection_h;
  static Fl_Button *selection_reset;
  static Fl_Button *selection_copy;
  static CheckBox *selection_alpha;
  static Button *selection_flip;
  static Button *selection_mirror;
  static Button *selection_rotate;
  static Fl_Button *selection_paste;
  static Fl_Button *selection_crop;

  static StaticText *offset_x;
  static StaticText *offset_y;
  static RepeatButton *offset_up;
  static RepeatButton *offset_left;
  static RepeatButton *offset_right;
  static RepeatButton *offset_down;

  static Fl_Hold_Browser *font_browse;
  static InputInt *font_size;
  static InputInt *font_angle;
  static Fl_Input *text_input;
  static CheckBox *text_smooth;

  // colors
  static Widget *hue;
  static Widget *satval;
  static InputText *hexcolor;
  static InputInt *trans_input;
  static Widget *trans;
  static Fl_Choice *blend;
  static Widget *palette_swatches;

  // files
  static Fl_Hold_Browser *file_browse;
  static Fl_Input *file_rename;
  static Button *file_close;
  static Button *file_move_up;
  static Button *file_move_down;
  static Fl_Box *file_mem;

  // view
  static View *view;

  // status
  static Fl_Box *coords;
  static Fl_Box *info;

  // progress
  static Fl_Progress *progress;

private:
  Gui() { }
  ~Gui() { }
};

#endif

