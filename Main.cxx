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
#include "Blend.H"
#include "Brush.H"
#include "Crop.H"
#include "Dialog.H"
#include "FX.H"
#include "File.H"
#include "GetColor.H"
#include "Gui.H"
#include "Map.H"
#include "Offset.H"
#include "Paint.H"
#include "Palette.H"
#include "Project.H"
#include "Rendera.H"
#include "Text.H"
#include "Tool.H"
#include "Transform.H"
#include "Undo.H"
#include "Widget.H"

namespace
{
  void setLightTheme()
  {
    Fl::set_color(FL_BACKGROUND_COLOR, 224, 224, 224);
    Fl::set_color(FL_BACKGROUND2_COLOR, 192, 192, 192);
    Fl::set_color(FL_FOREGROUND_COLOR, 0, 0, 0);
    Fl::set_color(FL_INACTIVE_COLOR, 128, 128, 128);
    Fl::set_color(FL_SELECTION_COLOR, 64, 64, 64);
    Project::theme = Project::THEME_LIGHT;
  }

  void setDarkTheme()
  {
    Fl::set_color(FL_BACKGROUND_COLOR, 80, 80, 80);
    Fl::set_color(FL_BACKGROUND2_COLOR, 64, 64, 64);
    Fl::set_color(FL_FOREGROUND_COLOR, 248, 248, 248);
    Fl::set_color(FL_INACTIVE_COLOR, 128, 128, 128);
    Fl::set_color(FL_SELECTION_COLOR, 248, 248, 248);
    Project::theme = Project::THEME_DARK;
  }
}

int main(int argc, char *argv[])
{
  // default to dark theme
  setDarkTheme();

  int count = argc;

  if(argc > 1)
  {
    int i;

    for(i = 1; i < argc; i++)
    {
      if(strcmp(argv[i], "--use-light-theme") == 0)
      {
        setLightTheme();
        count--;
      }

      if(strcmp(argv[i], "--use-dark-theme") == 0)
      {
        setDarkTheme();
        count--;
      }

      if(strcmp(argv[i], "--help") == 0)
      {
        printf("\n");
        printf("Usage: %s <options> <filename>\n", argv[0]);
        printf("\n");
        printf("Theme Settings:\n");
        printf("  --use-light-theme\n");
        printf("  --use-dark-theme\n");
        printf("\n");
        return 0;
      }
    }
  }

  Fl::visual(FL_DOUBLE | FL_RGB);

  Fl::scheme("gtk+");
  Fl_Shared_Image::add_handler(File::previewJPG);
  Fl_Shared_Image::add_handler(File::previewPNG);
  Fl_Shared_Image::add_handler(File::previewBMP);
  Fl_Shared_Image::add_handler(File::previewTGA);
  Fl_Shared_Image::add_handler(File::previewGPL);

  fl_message_hotspot(0);

  Project::init();
  Undo::init();
  Gui::init();

  if(count > 1)
  {
    if(File::loadFile(argv[argc - 1]) < 0)
    {
      printf("Usage: %s <options> <filename>\n", argv[0]);
      return 0;
    }
  }

  int ret = Fl::run();

  // delete undo bitmaps
  Undo::free();

  // delete project bitmaps
  delete Project::bmp;
  delete Project::map;

  return ret;
}

