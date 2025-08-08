/*
Copyright (c) 2024 Joe Davisson.

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
class ToggleButton;
class InputInt;
class View;
class Fl_Double_Window;
class Fl_Menu_Bar;

class Gui
{
public:
  static int getClone();
  static int getFillColorOnly();
  static int getFillFeather();
  static int getFillRange();
  static int getPaintMode();
  static int getPaletteIndex();
  static int getSelectAlpha();
  static int getTextFontFace();
  static int getTextFontSize();
  static int getTextFontAngle();
  static int getTextSmooth();
  static int getTool();

  static Fl_Menu_Bar *getMenuBar();
  static View *getView();
  static Fl_Double_Window *getWindow();

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
  static void palette3LevelRGB();
  static void palette4LevelRGB();
  static void palette332();
  static void paletteBlackAndWhite();
  static void paletteGrays();
  static void paletteTwoBits();
  static void paletteC64();
  static void paletteVCS();
  static void paletteDefault();
  static void paletteDraw();
  static void paletteIndex(int);
  static void paletteNormalize();
  static void paletteSwatches(Widget *, void *);
  static void paletteWebSafe();

  static int progressUpdate(int);
  static void progressEnable(bool);
  static void progressHide();
  static void progressShow(float);
  static void progressShow(float, int);

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
  static const char *textInput();

  static void toolChange(Widget *, void *);

  static void zoomIn(Button *, void *);
  static void zoomOut(Button *, void *);
  static void zoomOne(Button *, void *);
  static void zoomLevel();

private:
  Gui() { }
  ~Gui() { }
};

#endif

