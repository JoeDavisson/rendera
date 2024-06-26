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

#ifndef UNDO_H
#define UNDO_H

#include "Bitmap.H"

class Bitmap;

class Undo
{
public:
  enum
  {
    PARTIAL,
    FULL,
    OFFSET,
    FLIP_HORIZONTAL,
    FLIP_VERTICAL,
    ROTATE_90,
    ROTATE_180
  };

  Undo();
  ~Undo();

  void reset();
  void doPush(const int, const int, const int, const int, const int);
  void push();
  void push(const int);
  void push(const int, const int, const int, const int, const int);
  void pop();
  void pushRedo(const int, const int, const int, const int, const int);
  void popRedo();

  int levels = 16;
  int undo_current = levels - 1;
  int redo_current = levels - 1;

  Bitmap **undo_stack;
  Bitmap **redo_stack;

private:
  void printStacks();
};

#endif

