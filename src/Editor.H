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

#ifndef EDITOR_H
#define EDITOR_H

class Editor
{
public:
  static void begin();
  static void init();

  static void resetUndo();
  static void doPush();
  static void push();
  static void pop();
  static void pushRedo();
  static void popRedo();

private:
  static void updateInfo(char *);
  static void updateIndex(int);
  static void setHsvSliders();
  static void setHsv();
  static void updateHexColor();
  static void checkHexColor();
  static void checkHexColorWeb();
  static void insertColor();
  static void removeColor();
  static void checkReplaceColor(int);
  static void replaceColor();
  static void copyColor(int, int);
  static void swapColor(int, int);
  static void checkRampRgb(int);
  static void checkRampHsv(int);
  static void checkPalette();
  static void getHue();
  static void getSatVal();
  static void rgbRamp();
  static void hsvRamp();
  static void close();

  Editor() { }
  ~Editor() { }
};

#endif

