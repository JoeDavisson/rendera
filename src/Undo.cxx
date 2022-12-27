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

#include <vector>

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

  for(int i = 0; i < levels; i++)
  {
    undo_stack[i] = 0;
    redo_stack[i] = 0;
  }

  reset();
}

Undo::~Undo()
{
  for(int i = 0; i < levels; i++)
  {
    delete undo_stack[i];
    delete redo_stack[i];
  }
}

void Undo::reset()
{
  for(int i = 0; i < levels; i++)
  {
    delete undo_stack[i];
    delete redo_stack[i];

    undo_stack[i] = new Bitmap(8, 8);
    redo_stack[i] = new Bitmap(8, 8);
  }

  undo_current = levels - 1;
  redo_current = levels - 1;
}

void Undo::doPush()
{
  if(undo_current < 0)
  {
    undo_current = 0;

    Bitmap *temp_bmp = undo_stack[levels - 1];

    for(int i = levels - 1; i > 0; i--)
      undo_stack[i] = undo_stack[i - 1];

    undo_stack[0] = temp_bmp;
  }

  if(Project::enoughMemory(Project::bmp->w, Project::bmp->h) == false)
    return;

  delete undo_stack[undo_current];
  undo_stack[undo_current] = new Bitmap(Project::bmp->w, Project::bmp->h);

  Project::bmp->blit(undo_stack[undo_current], 0, 0, 0, 0,
                     Project::bmp->w, Project::bmp->h);

  undo_current--;
}

void Undo::push()
{
  doPush();

  // reset redo list since user performed some action
  for(int i = 0; i < levels; i++)
  {
    delete redo_stack[i];
    redo_stack[i] = new Bitmap(8, 8);
  }

  redo_current = levels - 1;
}

void Undo::pop()
{
  if(undo_current >= levels - 1)
  {
    Dialog::message("Undo", "No more undo levels.");
    return;
  }

  pushRedo();
  undo_current++;

  int w = undo_stack[undo_current]->w;
  int h = undo_stack[undo_current]->h;

  Project::replaceImage(w - Project::overscroll * 2, h - Project::overscroll * 2);

  undo_stack[undo_current]->blit(Project::bmp, 0, 0, 0, 0, w, h);

  Gui::getView()->ignore_tool = true;
  Gui::getView()->drawMain(true);
}

void Undo::pushRedo()
{
  if(redo_current < 0)
  {
    redo_current = 0;

    Bitmap *temp_bmp = redo_stack[levels - 1];

    for(int i = levels - 1; i > 0; i--)
      redo_stack[i] = redo_stack[i - 1];

    redo_stack[0] = temp_bmp;
  }

  if(Project::enoughMemory(Project::bmp->w, Project::bmp->h) == false)
    return;

  delete redo_stack[redo_current];
  redo_stack[redo_current] = new Bitmap(Project::bmp->w, Project::bmp->h);

  Project::bmp->blit(redo_stack[redo_current], 0, 0, 0, 0,
                     Project::bmp->w, Project::bmp->h);

  redo_current--;
}

void Undo::popRedo()
{
  if(redo_current >= levels - 1)
    return;

  doPush();
  redo_current++;

  int w = redo_stack[redo_current]->w;
  int h = redo_stack[redo_current]->h;

  Project::replaceImage(w - Project::overscroll * 2, h - Project::overscroll * 2);

  redo_stack[redo_current]->blit(Project::bmp, 0, 0, 0, 0, w, h);

  Gui::getView()->ignore_tool = true;
  Gui::getView()->drawMain(true);
}

