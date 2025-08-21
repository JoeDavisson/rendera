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

#ifndef RENDER_H
#define RENDER_H

class Bitmap;
class Map;
class Brush;
class Stroke;
class View;

class Render
{
public:
  enum
  {
    SOLID,
    ANTIALIASED,
    COARSE,
    FINE,
    BLURRY,
    WATERCOLOR,
    CHALK,
    TEXTURE,
    AVERAGE
  };

  static Bitmap *bmp;
  static Map *map;
  static Brush *brush;
  static Stroke *stroke;
  static View *view;
  static int color;
  static int trans;

  static void begin();

private:
  Render() { }
  ~Render() { }

  static bool isEdge(Map *, const int, const int);
  static int fineEdge(int, int, const int, const int, const int, const int);
  static void shrinkBlock(unsigned char *, unsigned char *,
                          unsigned char *, unsigned char *);
  static void growBlock(unsigned char *, unsigned char *,
                        unsigned char *, unsigned char *);
  static int update(int);

  static void solid();
  static void antialiased();
  static void coarse();
  static void fine();
  static void blur();
  static void watercolor();
  static void chalk();
  static void texture();
  static void average();
};

#endif

