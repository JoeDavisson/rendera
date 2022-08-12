/*
Copyright (c) 2021 Joe Davisson.

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

#ifndef PACKAGE_STRING
#  include "config.h"
#endif

#include <getopt.h>
#include <iostream>

//#include <FL/Fl_Shared_Image.H>

#include "Blend.H"
#include "Dialog.H"
#include "File.H"
#include "FX.H"
#include "FX2.H"
#include "Gui.H"
#include "Inline.H"
#include "Project.H"
#include "Transform.H"
#include "Undo.H"

#include "Palette.H"

namespace
{
  enum
  {
    OPTION_VERSION,
    OPTION_HELP
  };

  int verbose_flag;
  int hue = 0;
  int sat = 0;

  struct option long_options[] =
  {
    { "version", no_argument,       &verbose_flag, OPTION_VERSION },
    { "help",    no_argument,       &verbose_flag, OPTION_HELP    },
    { 0, 0, 0, 0 }
  };

  void setDarkTheme()
  {
    int r, g, b, h = hue, s = sat;

    // 32 - 55
    for(int i = 0; i < 24; i++)
    {
      int v = i * 6;
      Fl::set_color(32 + i, fl_rgb_color(v, v, v));
    }

    Project::theme = Project::THEME_DARK;

    Blend::hsvToRgb(h, s, 64, &r, &g, &b);
    Fl::set_color(FL_BACKGROUND_COLOR, fl_rgb_color(r, g, b));

    Blend::hsvToRgb(h, s, 48, &r, &g, &b);
    Fl::set_color(FL_BACKGROUND2_COLOR, fl_rgb_color(r, g, b));

    Blend::hsvToRgb(h, s, 208, &r, &g, &b);
    Fl::set_color(FL_FOREGROUND_COLOR, fl_rgb_color(r, g, b));

    Blend::hsvToRgb(h, s, 56, &r, &g, &b);
    Fl::set_color(FL_INACTIVE_COLOR, fl_rgb_color(r, g, b));

    Blend::hsvToRgb(h, s, 208, &r, &g, &b);
    Fl::set_color(FL_SELECTION_COLOR, fl_rgb_color(r, g, b));

    Blend::hsvToRgb(0, 0, 128, &r, &g, &b);
    Project::theme_highlight_color = makeRgb(r, g, b);

    const int blend = Project::theme_highlight_color;
    Project::fltk_theme_highlight_color = fl_rgb_color(getr(blend),
                                                       getg(blend),
                                                       getb(blend));
    Blend::hsvToRgb(h, s, 128, &r, &g, &b);
    Project::fltk_theme_bevel_up = fl_rgb_color(r, g, b);
    Blend::hsvToRgb(h, s, 16, &r, &g, &b);
    Project::fltk_theme_bevel_down = fl_rgb_color(r, g, b);
  }

  struct _help_type {};

  std::ostream &
  operator << (std::ostream &os, _help_type const &)
  {
    return
      os
      << std::endl << "Usage: rendera [OPTIONS] filename"
      << std::endl
      << std::endl << "  --version\t\t version information"
      << std::endl << std::endl;
  }
}

int main(int argc, char *argv[])
{
  setDarkTheme();

  // parse command line
  int option_index = 0;

  while(true)
  {
    const int c = getopt_long(argc, argv, "", long_options, &option_index);
    if(c < 0)
      break;

    switch(c)
    {
      case 0:
      {
        switch(option_index)
        {
          case OPTION_HELP:
            std::cout << _help_type();
            return EXIT_SUCCESS;

          case OPTION_VERSION:
            std::cout << PACKAGE_STRING << std::endl;
            return EXIT_SUCCESS;

          default:
            std::cerr << _help_type();
            return EXIT_FAILURE ;
        }

        break;
      }

      default:
      {
        std::cerr << _help_type();
        return EXIT_FAILURE ;
      }
    }
  }

  // fltk related inits
  Fl::visual(FL_DOUBLE | FL_RGB);
  Fl::scheme("gtk+");

  // program inits
  Project::init();
  File::init();
  Dialog::init();
  FX::init();
  FX2::init();
  Transform::init();
  Gui::init();

  //Fl_Shared_Image::add_handler(File::previewJpeg);
  //Fl_Shared_Image::add_handler(File::previewPng);
  //Fl_Shared_Image::add_handler(File::previewBmp);
  //Fl_Shared_Image::add_handler(File::previewTarga);
  //Fl_Shared_Image::add_handler(File::previewGimpPalette);

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

/*
  // view theme palette (for testing)
  Project::palette->max = 256;
  for(int i = 20; i < 256; i++)
  {
    int c = getFltkColor(i);
    Project::palette->data[i] = c;
  }
  Gui::drawPalette();
*/

  int ret = Fl::run();
  return ret;
}

