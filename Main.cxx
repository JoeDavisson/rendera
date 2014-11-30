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

#include <getopt.h>

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
  enum
  {
    OPTION_THEME,
    OPTION_HELP
  };

  int verbose_flag;

  struct option long_options[] =
  {
    { "theme", required_argument, &verbose_flag, OPTION_THEME },
    { "help", no_argument, &verbose_flag, OPTION_HELP },
    { 0, 0, 0, 0 }
  };

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

  // parse command line
  int c;
  int option_index = 0;

  while(1)
  {
    c = getopt_long(argc, argv, "", long_options, &option_index);
    if(c < 0)
      break;

    switch(c)
    {
      case 0:
        switch(option_index)
        {
          case OPTION_THEME:
            if(strcmp(optarg, "dark") == 0)
              setDarkTheme();
            else if(strcmp(optarg, "light") == 0)
              setLightTheme();
            else
              printf("Unknown theme: %s\n", optarg);
            break;
          case OPTION_HELP:
            printf("\n");
            printf("Usage: rendera [OPTIONS] filename\n");
            printf("\n");
            printf("Options:\n");
            printf("  --theme=dark\t\t use dark theme\n");
            printf("  --theme=light\t\t use light theme\n");
            printf("\n");
            return 0;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

  Fl::visual(FL_DOUBLE | FL_RGB);

  Fl::scheme("gtk+");
  Fl_Shared_Image::add_handler(File::previewJpeg);
  Fl_Shared_Image::add_handler(File::previewPng);
  Fl_Shared_Image::add_handler(File::previewBmp);
  Fl_Shared_Image::add_handler(File::previewTarga);
  Fl_Shared_Image::add_handler(File::previewGimpPalette);

  fl_message_hotspot(0);

  Project::init();
  Undo::init();
  Dialog::init();
  FX::init();
  Transform::init();
  Gui::init();

  // try to load image from command line
  if(optind < argc)
  {
    if(File::loadFile(argv[optind]) < 0)
    {
      printf("Could not load image: %s\n", argv[optind]);
      return 0;
    }
  }

  // delay showing main gui until after all arguments are checked
  Gui::show();

  int ret = Fl::run();
  return ret;
}

