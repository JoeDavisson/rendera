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

int main(int argc, char **argv)
{
  Fl::visual(FL_DOUBLE | FL_RGB);
  Fl::background(192, 192, 192);

  Bitmap::main = new Bitmap(1024 + 64, 1024 + 64);
  Bitmap::main->clear(makecol(0, 0, 0));
  Bitmap::main->set_clip(32, 32, Bitmap::main->w - 32 - 1, Bitmap::main->h - 32 - 1);
  Bitmap::main->rectfill(32, 32, Bitmap::main->w - 32 - 1, Bitmap::main->h - 32 - 1, makecol(255, 255, 255), 0);
  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);
  Brush::main = new Brush();
  Palette::main = new Palette();

  gui = new Gui();

  // initialize some things
  gui->hue->var = 95 + 96 * 48;
  gui->sat->var = 95;
  gui->val->var = 95;
  gui->hue->do_callback();
  Palette::main->draw(gui->palette);

  //Fl_File_Chooser *load_dialog = new Fl_File_Chooser(".", "Image Files (*.{bmp,gif,jpg,png})", 0, "Load Image");

  //load_dialog->show();

  return Fl::run();
}

