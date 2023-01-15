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

  int fillGetFeather();
  int fillGetRange();
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

  void menuClearItem(const char *);
  void menuSetItem(const char *);

  void offsetValues(int, int);
  void offsetLeft(Widget *, void *);
  void offsetRight(Widget *, void *);
  void offsetUp(Widget *, void *);
  void offsetDown(Widget *, void *);

  void originEnable(Widget *, void *);

  void paintChangeSize(int);
  void paintMode();
  int paintGetMode();
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
  void paletteDraw();
  int paletteGetIndex();
  void paletteSetIndex(int);
  void paletteSort();
  void paletteSwatches(Widget *, void *);
  void paletteWebSafe();


  int progressUpdate(int);
  void progressEnable(bool);
  void progressHide();
  void progressShow(float);
  void progressShow(float, int);

  int selectGetAlpha();
  void selectCreate();
  void selectCrop();
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
  int textGetFontFace();
  const char *textInput();
  int textGetFontSize();
  int textGetSmooth();

  void toolChange(Widget *, void *);
  int toolGet();

  void zoomIn(Button *, void *);
  void zoomOut(Button *, void *);
  void zoomOne(Button *, void *);
  void zoomLevel();
}
/*
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

  void checkClone(Widget *, void *);
  void checkOrigin(Widget *, void *);
  void checkConstrain(Widget *, void *);

  void checkSelectionCreate();
  void checkSelectionCrop();
  int getSelectionAlpha();
  void checkSelectionFlipHorizontal();
  void checkSelectionFlipVertical();
  void checkSelectionRotate90();
  void checkSelectionRotate180();
  void checkSelectionReset();
  void checkSelectionValues(int, int, int, int);
  void checkImageToSelection();
  void checkSelectionToNewImage();
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

  void checkFileBrowse();
  void checkFileRename();
  void addFile(const char *);
  void closeFile();
  void updateMemInfo();

  Fl_Double_Window *getWindow();
  Fl_Menu_Bar *getMenuBar();
  View *getView();
  int getTool();
  int getClone();

  void checkPaintMode();
  int getPaintMode();
  int getTextSmooth();
  int getFillRange();
  int getFillFeather();
  void checkFillReset();
  void duplicateImage();

  void showProgress(float);
  void showProgress(float, int);
  int updateProgress(int);
  void hideProgress();
  void enableProgress(bool);

  void updateCoords(char *);
  void updateInfo(char *);
}
*/

#endif

