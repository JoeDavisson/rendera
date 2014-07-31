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

#include "rendera.h"

extern Gui *gui;

Bitmap *undo_stack[MAX_UNDO];
int undo_current;

void undo_init()
{
  int i;

  for(i = 0; i < MAX_UNDO; i++)
    undo_stack[i] = new Bitmap(8, 8);

  undo_current = MAX_UNDO - 1;
}

void undo_push(int x, int y, int w, int h)
{
  if(undo_current < 0)
    return;

  Bitmap *bmp = undo_stack[undo_current];

  delete bmp;
  bmp = new Bitmap(w, h);
  bmp->x = x;
  bmp->y = y;

  Bitmap::main->blit(bmp, x, y, 0, 0, w, h);
  undo_current--;
}

void undo_pop()
{
  if(undo_current == MAX_UNDO - 1)
    return;

  undo_current++;

  Bitmap *bmp = undo_stack[undo_current];
  bmp->blit(Bitmap::main, 0, 0, bmp->x, bmp->y, bmp->w, bmp->h);

  gui->view->draw_main(1);
}

