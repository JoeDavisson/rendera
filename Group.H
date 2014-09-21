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

#ifndef GROUP_H
#define GROUP_H

class Bitmap;

#include "Rendera.H"

class Group : public Fl_Group
{
public:
  Group(int, int, int, int);
  virtual ~Group();

  Bitmap *bitmap;
  Fl_RGB_Image *image;
  Fl_Tiled_Image *tiled_image;
protected:
  virtual void draw();
};

#endif

