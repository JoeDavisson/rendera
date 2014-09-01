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

#ifndef TOOL_H
#define TOOL_H

class Stroke;
class View;

#include "rendera.h"

// extend this class for painting/interactive tools
class Tool
{
public:
  Tool();
  virtual ~Tool();

  // reset tool
  void reset();

  // store undo
  void undo(int);

  // called when mouse button is initially pressed
  virtual void push(View *) = 0;

  // called when mouse is dragged
  virtual void drag(View *) = 0;

  // called when mouse button is released
  virtual void release(View *) = 0;

  // called when mouse is moved, but tool is not completed yet
  // in the case of an tool that requires multiple steps (such as crop)
  virtual void move(View *) = 0;

  // call this to finish tool from external trigger
  virtual void done(View *) = 0;

  // call this to redraw tool
  virtual void redraw(View *) = 0;

  // vars
  Stroke *stroke;
  int beginx, beginy, lastx, lasty;
  int started;
  int active;
  int amount;

  // extended tools
  static Tool *paint;
  static Tool *getcolor;
  static Tool *crop;
  static Tool *offset;
};

#endif

