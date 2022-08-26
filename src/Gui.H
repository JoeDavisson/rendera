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
  void init();
  void show();
  void setMenuItem(const char *);
  void clearMenuItem(const char *);

  void checkHexColor();
  void updateHexColor();
  void updateColor(int);
  void updateSwatch();
  void updateGetColor(int);
  void setPaletteIndex(int);
  int getPaletteIndex();
  void checkPaletteSwatches(Widget *, void *);
  void drawPalette();
  void checkZoomIn(Button *, void *);
  void checkZoomOut(Button *, void *);
  void checkZoomFit(ToggleButton *, void *);
  void checkZoomOne(Button *, void *);
  void checkZoom();
  void checkGrid(ToggleButton *, void *);
  void checkGridSnap(ToggleButton *, void *);
  void checkGridX();
  void checkGridY();
  void changePaintSize(int);
  void checkPaintSize(Widget *, void *);
  void checkPaintSizeValue(Widget *, void *);
  void checkPaintShape(Widget *, void *);
  void checkPaintStroke(Widget *, void *);
  void checkPaintCoarseEdge(Widget *, void *);
  void checkPaintFineEdge(Widget *, void *);
  void checkPaintBlurryEdge(Widget *, void *);
  void checkPaintWatercolorEdge(Widget *, void *);
  void checkPaintChalkEdge(Widget *, void *);
  void checkPaintTextureEdge(Widget *, void *);
  void checkPaintTextureMarb(Widget *, void *);
  void checkPaintTextureTurb(Widget *, void *);
  void checkPaintAverageEdge(Widget *, void *);
  void checkTool(Widget *, void *);
  void checkColor(Widget *, void *);
  void checkHue(Widget *, void *);
  void checkSatVal(Widget *, void *);
  void checkTransInput(Widget *, void *);
  void checkTrans(Widget *, void *);
  void checkBlend(Widget *, void *);
  void checkWrap(Widget *, void *);
  void checkClone(Widget *, void *);
  void checkMirror(Widget *, void *);
  void checkOrigin(Widget *, void *);
  void checkConstrain(Widget *, void *);
  void checkSelectionCrop();
  void checkSelectionSelect();
  int getSelectionAlpha();
  void checkSelectionFlipHorizontal();
  void checkSelectionFlipVertical();
  void checkSelectionRotate90();
  void checkSelectionRotate180();
  void checkSelectionReset();
  void checkSelectionValues(int, int, int, int);
  void checkOffsetValues(int, int);
  void checkOffsetLeft(Widget *, void *);
  void checkOffsetRight(Widget *, void *);
  void checkOffsetUp(Widget *, void *);
  void checkOffsetDown(Widget *, void *);
  const char *getTextInput();
  int getFontFace();
  int getFontSize();
  void changedFontSize(InputInt *, void *);
  void paletteSort();
  void paletteDefault();
  void paletteBlackAndWhite();
  void paletteWebSafe();
  void palette3LevelRGB();
  void palette4LevelRGB();
  void checkClearToPaintColor();
  void checkClearToBlack();
  void checkClearToWhite();
  void checkClearToGray();
  void checkClearToTransparent();
  void checkConvertToIndexed();
  void checkConvertToGrayscale();

  Fl_Double_Window *getWindow();
  Fl_Menu_Bar *getMenuBar();
  View *getView();
  int getTool();
  int getClone();
  void checkPaintMode();
  int getPaintMode();
  int getTextSmooth();
  int getFillFeather();

  void resetTool();
  void showProgress(float);
  void showProgress(float, int);
  int updateProgress(const int);
  void hideProgress();

  void updateCoords(char *);
  void updateInfo(char *);
}

#endif

