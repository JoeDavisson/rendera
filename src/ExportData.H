/*
Copyright (c) 2023 Joe Davisson.

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

#ifndef EXPORT_DATA_H
#define EXPORT_DATA_H

#include <cstdio>
#include <stdint.h>

class Fl_Widget;
class Fl_Image;
class Bitmap;

class ExportData
{
public:
  enum
  {
    TYPE_BIN,
    TYPE_ASM,
    TYPE_JAVA
  };

  static void init();

  static void save(Fl_Widget *, void *);
  static int beginTile(FILE *, int, int);
  static int writeByte(FILE *, int, uint8_t);
  static int newLine(FILE *, int);
  static int endTile(FILE *, int);
  static int saveText(const char *, int);

private:
  ExportData() { }
  ~ExportData() { }

  static int last_type;
  static char save_dir[256];
  static const char *ext_string[];

  static void errorMessage();
  static bool fileExists(const char *);
};

#endif

