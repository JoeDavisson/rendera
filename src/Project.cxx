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

#include <climits>
#include <string>

#include <FL/Fl.H>

#include "Blend.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Dialog.H"
#include "Fill.H"
#include "GetColor.H"
#include "Inline.H"
#include "Map.H"
#include "Offset.H"
#include "Paint.H"
#include "Palette.H"
#include "Project.H"
#include "Selection.H"
#include "Stroke.H"
#include "Text.H"
#include "Tool.H"
#include "Undo.H"

Bitmap *Project::bmp;
Bitmap *Project::select_bmp;
Map *Project::map;

Brush *Project::brush;
Palette *Project::palette;
Stroke *Project::stroke;
Undo *Project::undo;

int Project::max_images;
Bitmap **Project::bmp_list;
Undo **Project::undo_list;
int Project::ox_list[256];
int Project::oy_list[256];
float Project::zoom_list[256];
int Project::current;
int Project::last;
int Project::mem_max;
int Project::undo_max;
  
Paint *Project::paint;
GetColor *Project::getcolor;
Selection *Project::selection;
Offset *Project::offset;
Text *Project::text;
Fill *Project::fill;

Tool *Project::tool;

int Project::theme;
int Project::theme_highlight_color;
Fl_Color Project::fltk_theme_highlight_color;
Fl_Color Project::fltk_theme_bevel_up;
Fl_Color Project::fltk_theme_bevel_down;

// called when the program starts
void Project::init(int memory_limit, int undo_limit)
{
  bmp = 0;
  map = 0;
  select_bmp = new Bitmap(8, 8);

  brush = new Brush();
  palette = new Palette();
  stroke = new Stroke();
  undo = 0;
  last = 0;

  paint = new Paint();
  getcolor = new GetColor();
  selection = new Selection();
  offset = new Offset();
  text = new Text();
  fill = new Fill();

  max_images = 256;
  mem_max = memory_limit;
  undo_max = undo_limit + 1;

  bmp_list = new Bitmap *[max_images];
  undo_list = new Undo *[max_images];

  for (int i = 0; i < max_images; i++)
  {
    bmp_list[i] = 0;
    undo_list[i] = 0;
    ox_list[i] = 0;
    oy_list[i] = 0;
    zoom_list[i] = 1;
  }

  newImage(512, 512);
  setTool(Tool::PAINT);
}

void Project::setTool(int num)
{
  switch (num)
  {
    case Tool::PAINT:
      tool = paint; 
      break;
    case Tool::GETCOLOR:
      tool = getcolor; 
      break;
    case Tool::SELECT:
      tool = selection; 
      break;
    case Tool::OFFSET:
      tool = offset; 
      break;
    case Tool::TEXT:
      tool = text; 
      break;
    case Tool::FILL:
      tool = fill; 
      break;
    default:
      tool = paint; 
      break;
  }
}

bool Project::enoughMemory(int w, int h)
{
  uint64_t data = w * h * sizeof(int);
  uint64_t row = h * sizeof(int *);

  if ((getImageMemory() + data + row) / 1000000 > mem_max)
  {
    Dialog::message("Error", "Memory limit reached: Close images or\nstart program with --mem option\nto increase limit.");
    return false;
  }
    else
  {
    return true;
  }
}

int Project::newImage(int w, int h)
{
  if (enoughMemory(w, h) == false)
    return -1;

  if (last > max_images - 2)
  {
    Dialog::message("Error", "Maximum number of images has been reached.");
    return -1;
  }

  delete bmp_list[last];
  delete undo_list[last];

  bmp_list[last] = new Bitmap(w, h);
  undo_list[last] = new Undo();
  bmp = bmp_list[last];
  undo = undo_list[last];
  current = last;
  last++;

  delete map;
  map = new Map(bmp->w, bmp->h);
  map->clear(0);

  return 0;
}

int Project::newImageFromBitmap(Bitmap *temp)
{
  if (enoughMemory(temp->w, temp->h) == false)
    return -1;

  if (last > max_images - 2)
  {
    Dialog::message("Error", "Maximum number of images has been reached.");
    return -1;
  }

  //puts("newImageFromBitmap()");
  //printf("last = %lu\n", (uint64_t)bmp_list[last]);

  delete bmp_list[last];
  bmp_list[last] = temp;

  delete undo_list[last];
  undo_list[last] = new Undo();
  bmp = bmp_list[last];
  undo = undo_list[last];
  current = last;
  last++;

  delete map;
  map = new Map(bmp->w, bmp->h);
  map->clear(0);

  return 0;
}

void Project::replaceImage(int w, int h)
{
  if (enoughMemory(w, h) == false)
    return;

  delete bmp_list[current];
  bmp_list[current] = new Bitmap(w, h);
  bmp = bmp_list[current];

  delete map;
  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

void Project::replaceImageFromBitmap(Bitmap *temp)
{
  if (enoughMemory(temp->w, temp->h) == false)
    return;

  delete bmp_list[current];
  bmp_list[current] = temp;
  bmp = bmp_list[current];

  delete map;
  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

void Project::resizeImage(int w, int h)
{
  if (enoughMemory(w, h) == false)
    return;

  Bitmap temp(w, h);
  bmp->blit(&temp, 0, 0, 0, 0, bmp->w, bmp->h);

  delete bmp_list[current];
  bmp_list[current] = new Bitmap(w, h);
  temp.blit(bmp_list[current], 0, 0, 0, 0, temp.w, temp.h);
  bmp = bmp_list[current];

  delete map;
  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

void Project::switchImage(int index)
{
  current = index;

  //puts("switchImage()");
  //printf("current = %d, last = %d\n", current, last);

  bmp = bmp_list[current];
  undo = undo_list[current];

  delete map;
  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

bool Project::removeImage()
{
  if (Dialog::choice("Close Image", "Are you sure?") == false)
    return false;

  if (last == 1)
  {
    delete bmp_list[0];
    delete undo_list[0];

    bmp_list[0] = new Bitmap(512, 512);
    undo_list[0] = new Undo();
    bmp = bmp_list[0];
    undo = undo_list[0];
    current = 0;
    last = 0;

    delete map;
    map = new Map(bmp->w, bmp->h);
    map->clear(0);
  }
    else
  {
    delete bmp_list[current];
    delete undo_list[current];

    for (int i = current; i < last; i++)
    {
      bmp_list[i] = bmp_list[i + 1];
      undo_list[i] = undo_list[i + 1];
    }

    delete bmp_list[last];
    delete undo_list[last];

    last--;
  }

  return true;
}

bool Project::swapImage(const int a, const int b)
{
  if (a < 0 || a >= last || b < 0 || b >= last)
    return -1;

  Bitmap *temp_bmp = Project::bmp_list[a];
  Project::bmp_list[a] = Project::bmp_list[b];
  Project::bmp_list[b] = temp_bmp;

  Undo *temp_undo = Project::undo_list[a];
  Project::undo_list[a] = Project::undo_list[b];
  Project::undo_list[b] = temp_undo;

  const int temp_x = ox_list[a];
  ox_list[a] = ox_list[b];
  ox_list[b] = temp_x;

  const int temp_y = oy_list[a];
  oy_list[a] = oy_list[b];
  oy_list[b] = temp_y;

  float temp_zoom = zoom_list[a];
  zoom_list[a] = zoom_list[b];
  zoom_list[b] = temp_zoom;

  current = b;

  return 0;
}

double Project::getImageMemory()
{
  double bytes = 0;

  for (int j = 0; j < last; j++)
  {
    Bitmap *temp = bmp_list[j];

    bytes += temp->w * temp->h * sizeof(int);
    bytes += temp->h * sizeof(int *);

    for (int i = 0; i < undo_list[j]->levels; i++)
    {
      temp = undo_list[j]->undo_stack[i];

      bytes += temp->w * temp->h * sizeof(int);
      bytes += temp->h * sizeof(int *);
    }

    for (int i = 0; i < undo_list[j]->levels; i++)
    {
      temp = undo_list[j]->redo_stack[i];

      bytes += temp->w * temp->h * sizeof(int);
      bytes += temp->h * sizeof(int *);
    }
  }

  return bytes;
}

void Project::pop()
{
  undo->pop();
}

void Project::popRedo()
{
  undo->popRedo();
}

