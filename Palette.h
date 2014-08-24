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

#ifndef PALETTE_H
#define PALETTE_H

class Widget;

#include "rendera.h"

class Palette
{
public:
  Palette();
  virtual ~Palette();

  void draw(Widget *);
  void copy(Palette *);
  void set_default();
  void insert_color(int, int);
  void delete_color(int);
  void replace_color(int, int);
  void fill_lookup();
  void load(const char *);
  void save(const char *);

  int *data;
  unsigned char *lookup;
  int max;

  static Palette *main;
  static Palette *undo;
};

#endif

