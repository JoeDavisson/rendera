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

#ifndef MAP_H
#define MAP_H

class Map
{
public:
  Map(int, int);
  ~Map();

  int w, h;
  unsigned char *data;
  unsigned char **row;

  // make single-pixel antialised lines appear thicker
  int thick_aa;

  // used by brushstroke preview
  bool isEdge(const int, const int);

  // drawing functions
  void resize(int, int);
  void clear(const unsigned char);
  void invert();
  void setpixel(const int, const int, const int);
  int getpixel(const int, const int);
  int getpixelNoClip(const int x, const int y);
  void line(int, int, int, int, int);
  void oval(int, int, int, int, int);
  void ovalfill(int, int, int, int, int);
  void rect(int, int, int, int, int);
  void rectfill(int, int, int, int, int);
  void hline(int, int, int, int);
  void vline(int, int, int, int);
  void polyfill(int *, int *, int, int, int, int);

  // antialiased versions
  void blendAA(int, int, int);
  void setpixelAA(int, int, int);
  void hlineAA(int, int, int, int);
  void lineAA(int, int, int, int, int);
  void ovalAA(int, int, int, int, int);
  void ovalfillAA(int, int, int, int, int);
  void rectAA(int, int, int, int, int);
  void rectfillAA(int, int, int, int, int);
  void polyfillAA(int *, int *, int, int, int, int);

  // cheap edge feathering
  void shrinkBlock(unsigned char *, unsigned char *,
                   unsigned char *, unsigned char *);
  void growBlock(unsigned char *, unsigned char *,
                 unsigned char *, unsigned char *);
  void shrink(int);
  void grow(int);
  void dilate();

  // misc
  void blur(int);
};

#endif
