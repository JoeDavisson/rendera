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

#include "rendera.h"

Tool *Tool::paint;
Tool *Tool::airbrush;
Tool *Tool::pixelart;
Tool *Tool::stump;
Tool *Tool::crop;
Tool *Tool::getcolor;
Tool *Tool::offset;

Tool *Tool::current;

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
  undo_push(stroke->x1,
            stroke->y1,
            stroke->x2 - stroke->x1 + 1,
            stroke->y2 - stroke->y1 + 1, resized);
}

