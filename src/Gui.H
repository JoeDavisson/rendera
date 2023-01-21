/*
Copyright (c) 2021 Joe Davisson.

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

namespace Gui
{
  int getClone();
  int getFillFeather();
  int getFillRange();
  int getPaintMode();
  int getPaletteIndex();
  int getSelectAlpha();
  int getTextFontFace();
  int getTextFontSize();
  int getTextSmooth();
  int getTool();

  Fl_Menu_Bar *getMenuBar();
  View *getView();
  Fl_Double_Window *getWindow();

  void init();
  void show();

  void clearToBlack();
  void clearToColor();
  void clearToGray();
  void clearToTrans();
  void clearToWhite();

  void constrainEnable(Widget *, void *);
  void cloneEnable(Widget *, void *);

  void colorBlend(Widget *, void *);
  void colorChange(Widget *, void *);
  void colorHexInput();
  void colorHexUpdate();
  void colorHue(Widget *, void *);
  void colorSatVal(Widget *, void *);
  void colorSwatch();
  void colorTrans(Widget *, void *);
  void colorTransInput(Widget *, void *);
  void colorUpdate(int);

  void fillReset();

  void getcolorUpdate(int);

  void gridEnable(ToggleButton *, void *);
  void gridSnap(ToggleButton *, void *);
  void gridX();
  void gridY();

  void imagesBrowse();
  void imagesRename();
  void imagesAddFile(const char *);
  void imagesCloseFile();
  void imagesUpdateMemInfo();
  void imagesDuplicate();

  void menuCheckItem(const char *);
  void menuClearItem(const char *);

  void offsetValues(int, int);
  void offsetLeft(Widget *, void *);
  void offsetRight(Widget *, void *);
  void offsetUp(Widget *, void *);
  void offsetDown(Widget *, void *);

  void originEnable(Widget *, void *);

  void paintChangeSize(int);
  void paintMode();
  void paintSize(Widget *, void *);
  void paintSizeValue(Widget *, void *);
  void paintShape(Widget *, void *);
  void paintStroke(Widget *, void *);
  void paintCoarseEdge(Widget *, void *);
  void paintFineEdge(Widget *, void *);
  void paintBlurryEdge(Widget *, void *);
  void paintWatercolorEdge(Widget *, void *);
  void paintChalkEdge(Widget *, void *);
  void paintTextureEdge(Widget *, void *);
  void paintTextureMarb(Widget *, void *);
  void paintTextureTurb(Widget *, void *);
  void paintAverageEdge(Widget *, void *);

  void palette3LevelRGB();
  void palette4LevelRGB();
  void paletteBlackAndWhite();
  void paletteDefault();
  void paletteCMYK();
  void paletteDraw();
  void paletteIndex(int);
  void paletteSort();
  void paletteSwatches(Widget *, void *);
  void paletteWebSafe();


  int progressUpdate(int);
  void progressEnable(bool);
  void progressHide();
  void progressShow(float);
  void progressShow(float, int);

  void selectCopy();
  void selectCopyEnable(bool);
  void selectPaste();
  void selectPasteEnable(bool);
  void selectCrop();
  void selectCropEnable(bool);
  void selectFlipX();
  void selectFlipY();
  void selectFromImage();
  void selectReset();
  void selectRotate180();
  void selectRotate90();
  void selectToImage();
  void selectValues(int, int, int, int);

  void statusCoords(char *);
  void statusInfo(char *);

  void textChangedSize(InputInt *, void *);
  const char *textInput();

  void toolChange(Widget *, void *);

  void zoomIn(Button *, void *);
  void zoomOut(Button *, void *);
  void zoomOne(Button *, void *);
  void zoomLevel();
}

#endif

