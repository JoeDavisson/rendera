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

#include "Bitmap.H"
#include "Clone.H"
#include "Dialog.H"
#include "Gui.H"
#include "Map.H"
#include "Project.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"

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

/*
void Undo::doPush(const int x, const int y, const int w, const int h)
{
  if (undo_current < 1)
  {
    undo_current = 1;

    Bitmap *temp_bmp = undo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      undo_stack[i] = undo_stack[i - 1];

    undo_stack[0] = temp_bmp;
  }

  if (Project::enoughMemory(Project::bmp->w, Project::bmp->h) == false)
    return;

  delete undo_stack[undo_current];
  undo_stack[undo_current] = new Bitmap(Project::bmp->w, Project::bmp->h);

  Project::bmp->blit(undo_stack[undo_current], 0, 0, 0, 0,
                     Project::bmp->w, Project::bmp->h);

  undo_current--;
}
*/

void Undo::doPush()
{
  const int x = 0;
  const int y = 0;
  const int w = Project::bmp->w;
  const int h = Project::bmp->h;

  doPush(x, y, w, h);
}

void Undo::doPush(const int x, const int y, const int w, const int h)
{
  if (undo_current < 1)
  {
    undo_current = 1;

    Bitmap *temp_bmp = undo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      undo_stack[i] = undo_stack[i - 1];

    undo_stack[0] = temp_bmp;
  }

  if (Project::enoughMemory(w, h) == false)
    return;

  delete undo_stack[undo_current];
  undo_stack[undo_current] = new Bitmap(w, h);
  undo_stack[undo_current]->x = x;
  undo_stack[undo_current]->y = y;

  Project::bmp->blit(undo_stack[undo_current], x, y, 0, 0, w, h);

  undo_current--;
}

void Undo::push()
{
  const int x = 0;
  const int y = 0;
  const int w = Project::bmp->w;
  const int h = Project::bmp->h;

  push(x, y, w, h);
}

void Undo::push(const int x, const int y, const int w, const int h)
{
  doPush(x, y, w, h);

  // reset redo list since user performed some action
  for (int i = 0; i < levels; i++)
  {
    delete redo_stack[i];
    redo_stack[i] = new Bitmap(8, 8);
  }

  redo_current = levels - 1;
}

void Undo::pop()
{
  if (undo_current >= levels - 1)
    return;

  int x = undo_stack[(undo_current + 1)]->x;
  int y = undo_stack[(undo_current + 1)]->y;
  int w = undo_stack[(undo_current + 1)]->w;
  int h = undo_stack[(undo_current + 1)]->h;

  pushRedo(x, y, w, h);

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

  if (x == 0 && y == 0)
    Project::replaceImage(w, h);

  undo_stack[undo_current]->blit(Project::bmp, 0, 0, x, y, w, h);
  Project::bmp->x = x;
  Project::bmp->y = x;
  Gui::getView()->drawMain(true);
}

void Undo::pushRedo(const int x, const int y, const int w, const int h)
{
  if (redo_current < 0)
  {
    redo_current = 0;

    Bitmap *temp_bmp = redo_stack[levels - 1];

    for (int i = levels - 1; i > 0; i--)
      redo_stack[i] = redo_stack[i - 1];

    redo_stack[0] = temp_bmp;
  }

  if (Project::enoughMemory(w, h) == false)
    return;

  delete redo_stack[redo_current];
  redo_stack[redo_current] = new Bitmap(w, h);
  redo_stack[redo_current]->x = x;
  redo_stack[redo_current]->y = y;

  Project::bmp->blit(redo_stack[redo_current], x, y, 0, 0, w, h);

  redo_current--;

  if (redo_current < 0)
    redo_current = 0;
}

void Undo::popRedo()
{
  if (redo_current >= levels - 1)
    return;

  int max = Project::undo_max;
  int x = redo_stack[(redo_current + 1) % max]->x;
  int y = redo_stack[(redo_current + 1) % max]->y;
  int w = redo_stack[(redo_current + 1) % max]->w;
  int h = redo_stack[(redo_current + 1) % max]->h;

  doPush(x, y, w, h);

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

  if (x == 0 && y == 0)
    Project::replaceImage(w, h);

  redo_stack[redo_current]->blit(Project::bmp, 0, 0, x, y, w, h);
  Project::bmp->x = x;
  Project::bmp->y = y;
  Gui::getView()->drawMain(true);
}

