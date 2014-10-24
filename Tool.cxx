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

#include "Bitmap.H"
#include "Brush.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"

Tool *Tool::paint;
Tool *Tool::getcolor;
Tool *Tool::crop;
Tool *Tool::offset;
Tool *Tool::text;

Tool::Tool()
{
  stroke = new Stroke();
  state = 0;
  active = false;
}

Tool::~Tool()
{
  delete stroke;
}

