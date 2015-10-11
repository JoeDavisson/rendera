/*
Copyright (c) 2015 Joe Davisson.

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

#include "Bitmap.H"
#include "Brush.H"
#include "Crop.H"
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

namespace Project
{
  Bitmap *bmp = 0;
  Map *map = 0;

  SP<Brush> brush = new Brush();
  SP<Palette> palette = new Palette();
  SP<Stroke> stroke = new Stroke();

  // tools
  Tool *tool = 0;
  SP<Paint> paint = new Paint();
  SP<GetColor> getcolor = new GetColor();
  SP<Crop> crop = new Crop();
  SP<Offset> offset = new Offset();
  SP<Text> text = new Text();
  SP<Fill> fill = new Fill();

  int overscroll = 64;
  int theme = THEME_DARK;
  char theme_path[PATH_MAX];
  int theme_highlight_color;
  Fl_Color fltk_theme_highlight_color; 
}

void Project::init()
{
  newImage(640, 480);
  setTool(Tool::PAINT);
}

void Project::setTool(int num)
{
  switch(num)
  {
    case Tool::PAINT:
      tool = paint.get(); 
      break;
    case Tool::GETCOLOR:
      tool = getcolor.get(); 
      break;
    case Tool::CROP:
      tool = crop.get(); 
      break;
    case Tool::OFFSET:
      tool = offset.get(); 
      break;
    case Tool::TEXT:
      tool = text.get(); 
      break;
    case Tool::FILL:
      tool = fill.get(); 
      break;
    default:
      tool = paint.get(); 
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

  Undo::init();
}

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

