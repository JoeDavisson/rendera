/*
Copyright (c) 2024 Joe Davisson.

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

/*
Notes:

The undo/redo feature tries not to waste more memory than required by using
three different modes (see enum in Undo.H):

8x8 pixel dummy images are used as placeholders in the undo/redo stacks. Modes
are stored in the stack images themselves (undo_mode in class Bitmap).
*/

#include "Bitmap.H"
#include "Gui.H"
#include "Project.H"
#include "Undo.H"
#include "View.H"

// for debugging
void Undo::printStacks()
{
  printf("Undo Stack:\n");

  for (int i = 0; i < levels; i++)
  {
    int x2 = undo_stack[i]->x;
    int y2 = undo_stack[i]->y;
    int w2 = undo_stack[i]->w;
    int h2 = undo_stack[i]->h;

    printf("%d: x2 = %d, y2 = %d, w2 = %d, h2 = %d ", i, x2, y2, w2, h2);

    if (i == undo_current)
      printf("<");

    printf("\n");
  }

  printf("\n");
  printf("Redo Stack:\n");

  for (int i = 0; i < levels; i++)
  {
    int x2 = redo_stack[i]->x;
    int y2 = redo_stack[i]->y;
    int w2 = redo_stack[i]->w;
    int h2 = redo_stack[i]->h;

    printf("%d: x2 = %d, y2 = %d, w2 = %d, h2 = %d ", i, x2, y2, w2, h2);

    if (i == redo_current)
      printf("<");

    printf("\n");
  }

  printf("--------------------------------------\n\n");
}

Undo::Undo()
{
  levels = Project::undo_max;

  undo_stack = new Bitmap *[levels];
  redo_stack = new Bitmap *[levels];

  for (int i = 0; i < levels; i++)
  {
    undo_stack[i] = 0;
    redo_stack[i] = 0;
  }

  reset();
}

Undo::~Undo()
{
  for (int i = 0; i < levels; i++)
  {
    delete undo_stack[i];
    delete redo_stack[i];
  }
}

void Undo::reset()
{
  for (int i = 0; i < levels; i++)
  {
    delete undo_stack[i];
    delete redo_stack[i];

    undo_stack[i] = new Bitmap(8, 8);
    redo_stack[i] = new Bitmap(8, 8);
  }

  undo_current = levels - 1;
  redo_current = levels - 1;
}

void Undo::doPush(const int x, const int y, const int w, const int h,
                  const int undo_mode)
{
  if (undo_current < 0)
  {
    undo_current = 0;

    Bitmap *temp_bmp = undo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      undo_stack[i] = undo_stack[i - 1];

    undo_stack[0] = temp_bmp;
  }

  if (undo_mode == Undo::OFFSET || undo_mode == Undo::FLIP_HORIZONTAL ||
      undo_mode == Undo::FLIP_VERTICAL || undo_mode == Undo::FLIP_VERTICAL ||
      undo_mode == Undo::ROTATE_180)
  {
    delete undo_stack[undo_current];
    undo_stack[undo_current] = new Bitmap(8, 8);
  }
   else
  {
    if (Project::enoughMemory(w, h) == false)
      return;

    delete undo_stack[undo_current];
    undo_stack[undo_current] = new Bitmap(w, h);

    Project::bmp->blit(undo_stack[undo_current], x, y, 0, 0, w, h);
  }

  undo_stack[undo_current]->x = x;
  undo_stack[undo_current]->y = y;
  undo_stack[undo_current]->undo_mode = undo_mode;
  undo_current--;
}

void Undo::push()
{
  const int x = 0;
  const int y = 0;
  const int w = Project::bmp->w;
  const int h = Project::bmp->h;
  const int undo_mode = Undo::FULL;

  push(x, y, w, h, undo_mode);
}

void Undo::push(const int undo_mode)
{
  const int x = 0;
  const int y = 0;
  const int w = Project::bmp->w;
  const int h = Project::bmp->h;

  push(x, y, w, h, undo_mode);
}

void Undo::push(const int x, const int y, const int w, const int h,
                const int undo_mode)
{
  doPush(x, y, w, h, undo_mode);

  // reset redo list since user performed some action
  for (int i = 0; i < levels; i++)
  {
    delete redo_stack[i];
    redo_stack[i] = new Bitmap(8, 8);
  }

  redo_current = levels - 1;

  // printStacks();
}

void Undo::pop()
{
  if (undo_current >= levels - 1)
    return;

  int x = undo_stack[undo_current + 1]->x;
  int y = undo_stack[undo_current + 1]->y;
  int w = undo_stack[undo_current + 1]->w;
  int h = undo_stack[undo_current + 1]->h;
  int undo_mode = undo_stack[undo_current + 1]->undo_mode;

  if (undo_mode == Undo::OFFSET)
  {
    Project::bmp->offset(x, y, true);
    undo_current++;
    Gui::getView()->drawMain(true);
    pushRedo(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::FLIP_HORIZONTAL)
  {
    Project::bmp->flipHorizontal();
    undo_current++;
    Gui::getView()->drawMain(true);
    pushRedo(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::FLIP_VERTICAL)
  {
    Project::bmp->flipVertical();
    undo_current++;
    Gui::getView()->drawMain(true);
    pushRedo(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::ROTATE_180)
  {
    Project::bmp->rotate180();
    undo_current++;
    Gui::getView()->drawMain(true);
    pushRedo(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::FULL)
  {
    pushRedo(0, 0, Project::bmp->w, Project::bmp->h, undo_mode);
  }
    else
  {
    pushRedo(x, y, w, h, undo_mode);
  }

  if (undo_current >= 0)
  {
    delete undo_stack[undo_current];
    undo_stack[undo_current] = new Bitmap(8, 8);
  }

  undo_current++;

  x = undo_stack[undo_current]->x;
  y = undo_stack[undo_current]->y;
  w = undo_stack[undo_current]->w;
  h = undo_stack[undo_current]->h;
  undo_mode = undo_stack[undo_current]->undo_mode;

  if(undo_mode == Undo::FULL)
    Project::replaceImage(w, h);

  undo_stack[undo_current]->blit(Project::bmp, 0, 0, x, y, w, h);
  Gui::getView()->drawMain(true);

  // printStacks();
}

void Undo::pushRedo(const int x, const int y, const int w, const int h,
                    const int undo_mode)
{
  if (redo_current < 0)
  {
    redo_current = 0;

    Bitmap *temp_bmp = redo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      redo_stack[i] = redo_stack[i - 1];

    redo_stack[0] = temp_bmp;
  }

  if (undo_mode == Undo::OFFSET || undo_mode == Undo::FLIP_HORIZONTAL ||
      undo_mode == Undo::FLIP_VERTICAL || undo_mode == Undo::FLIP_VERTICAL ||
      undo_mode == Undo::ROTATE_180)
  {
    delete redo_stack[redo_current];
    redo_stack[redo_current] = new Bitmap(8, 8);
  }
   else
  {
    if (Project::enoughMemory(w, h) == false)
      return;

    delete redo_stack[redo_current];
    redo_stack[redo_current] = new Bitmap(w, h);

    Project::bmp->blit(redo_stack[redo_current], x, y, 0, 0, w, h);
  }

  redo_stack[redo_current]->x = x;
  redo_stack[redo_current]->y = y;
  redo_stack[redo_current]->undo_mode = undo_mode;
  redo_current--;

  if (redo_current < 0)
    redo_current = 0;
}

void Undo::popRedo()
{
  if (redo_current >= levels - 1)
    return;

  int x = redo_stack[redo_current + 1]->x;
  int y = redo_stack[redo_current + 1]->y;
  int w = redo_stack[redo_current + 1]->w;
  int h = redo_stack[redo_current + 1]->h;
  int undo_mode = redo_stack[redo_current + 1]->undo_mode;

  if (undo_mode == Undo::OFFSET)
  {
    Project::bmp->offset(x, y, false);
    redo_current++;
    Gui::getView()->drawMain(true);
    doPush(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::FLIP_HORIZONTAL)
  {
    Project::bmp->flipHorizontal();
    redo_current++;
    Gui::getView()->drawMain(true);
    doPush(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::FLIP_VERTICAL)
  {
    Project::bmp->flipVertical();
    redo_current++;
    Gui::getView()->drawMain(true);
    doPush(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::ROTATE_180)
  {
    Project::bmp->rotate180();
    redo_current++;
    Gui::getView()->drawMain(true);
    doPush(x, y, w, h, undo_mode);
    return;
  }
  else if (undo_mode == Undo::FULL)
  {
    doPush(0, 0, Project::bmp->w, Project::bmp->h, 1);
  }
   else
  {
    doPush(x, y, w, h, undo_mode);
  }

  if (redo_current >= 0)
  {
    delete redo_stack[redo_current];
    redo_stack[redo_current] = new Bitmap(8, 8);
  }

  redo_current++;

  x = redo_stack[redo_current]->x;
  y = redo_stack[redo_current]->y;
  w = redo_stack[redo_current]->w;
  h = redo_stack[redo_current]->h;
  undo_mode = redo_stack[redo_current]->undo_mode;

  if (undo_mode == Undo::FULL)
    Project::replaceImage(w, h);

  redo_stack[redo_current]->blit(Project::bmp, 0, 0, x, y, w, h);

  Gui::getView()->drawMain(true);

  // printStacks();
}

