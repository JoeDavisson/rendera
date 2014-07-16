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
  Fl_Shared_Image::add_handler(preview_jpg);
  fl_message_hotspot(0);
//  Fl_File_Icon::load_system_icons();

  int overscroll = 64;
  Bitmap::overscroll = overscroll;

  Bitmap::main = new Bitmap(640 + overscroll * 2, 480 +  + overscroll * 2);
  Bitmap::main->clear(makecol(128, 128, 128));
  Bitmap::main->set_clip(overscroll, overscroll, Bitmap::main->w - overscroll - 1, Bitmap::main->h - overscroll - 1);
  Bitmap::main->rectfill(overscroll, overscroll, Bitmap::main->w - overscroll - 1, Bitmap::main->h - overscroll - 1, makecol(255, 255, 255), 0);
  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);
  Map::main->clear(0);
  Brush::main = new Brush();
  Palette::main = new Palette();
  Bitmap::clone_buffer = new Bitmap(8, 8);
  Bitmap::offset_buffer = new Bitmap(8, 8);

  dialog = new Dialog();

  Tool::paint = new Paint();
  Tool::airbrush = new Airbrush();
  Tool::crop = new Crop();
  Tool::getcolor = new GetColor();
  Tool::offset = new Offset();

  gui = new Gui();

  // initialize some things
//  gui->hue->var = 95 + 96 * 48;
//  gui->sat->var = 95;
//  gui->val->var = 95;
//  gui->hue->do_callback();
  Palette::main->draw(gui->palette);
  gui->palette->do_callback();
  check_zoom();

  return Fl::run();
}

