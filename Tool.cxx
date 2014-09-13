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

#include "Tool.H"
#include "Bitmap.H"
#include "Stroke.H"
#include "Undo.H"
#include "Brush.H"

Tool *Tool::paint;
Tool *Tool::getcolor;
Tool *Tool::crop;
Tool *Tool::offset;

Tool::Tool()
{
  stroke = new Stroke();
  reset();
}

Tool::~Tool()
{
  delete stroke;
}

void Tool::reset()
{
  started = 0;
  active = 0;
}

void Tool::undo(int resized)
{
  if(resized)
  {
    stroke->x1 = Bitmap::main->cl;
    stroke->y1 = Bitmap::main->ct;
    stroke->x2 = Bitmap::main->cr;
    stroke->y2 = Bitmap::main->cb;
  }

  int x1 = stroke->x1;
  int y1 = stroke->y1;
  int x2 = stroke->x2;
  int y2 = stroke->y2;

  if(x1 < Bitmap::main->cl)
    x1 = Bitmap::main->cl;
  if(y1 < Bitmap::main->ct)
    y1 = Bitmap::main->ct;
  if(x2 > Bitmap::main->cr)
    x2 = Bitmap::main->cr;
  if(y2 > Bitmap::main->cb)
    y2 = Bitmap::main->cb;

  Undo::push(x1, y1, (x2 - x1) + 1, (y2 - y1) + 1, resized);
}

