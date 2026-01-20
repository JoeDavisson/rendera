/*
Copyright (c) 2025 Joe Davisson.

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

//#include <FL/Fl_Shared_Image.H>
#include "FL/Fl_File_Icon.H"

#include "Blend.H"
#include "Dialog.H"
#include "Editor.H"
#include "ExportData.H"
#include "File.H"
#include "FontPreview.H"
#include "FX/FX.H"
#include "Gamma.H"
#include "Gui.H"
#include "Inline.H"
#include "Project.H"
#include "Transform.H"
#include "Undo.H"

#include "Palette.H"

FL_EXPORT bool fl_disable_wayland = true;

enum
{
  OPTION_MEM,
  OPTION_UNDOS,
  OPTION_VERSION,
  OPTION_HELP
};

static int verbose_flag;

struct option long_options[] =
{
  { "mem", optional_argument,       &verbose_flag, OPTION_MEM },
  { "undos", optional_argument,       &verbose_flag, OPTION_UNDOS },
  { "version", no_argument,       &verbose_flag, OPTION_VERSION },
  { "help",    no_argument,       &verbose_flag, OPTION_HELP    },
  { 0, 0, 0, 0 }
};

void setDarkTheme()
{
  int r, g, b;
  int h = 0;
  int s = 0;

  // greyscale ramps (colors 32 - 55) used in the GUI
  for (int i = 0; i < 24; i++)
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

void printHelp()
{
  printf("Usage: rendera [OPTIONS] filename\n\n");
  printf("--mem=<value>\t\t memory limit (in megabytes)\n");
  printf("--undos=<value>\t\t undo limit (1-100)\n");
  printf("--version\t\t version information\n\n");
}

int main(int argc, char *argv[])
{
  //Fl::keyboard_screen_scaling(0);
  Fl_File_Icon::load_system_icons();
  setDarkTheme();

  // parse command line
  int memory_max = 1000;
  int undo_max = 16;
  int option_index = 0;
  bool exit = false;
  bool custom_settings = false;

  while (true)
  {
    const int c = getopt_long(argc, argv, "", long_options, &option_index);
    if (c < 0)
      break;

    switch (c)
    {
      case 0:
      {
        switch (option_index)
        {
          case OPTION_HELP:
            printHelp();
            exit = true;
            break;

          case OPTION_VERSION:
            printf("Rendera %s\n\n", PACKAGE_STRING);
            printf("This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
            exit = true;
            break;

          case OPTION_MEM:
            if (optarg)
            {
              memory_max = atoi(optarg);

              if (memory_max < 16)
                memory_max = 16;

              printf("Image memory limit set to: %d MB\n", memory_max);
              custom_settings = true;
              exit = false;
            }
              else
            {
              printHelp();
              exit = true;
            }
            
            break;

          case OPTION_UNDOS:
            if (optarg)
            {
              undo_max = atoi(optarg);

              if (undo_max < 1)
                undo_max = 1;

              if (undo_max > 100)
                undo_max = 100;

              printf("Undo levels set to: %d\n", undo_max);
              custom_settings = true;
              exit = false;
            }
              else
            {
              printHelp();
              exit = true;
              break;
            }
            
            break;

          default:
            printHelp();
            exit = true;
            break;
        }

        break;
      }

      default:
      {
        printHelp();
        exit = true;
        break;
      }
    }
  }

  if (exit == true)
  {
    #ifndef WIN32
      return 0;
    #endif
  }

  // fltk related inits
  Fl::visual(FL_DOUBLE | FL_RGB);
  Fl::scheme("gtk+");

  // program initalization
  Gamma::init();
  Project::init(memory_max, undo_max);
  File::init();
  ExportData::init();
  FX::init();
  Transform::init();
  FontPreview::init();
  Gui::init();
  Dialog::init();
  Editor::init();

  //Fl_Shared_Image::add_handler(File::previewJpeg);
  //Fl_Shared_Image::add_handler(File::previewPng);
  //Fl_Shared_Image::add_handler(File::previewBmp);
  //Fl_Shared_Image::add_handler(File::previewTarga);
  //Fl_Shared_Image::add_handler(File::previewGimpPalette);

  // try to load image from command line
  if (optind < argc)
  {
    if (File::loadFile(argv[optind]) < 0)
    {
      printf("Could not load image: %s\n", argv[optind]);
      return 0;
    }
  }

  // delay showing main gui until after all arguments are checked
  Gui::show();
//  Gui::imagesAddFile("new");

    if (custom_settings == true)
    {
      #ifdef WIN32
      char s[256];
      snprintf(s, sizeof(s), "Rendera %s\nImage memory limit set to: %d MB\nUndo levels set to: %d", PACKAGE_STRING, memory_max, undo_max);

      Dialog::message("Custom Settings", s);
      #endif
    }

/*
  // view FLTK theme palette (for testing)
  Project::palette->max = 256;
  for (int i = 20; i < 256; i++)
  {
    int c = getFltkColor(i);
    Project::palette->data[i] = c;
  }
  Gui::drawPalette();
*/

  Fl::add_timeout(1.0 / 10, (Fl_Timeout_Handler)Gui::updateMemInfo);
  Fl::add_timeout(1.0 / 125, (Fl_Timeout_Handler)Gui::mouseTimer);

  return Fl::run();
}

