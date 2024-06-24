/*
Copyright (c) 2024 Joe Davisson.

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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <stdint.h>
#include <vector>

#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "Bitmap.H"
#include "CheckBox.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "ExportData.H"
#include "File.H"
#include "FileSP.H"
#include "Gui.H"
#include "Inline.H"
#include "InputInt.H"
#include "Palette.H"
#include "Project.H"
#include "View.H"

namespace ExportOptions
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *tilex;
    InputInt *tiley;
    Fl_Choice *bpp;
    CheckBox *aligned;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  int begin()
  {
    Items::dialog->show();

    while (true)
    {
      Fl_Widget *action = Fl::readqueue();

      if (!action)
      {
        Fl::wait();
      }
      else if (action == Items::ok)
      {
        Items::dialog->hide();
        return 0;
      }
      else if (action == Items::cancel)
      {
        Items::dialog->hide();
        return -1;
      }
    }
  }

  void close()
  {
    Items::dialog->hide();
  }

  void quit()
  {
    Items::dialog->hide();
  }

  void init()
  {
    int y1 = 8;
    int ww = 0, hh = 0;

    Items::dialog = new DialogWindow(256, 0, "Export Data Options");

    Items::tilex = new InputInt(Items::dialog, 0, y1, 96, 24, "Tile Width:", 0, 1, 256);
    Items::tilex->value("8");
    Items::tilex->center();
    y1 += 24 + 8;

    Items::tiley = new InputInt(Items::dialog, 0, y1, 96, 24, "Tile Height:", 0, 1, 256);
    Items::tiley->value("8");
    Items::tiley->center();
    y1 += 24 + 8;

    Items::bpp = new Fl_Choice(0, y1, 128, 24, "Colors:");
    Items::bpp->labelsize(12);
    Items::bpp->textsize(12);
    Items::bpp->add("2 (1 bpp)");
    Items::bpp->add("4 (2 bpp)");
    Items::bpp->add("16 (4 bpp)");
    Items::bpp->add("256 (8 bpp)");
    Items::bpp->value(0);
    Items::bpp->align(FL_ALIGN_LEFT);
    Items::bpp->measure_label(ww, hh);
    Items::bpp->resize(Items::dialog->x() + Items::dialog->w() / 2
                        - (Items::bpp->w() + ww) / 2 + ww,
                        Items::bpp->y(), Items::bpp->w(), Items::bpp->h());
    y1 += 24 + 8;

    Items::aligned = new CheckBox(Items::dialog, 0, y1, 16, 16, "Aligned (Binary)", 0);
    Items::aligned->tooltip("Keeps binary output aligned\nto even values");
    Items::aligned->center();
    y1 += 24 + 8;

    Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
    Items::dialog->set_modal();
    Items::dialog->end();
  }
}

int ExportData::last_type = 0;

// store previous directory paths
char ExportData::save_dir[256];

const char *ExportData::ext_string[] = { ".bin", ".asm", ".java" };

// show error dialog
void ExportData::errorMessage()
{
  Dialog::message("File Error", "An error occured during the operation.");
}

// check if trying to overwrite existing file
bool ExportData::fileExists(const char *fn)
{
  FILE *temp = fopen(fn, "rb");

  if (temp)
  {
    fclose(temp);
    return 1;
  }

  return 0;
}

void ExportData::init()
{
  ExportOptions::init();
  strcpy(save_dir, ".");
}

void ExportData::save(Fl_Widget *, void *)
{
  if (ExportOptions::begin() == -1)
    return;

  Fl_Native_File_Chooser fc;
  fc.title("Save Image");
  fc.filter("Binary Data \t*.bin\n"
            "Assembly \t*.asm\n"
            "Java Array \t*.java\n");

  fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc.filter_value(last_type);
  fc.directory(save_dir);

  switch (fc.show())
  {
    case -1:
    case 1:
      return;
    default:
      File::getDirectory(save_dir, fc.filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc.filename());
  int ext_value = fc.filter_value();
  fl_filename_setext(fn, sizeof(fn), ext_string[ext_value]);

  if (fileExists(fn))
  {
    if (!Dialog::choice("Replace File?", "Overwrite?"))
      return;
  }

  int ret = -1;
  
  ret = ExportData::saveText(fn, ext_value);

  if (ret < 0)
    errorMessage();

  last_type = ext_value;
}

int ExportData::beginTile(FILE *outp, int ext_value, int index)
{
  switch (ext_value)
  {
    case TYPE_BIN:
      break;
    case TYPE_ASM:
      if (fprintf(outp, "tile_%d:\n  db ", index) < 0)
        return -1;
      break;
    case TYPE_JAVA:
      if (fprintf(outp, "  public static byte[] tile_%d =\n  {\n    ", index) < 0)
        return -1;
      break;
  }

  return 0;
}

int ExportData::writeByte(FILE *outp, int ext_value, uint8_t value)
{
  switch (ext_value)
  {
    case TYPE_BIN:
      if (fputc(value, outp) != value)
         return -1;
      break;
    case TYPE_ASM:
      if (fprintf(outp, "0x%02x", value) < 0)
         return -1;
      break;
    case TYPE_JAVA:
      if (value > 127)
        value -= 256;

      if (fprintf(outp, "%d", value) < 0)
         return -1;
      break;
    default:
      return -1;
  }

  return 0;
}

int ExportData::newLine(FILE *outp, int ext_value)
{
  switch (ext_value)
  {
    case TYPE_BIN:
      break;
    case TYPE_ASM:
      if (fprintf(outp, "  db ") < 0)
        return -1;
      break;
    case TYPE_JAVA:
      if (fprintf(outp, "    ") < 0)
        return -1;
      break;
    default:
      return -1;
  }

  return 0;
}

int ExportData::endTile(FILE *outp, int ext_value)
{
  switch (ext_value)
  {
    case TYPE_BIN:
      if (ExportOptions::Items::aligned->value())
      {
        if (fputc(0, outp) != 0)
           return -1;
      }
      break;
    case TYPE_ASM:
      if (fprintf(outp, "\n") < 0)
        return -1;
      break;
    case TYPE_JAVA:
      if (fprintf(outp, "  };\n\n") < 0)
        return -1;
      break;
    default:
      return -1;
  }

  return 0;
}

int ExportData::saveText(const char *fn, int ext_value)
{
  const char *mode_str[2] = { "wb", "w" };

  FileSP out(fn, mode_str[ext_value == TYPE_BIN ? 0 : 1]);

  FILE *outp = out.get();
  if (!outp)
    return -1;

  Bitmap *bmp = Project::bmp;
  Palette *pal = Project::palette;

  int tilex = atoi(ExportOptions::Items::tilex->value());
  int tiley = atoi(ExportOptions::Items::tiley->value());
  int bit_level = ExportOptions::Items::bpp->value();
  int shift = 1 << bit_level;
  int pixels = 1 << (3 - bit_level);
  int count = 0;
  int tile_count = 0;
  int tile_bytes = (tilex / pixels) * tiley;

/*
  printf("pixels = %d\n", pixels);
  printf("shift = %d\n", shift);
  printf("tilex = %d\n", tilex);
  printf("tiley = %d\n", tiley);
  printf("tile_bytes = %d\n", tile_bytes);
*/

  for (int y = 0; y < bmp->h; y += tiley)
  {
    for (int x = 0; x < bmp->w; x += tilex)
    {
      if (beginTile(outp, ext_value, tile_count++) < 0)
        return -1;

      int tile_byte_count = 0;

      for (int j = 0; j < tiley; j++)
      {
        for (int i = 0; i < tilex; i += pixels)
        {
          int value = 0;

          for (int z = 0; z < pixels; z++)
          {
            int c = pal->lookup(bmp->getpixel(x + i + z, y + j));

            if (c >= (1 << shift))
              c = 0;

            value |= c;

            if (z < pixels - 1)
              value <<= shift;
          }

          if (writeByte(outp, ext_value, value) < 0)
            return -1;

          count++;

          if (count > 7)
          {
            count = 0;

            if (ext_value != TYPE_BIN)
              if (fprintf(outp, "\n") < 0)
                return -1;

            if (tile_byte_count < tile_bytes - 1)
              if (newLine(outp, ext_value) < 0)
                return -1;
          }
            else
          {
            if (ext_value != TYPE_BIN)
              if (fprintf(outp, ", ") < 0)
                return -1;
          }

          tile_byte_count++;
        }
      }

      if (endTile(outp, ext_value) < 0)
        return -1;
    }
  }

  return 0;
}

