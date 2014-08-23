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

Dialog *dialog;
Gui *gui;

int main(int argc, char **argv)
{
  Fl::visual(FL_DOUBLE | FL_RGB);
  Fl::background(192, 192, 192);
  Fl_Shared_Image::add_handler(File::previewJPG);
  Fl_Shared_Image::add_handler(File::previewPNG);
  Fl_Shared_Image::add_handler(File::previewBMP);
  Fl_Shared_Image::add_handler(File::previewTGA);
  Fl_Shared_Image::add_handler(File::previewGPL);
  fl_message_hotspot(0);
//  Fl_File_Icon::load_system_icons();

  Bitmap::main = new Bitmap(640, 480, 64,
                            makecol(255, 255, 255), makecol(128, 128, 128));
  Bitmap::preview = new Bitmap(8, 8);

  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);
  Map::main->clear(0);
  Brush::main = new Brush();
  Palette::main = new Palette();
  Palette::undo = new Palette();
  Bitmap::clone_buffer = new Bitmap(8, 8);
  Bitmap::offset_buffer = new Bitmap(8, 8);
  Blend::bmp = Bitmap::main;

  dialog = new Dialog();

  Tool::paint = new Paint();
  Tool::getcolor = new GetColor();
  Tool::crop = new Crop();
  Tool::offset = new Offset();

  gui = new Gui();

  // initialize some things
  Palette::main->draw(gui->palette);
  gui->tool->do_callback();
  gui->palette->do_callback();
  check_zoom();

  FX::init();
  undo_init();

  return Fl::run();
}

