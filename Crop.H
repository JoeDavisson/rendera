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

#ifndef CROP_H
#define CROP_H

class View;

#include "Rendera.H"
#include "Tool.H"

class Crop : public Tool
{
public:
  Crop();
  virtual ~Crop();

  virtual void push(View *);
  virtual void drag(View *);
  virtual void release(View *);
  virtual void move(View *);
  virtual void done(View *);
  virtual void redraw(View *);

private:
  int drag_started;
  int resize_started;
  int side;
  int offset;
};

#endif

