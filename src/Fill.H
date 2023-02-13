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

#ifndef FILL_H
#define FILL_H

class Map;
class View;

#include <vector>

#include "Tool.H"

class Fill : public Tool
{
public:
  Fill();
  ~Fill();

  void push(View *);
  void drag(View *);
  void release(View *);
  void move(View *);
  void key(View *);
  void done(View *, int);
  void redraw(View *);
  bool isActive();
  void reset();

private:
  int *stack_x;
  int *stack_y;
  int stack_size;
  int sp;

  bool inbox(int, int, int, int, int, int);
  bool isEdge(Map *, const int, const int);
  int fineEdge(int, int, const int, const int, const int, const int);
  bool pop(int *, int *);
  bool push(int, int);
  void clear();
  bool inRange(const int, const int, const int);
  void fill(int, int, int, int, int, int);
};

#endif

