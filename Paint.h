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

#ifndef PAINT_H
#define PAINT_H

#include "rendera.h"

class Paint : public Tool
{
public:
  Paint();
  virtual ~Paint();

  virtual void render_begin_normal(View *);
  virtual void render_begin_smooth(View *);
  virtual void render_begin(View *);
  virtual int render_callback_normal(View *);
  virtual int render_callback_smooth(View *);
  virtual int render_callback(View *);

  virtual void push(View *);
  virtual void drag(View *);
  virtual void release(View *);
  virtual void move(View *);

private:
  int render_pos, render_end, render_count;
  float soft_trans, soft_step;
};

#endif

