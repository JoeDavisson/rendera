/*
Copyright (c) 2021 Joe Davisson.

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

#include "Bitmap.H"
#include "Gui.H"
#include "Offset.H"
#include "Project.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"

namespace
{
  int beginx = 0, beginy = 0;
  Bitmap *offset_buffer = 0;
}

Offset::Offset()
{
}

Offset::~Offset()
{
}

void Offset::push(View *view)
{
  Project::undo->push();

  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  beginx = view->imgx;
  beginy = view->imgy;

  if(offset_buffer)
    delete offset_buffer;

  offset_buffer = new Bitmap(w, h);
  Project::bmp->blit(offset_buffer, 0, 0, 0, 0, w, h);
}

void Offset::drag(View *view)
{
  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  int dx = view->imgx - beginx;
  int dy = view->imgy - beginy;

  int x = dx;
  int y = dy;

  while(x < 0)
    x += w;
  while(y < 0)
    y += h;
  while(x >= w)
    x -= w;
  while(y >= h)
    y -= h;

  offset_buffer->blit(Project::bmp, w - x, h - y, 0, 0, x, y);
  offset_buffer->blit(Project::bmp, 0, h - y, x, 0, w - x, y);
  offset_buffer->blit(Project::bmp, w - x, 0, 0, y, x, h - y);
  offset_buffer->blit(Project::bmp, 0, 0, x, y, w - x, h - y);

  view->drawMain(true);
  Gui::checkOffsetValues(dx, dy);
}

void Offset::release(View *)
{
  if(offset_buffer)
    delete offset_buffer;

  offset_buffer = 0;
  Gui::checkOffsetValues(0, 0);
}

void Offset::move(View *)
{
}

void Offset::key(View *)
{
}

void Offset::done(View *, int)
{
}

void Offset::redraw(View *)
{
}

bool Offset::isActive()
{
  return false;
}

void Offset::reset()
{
}

