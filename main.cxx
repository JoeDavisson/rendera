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

Gui *gui;

//static int x[4] = { 20, 10, 60, 50 };
//static int y[4] = { 10, 20, 30, 40 };
static int x[4] = { 10, 20, 50, 60 };
static int y[4] = { 20, 10, 40, 30 };

int main(int argc, char **argv)
{
  Fl::visual(FL_DOUBLE | FL_INDEX);

  Bmp::main = new Bitmap(512, 512);
  Bmp::main->clear(makecol(255, 255, 255));
  Bmp::main->quad(x, y, makecol(0, 0, 0), 0);
  Bmp::preview = new Bitmap(8, 8);

  gui = new Gui();

  return Fl::run();
}

