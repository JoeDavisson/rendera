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
class Octree;

#include "Rendera.H"

class Palette
{
public:
  Palette();
  virtual ~Palette();

  void draw(Widget *);
  void copy(Palette *);
  void insertColor(int, int);
  void deleteColor(int);
  void replaceColor(int, int);
  void fillTable();
  int lookup(const int &);
  void sort();
  int load(const char *);
  int save(const char *);

  void setDefault();
  void setWebSafe();
  void set3LevelRGB();
  void set4LevelRGB();
  void set332();

  int *data;
  Octree *table;
  int max;
};

#endif

