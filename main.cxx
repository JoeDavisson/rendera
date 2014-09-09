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

#include <cmath>

#include "rendera.h"
#include "Bitmap.H"
#include "Map.H"
#include "Palette.H"
#include "Blend.H"
#include "Brush.H"
#include "FX.H"
#include "Undo.H"
#include "Dialog.H"
#include "Gui.H"
#include "Tool.H"
#include "Paint.H"
#include "GetColor.H"
#include "Crop.H"
#include "Offset.H"
#include "File.H"
#include "Widget.H"

// for fast random number generator in inline.h
int seed = 12345;

int *fix_gamma;
int *unfix_gamma;

int main(int /* argc */, char** /* argv */)
{
  Fl::visual(FL_DOUBLE | FL_RGB);
  Fl::background(192, 192, 192);
  Fl_Shared_Image::add_handler(File::previewJPG);
  Fl_Shared_Image::add_handler(File::previewPNG);
  Fl_Shared_Image::add_handler(File::previewBMP);
  Fl_Shared_Image::add_handler(File::previewTGA);
  Fl_Shared_Image::add_handler(File::previewGPL);
  fl_message_hotspot(0);

  // gamma correction tables
  fix_gamma = new int[256];
  unfix_gamma = new int[65536];

  int i;
  for(i = 0; i < 65536; i++)
    unfix_gamma[i] = std::pow((double)i / 65535, (1.0 / 2.2)) * 255;
  for(i = 0; i < 256; i++)
    fix_gamma[i] = std::pow((double)i / 255, 2.2) * 65535;

  Bitmap::main = new Bitmap(640, 480, 64,
                            make_rgb(255, 255, 255), make_rgb(128, 128, 128));
  Bitmap::preview = new Bitmap(8, 8);

  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);
  Map::main->clear(0);
  Brush::main = new Brush();
  Palette::main = new Palette();
  Palette::undo = new Palette();
  Bitmap::clone_buffer = new Bitmap(8, 8);
  Bitmap::offset_buffer = new Bitmap(8, 8);

  Tool::paint = new Paint();
  Tool::getcolor = new GetColor();
  Tool::crop = new Crop();
  Tool::offset = new Offset();

  FX::init();
  Undo::init();
  Dialog::init();
  Gui::init();

  return Fl::run();
}

