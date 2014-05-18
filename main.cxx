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
#include <iostream>

Var *var;
Bmp *bmp;
Brush *brush;
Blend *blend;
Stroke *stroke;
Gui *gui;

int main(int argc, char **argv)
{
  Fl::visual(FL_DOUBLE | FL_INDEX);

  var = new Var();
  bmp = new Bmp();
  blend = new Blend();
  stroke = new Stroke();
  gui = new Gui();

  return Fl::run();
}

