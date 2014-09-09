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

class View;

#include "rendera.h"
#include "Tool.H"

class Paint : public Tool
{
public:
  Paint();
  virtual ~Paint();

  virtual void renderBeginNormal(View *);
  virtual void renderBeginSmooth(View *);
  virtual void renderBegin(View *);
  virtual int renderCallbackNormal(View *);
  virtual int renderCallbackSmooth(View *);
  virtual int renderCallback(View *);

  virtual void push(View *);
  virtual void drag(View *);
  virtual void release(View *);
  virtual void move(View *);
  virtual void done(View *);
  virtual void redraw(View *);

private:
  int render_pos, render_end, render_count;
  float soft_trans, soft_step;
};

#endif

