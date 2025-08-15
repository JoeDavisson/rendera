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

#ifndef TEXT_H
#define TEXT_H

class Bitmap;
class InputInt;
class View;

#include "Tool.H"

class Text : public Tool
{
public:
  Text();
  ~Text();

  void push(View *);
  void drag(View *);
  void release(View *);
  void move(View *);
  void key(View *);
  void done(View *, int);
  void redraw(View *);
  bool isActive();
  void reset();

//  static int getTextFontFace();
//  static int getTextFoneSize();
//  static int getTextFontAngle();

  static void textChangedSize(InputInt *, void *);

private:
  Bitmap *textbmp;
};

#endif

