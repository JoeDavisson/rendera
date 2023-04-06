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

#ifndef CLONE_H
#define CLONE_H

class Bitmap;
class Widget;
class View;

class Fl_Double_Window;

class Clone
{
public:
  enum
  {
    RESET,
    PLACED,
    STARTED
  };

  static int x;
  static int y;
  static int dx;
  static int dy;
  static int state;
  static bool active;
  static bool moved;
  static Bitmap *buffer_bmp;

  static void move(int, int);
  static void refresh(int, int, int, int);

private:
  Clone() { }
  ~Clone() { }
};

#endif

