/*
Copyright (c) 2014 Joe Davisson.

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

#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tooltip.H>

class Widget;
class Button;
class ToggleButton;
class InputInt;
class View;

namespace Gui
{
  void init();
  void show();
  void setMenuItem(const char *);
  void clearMenuItem(const char *);

  void updateColor(int);
  void updateGetColor(int);
  void checkPalette(Widget *, void *);
  void drawPalette();
  void checkZoomIn(Button *, void *);
  void checkZoomOut(Button *, void *);
  void checkZoomFit(ToggleButton *, void *);
  void checkZoomOne(Button *, void *);
  void checkZoom();
  void checkGrid(ToggleButton *, void *);
  void checkGridX();
  void checkGridY();
  void checkPaintSize(Widget *, void *);
  void checkPaintShape(Widget *, void *);
  void checkPaintStroke(Widget *, void *);
  void checkPaintEdge(Widget *, void *);
  void checkArtSize(Widget *, void *);
  void checkArtEdge(Widget *, void *);
  void checkTool(Widget *, void *);
  void checkColor(Widget *, void *);
  void checkHue(Widget *, void *);
  void checkSatVal(Widget *, void *);
  void checkTrans(Widget *, void *);
  void checkBlend(Widget *, void *);
  void checkWrap(Widget *, void *);
  void checkClone(Widget *, void *);
  void checkMirror(Widget *, void *);
  void checkOrigin(Widget *, void *);
  void checkConstrain(Widget *, void *);
  void checkCropDo();
  void checkCropValues(int, int, int, int);
  void checkOffsetValues(int, int);
  void textStartOver();
  const char *getTextInput();
  int getFontFace();
  int getFontSize();
  void paletteSort();
  void paletteSort();
  void paletteDefault();
  void paletteBlackAndWhite();
  void paletteWebSafe();
  void palette3LevelRGB();
  void palette4LevelRGB();
  void palette332();
  void checkClear();

  View *getView();
  int getTool();
  int getClone();
  void checkPaintMode();
  int getPaintMode();
  int getTextSmooth();
}

#endif

