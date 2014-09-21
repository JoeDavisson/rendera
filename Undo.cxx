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

#include "Undo.H"
#include "Bitmap.H"
#include "Map.H"
#include "Gui.H"
#include "View.H"

#define MAX_UNDO 10

namespace
{
  Bitmap *undo_stack[MAX_UNDO];
  int undo_resized[MAX_UNDO];
  int undo_current;
}

void Undo::init()
{
  int i;

  for(i = 0; i < MAX_UNDO; i++)
  {
    undo_stack[i] = new Bitmap(8, 8);
    undo_resized[i] = 0;
  }

  undo_current = MAX_UNDO - 1;
}

void Undo::reset()
{
  int i;

  // free some memory
  for(i = 0; i < MAX_UNDO; i++)
  {
    delete undo_stack[i];
    undo_stack[i] = new Bitmap(8, 8);
    undo_resized[i] = 0;
  }

  undo_current = MAX_UNDO - 1;
}

void Undo::push(int x, int y, int w, int h, int resized)
{
  int i;

  if(undo_current < 0)
  {
    undo_current = 0;

    Bitmap *temp_bmp = undo_stack[MAX_UNDO - 1];
    int temp_resized = undo_resized[MAX_UNDO - 1];
    for(i = MAX_UNDO - 1; i > 0; i--)
    {
      undo_stack[i] = undo_stack[i - 1];
      undo_resized[i] = undo_resized[i - 1];
    }

    undo_stack[0] = temp_bmp;
    undo_resized[0] = temp_resized;
  }

  undo_resized[undo_current] = resized;

  delete undo_stack[undo_current];
  undo_stack[undo_current] = new Bitmap(w, h);
  undo_stack[undo_current]->x = x;
  undo_stack[undo_current]->y = y;

  Bitmap::main->blit(undo_stack[undo_current], x, y, 0, 0, w, h);
  undo_current--;
}

void Undo::pop()
{
  if(undo_current >= MAX_UNDO - 1)
    return;

  undo_current++;

  if(undo_resized[undo_current])
  {
    int overscroll = Bitmap::main->overscroll;
    delete Bitmap::main;
    Bitmap::main = new Bitmap(undo_stack[undo_current]->w,
                              undo_stack[undo_current]->h,
                              overscroll,
                              makeRgb(255, 255, 255),
                              getFltkColor(FL_BACKGROUND2_COLOR));
    delete Map::main;
    Map::main = new Map(Bitmap::main->w, Bitmap::main->h);
    Gui::getView()->ox = 0;
    Gui::getView()->oy = 0;
    undo_stack[undo_current]->blit(Bitmap::main, 0, 0,
                                   Bitmap::main->overscroll,
                                   Bitmap::main->overscroll,
                                   undo_stack[undo_current]->w,
                                   undo_stack[undo_current]->h);
  }
  else
  {
    undo_stack[undo_current]->blit(Bitmap::main, 0, 0, 
                                   undo_stack[undo_current]->x,
                                   undo_stack[undo_current]->y,
                                   undo_stack[undo_current]->w,
                                   undo_stack[undo_current]->h);
  }

  Gui::getView()->drawMain(1);
}

