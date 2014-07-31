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

#define MAX_UNDO 32

extern Gui *gui;

Bitmap *undo_stack[MAX_UNDO];
int undo_resized[MAX_UNDO];
int undo_current;

void undo_init()
{
  int i;

  for(i = 0; i < MAX_UNDO; i++)
  {
    undo_stack[i] = new Bitmap(8, 8);
    undo_resized[i] = 0;
  }

  undo_current = MAX_UNDO - 1;
}

void undo_push(int x, int y, int w, int h, int resized)
{
  int i;

  if(undo_current < 0)
  {
    undo_current = 0;

    undo_stack[0] = undo_stack[MAX_UNDO - 1];
    for(i = MAX_UNDO - 1; i > 0; i--)
    {
      undo_stack[i] = undo_stack[i - 1];
    }
  }

  Bitmap *bmp = undo_stack[undo_current];
  undo_resized[undo_current] = resized;

  delete bmp;
  bmp = new Bitmap(w, h);
  bmp->x = x;
  bmp->y = y;

  Bitmap::main->blit(bmp, x, y, 0, 0, w, h);
  undo_current--;
}

void undo_pop()
{
  if(undo_current >= MAX_UNDO - 1)
    return;

  undo_current++;
  Bitmap *bmp = undo_stack[undo_current];

  if(undo_resized[undo_current])
  {
    delete Bitmap::main;
    Bitmap::main = new Bitmap(bmp->w, bmp->h, 64,
                              makecol(255, 255, 255), makecol(128, 128, 128));
    gui->view->ox = 0;
    gui->view->oy = 0;
  }

  bmp->blit(Bitmap::main, 0, 0, bmp->x, bmp->y, bmp->w, bmp->h);

  gui->view->draw_main(1);
}

