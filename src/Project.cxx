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

#include <climits>
#include <string>

#include <FL/Fl.H>

#include "Blend.H"
#include "Bitmap.H"
#include "Brush.H"
#include "Crop.H"
#include "Dialog.H"
#include "Selection.H"
#include "Fill.H"
#include "GetColor.H"
#include "Inline.H"
#include "Map.H"
#include "Offset.H"
#include "Paint.H"
#include "Palette.H"
#include "Project.H"
#include "Stroke.H"
#include "Text.H"
#include "Tool.H"
#include "Undo.H"

// container for commonly-used objects and related functions
namespace Project
{
  Bitmap *bmp = 0;
  Bitmap *select_bmp = new Bitmap(8, 8);
  int select_trans = 0;
  Map *map = 0;

  Brush *brush = new Brush();
  Palette *palette = new Palette();
  Stroke *stroke = new Stroke();
  Undo *undo = 0;

  Bitmap **bmp_list;
  Undo **undo_list;
  const int max_images = 256;
  int last = 0;
  int current = 0;

  // tools
  Tool *tool = 0;

  Paint *paint = new Paint();
  GetColor *getcolor = new GetColor();
  Crop *crop = new Crop();
  Selection *selection = new Selection();
  Offset *offset = new Offset();
  Text *text = new Text();
  Fill *fill = new Fill();

  int overscroll = 64;
  int theme = THEME_DARK;
  char theme_path[PATH_MAX];
  int theme_highlight_color;
  Fl_Color fltk_theme_highlight_color; 
  Fl_Color fltk_theme_bevel_up; 
  Fl_Color fltk_theme_bevel_down; 
}

// called when the program starts
void Project::init()
{
  bmp_list = new Bitmap *[max_images];
  undo_list = new Undo *[max_images];

  for(int i = 0; i < max_images; i++)
  {
    bmp_list[i] = 0;
    undo_list[i] = 0;
  }

  newImage(512, 512);

  setTool(Tool::PAINT);
}

void Project::setTool(int num)
{
  switch(num)
  {
    case Tool::PAINT:
      tool = paint; 
      break;
    case Tool::GETCOLOR:
      tool = getcolor; 
      break;
    case Tool::CROP:
      tool = crop; 
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

int Project::newImage(int w, int h)
{
  if(last > max_images - 1)
  {
    Dialog::message("Error", "Maximum number of images\nhas been reached.");
    return -1;
  }

  if(bmp_list[last])
    delete bmp_list[last];

  if(undo_list[last])
    delete undo_list[last];

  bmp_list[last] = new Bitmap(w, h, overscroll);
  undo_list[last] = new Undo();
  bmp = bmp_list[last];
  undo = undo_list[last];
  current = last;
  last++;

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);

  return 0;
}

int Project::newImageFromBitmap(Bitmap *temp)
{
  if(last > max_images - 1)
  {
    Dialog::message("Error", "Maximum number of images\nhas been reached.");
    return -1;
  }

  if(bmp_list[last])
    delete bmp_list[last];

  bmp_list[last] = temp;

  if(undo_list[last])
    delete undo_list[last];

  undo_list[last] = new Undo();
  bmp = bmp_list[last];
  undo = undo_list[last];
  current = last;
  last++;

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);

  return 0;
}

void Project::replaceImage(int w, int h)
{
  if(bmp_list[current])
    delete bmp_list[current];

  bmp_list[current] = new Bitmap(w, h, overscroll);
  bmp = bmp_list[current];

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

void Project::replaceImageFromBitmap(Bitmap *temp)
{
  if(bmp_list[current])
    delete bmp_list[current];

  bmp_list[current] = temp;
  bmp = bmp_list[current];

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

/*
void Project::newImage(int w, int h)
{
  if(bmp)
    delete bmp;

  bmp = new Bitmap(w, h, overscroll);

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}
*/

void Project::resizeImage(int w, int h)
{
  Bitmap *temp = new Bitmap(w, h, overscroll);
  bmp->blit(temp, overscroll, overscroll, overscroll, overscroll,
            bmp->cw, bmp->ch);

  if(bmp)
    delete bmp;

  bmp = temp;

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);
}

void Project::switchImage(int index)
{
  bmp = bmp_list[index];
  undo = undo_list[index];

  if(map)
    delete map;

  map = new Map(bmp->w, bmp->h);
  map->clear(0);

  current = index;
}

bool Project::removeImage()
{
  if(last == 1)
  {
    Dialog::message("Error", "Last image, nothing to do.");
    return false;
  }

  if(Dialog::choice("Close", "Are you sure?") == false)
    return false;

  for(int i = current; i < last; i++)
  {
    bmp_list[i] = bmp_list[i + 1];
    undo_list[i] = undo_list[i + 1];
  }

  last--;

  return true;
}

void Project::pop()
{
  undo->pop();
}

void Project::popRedo()
{
  undo->popRedo();
}

