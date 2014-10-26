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

#include "Bitmap.H"
#include "Brush.H"
#include "Crop.H"
#include "GetColor.H"
#include "Map.H"
#include "Offset.H"
#include "Paint.H"
#include "Palette.H"
#include "Project.H"
#include "Stroke.H"
#include "Text.H"
#include "Tool.H"

namespace
{
  Paint *paint = 0;
  GetColor *getcolor = 0;
  Crop *crop = 0;
  Offset *offset = 0;
  Text *text = 0;
}

namespace Project
{
  Bitmap *bmp = 0;
  Map *map = 0;
  Brush *brush = 0;
  Palette *palette = 0;
  Tool *tool = 0;
  Stroke *stroke = 0;

  int overscroll = 64;
  int theme = THEME_DARK;
}

void Project::init()
{
  newImage(640, 480);

  bmp->wrap = 0;
  bmp->clone = 0;
  bmp->clone_moved = 0;
  bmp->clone_x = 0;
  bmp->clone_y = 0;
  bmp->clone_dx = 0;
  bmp->clone_dy = 0;
  bmp->clone_mirror = 0;

  brush = new Brush();
  palette = new Palette();
  stroke = new Stroke();

  // tools
  paint = new Paint();
  getcolor = new GetColor();
  crop = new Crop();
  offset = new Offset();
  text = new Text();

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
    case Tool::OFFSET:
      tool = offset; 
      break;
    case Tool::TEXT:
      tool = text; 
      break;
    default:
      tool = paint; 
      break;
  }
}

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

