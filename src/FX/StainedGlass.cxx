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

#include "StainedGlass.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *detail;
    InputInt *edge;
    CheckBox *uniform;
    CheckBox *sat_alpha;
    CheckBox *draw_edges;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  inline int isEdge(Bitmap *b, const int x, const int y, const int div)
  {
    const int c0 = getl(b->getpixel(x, y)) / div;
    const int c1 = getl(b->getpixel(x + 1, y)) / div;
    const int c2 = getl(b->getpixel(x, y + 1)) / div;
    const int c3 = getl(b->getpixel(x + 1, y + 1)) / div;

    if ((c0 == c1) && (c0 == c2) && (c0 == c3))
      return 0;
    else
      return 1;
  }

  inline int isSegmentEdge(Bitmap *b, const int x, const int y)
  {
    const int c0 = b->getpixel(x, y);
    const int c1 = b->getpixel(x + 1, y);
    const int c2 = b->getpixel(x, y + 1);
    const int c3 = b->getpixel(x + 1, y + 1);

    if ((c0 == c1) && (c0 == c2) && (c0 == c3))
      return 0;
    else
      return 1;
  }
}

void StainedGlass::apply()
{
  Bitmap *bmp = Project::bmp;
  int size = atoi(Items::detail->value());
  int div = atoi(Items::edge->value());

  std::vector<int> seedx(size);
  std::vector<int> seedy(size);
  std::vector<int> color(size);

  for (int i = 0; i < size; i++)
  {
    if (Items::uniform->value())
    {
      seedx[i] = rnd() % bmp->w; 
      seedy[i] = rnd() % bmp->h; 
    }
      else
    {
      seedx[i] = rnd() % bmp->w; 
      seedy[i] = rnd() % bmp->h; 

      int count = 0;

      do
      {
        seedx[i] = rnd() % bmp->w; 
        seedy[i] = rnd() % bmp->h; 
        count++;
      }
      while (!isEdge(bmp, seedx[i], seedy[i], div) && count < 10000);
    }

    color[i] = bmp->getpixel(seedx[i], seedy[i]);
  }

  Progress::show(bmp->h);

  // draw segments
  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      // find nearest color
      int nearest = 999999999;
      int use = -1;

      for (int i = 0; i < size; i++)
      {
        const int dx = x - seedx[i];
        const int dy = y - seedy[i];
        const int distance = dx * dx + dy * dy;

        if (distance < nearest)
        {
          nearest = distance;
          use = i;
        }
      }

      if (use != -1)
      {
        if (Items::sat_alpha->value())
        {
          rgba_type rgba = getRgba(color[use]);

          int h, s, v;

          Blend::rgbToHsv(rgba.r, rgba.g, rgba.b, &h, &s, &v);
          *p = makeRgba(rgba.r, rgba.g, rgba.b, std::min(192, s / 2 + 128));
        }
          else
        {
          *p = color[use];
        }
      }

      p++;
    }

    if (Progress::update(y) < 0)
      return;
  }

  // draw edges
  if (Items::draw_edges->value())
  {
    Map *map = Project::map;
    map->clear(0);
//      for (int y = bmp->ct * 4; y <= bmp->cb * 4; y++)
//      {
//        for (int x = bmp->cl * 4; x <= bmp->cr * 4; x++)
//        {
    for (int y = bmp->ct; y <= bmp->cb; y++)
    {
      for (int x = bmp->cl; x <= bmp->cr; x++)
      {
        if (isSegmentEdge(bmp, x, y))
          map->setpixel(x, y, 1);
//            map->setpixelAA(x, y, 255);
      }
    }

    for (int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for (int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = map->getpixel(x, y);

//          *p = Blend::trans(*p, makeRgb(0, 0, 0), 255 - c);
        if (c)
          *p = makeRgb(0, 0, 0);
//        *p = Blend::trans(*p, makeRgb(0, 0, 0), 160);

        p++;
      }
    }
  }

  Progress::hide();
}

void StainedGlass::close()
{
    Items::dialog->hide();
    Project::undo->push();
    apply();
}

void StainedGlass::quit()
{
  Progress::hide();
  Items::dialog->hide();
}

void StainedGlass::begin()
{
  Items::dialog->show();
}

void StainedGlass::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(400, 0, "Stained Glass");
  Items::detail = new InputInt(Items::dialog, 0, y1, 128, 32, "Detail (1-50000)", 0, 1, 50000);
  y1 += 32 + 16;
  Items::detail->value("5000");
  Items::detail->center();
  Items::edge = new InputInt(Items::dialog, 0, y1, 128, 32, "Edge Detect (1-50)", 0, 1, 50);
  y1 += 32 + 16;
  Items::edge->value("16");
  Items::edge->center();
  Items::uniform = new CheckBox(Items::dialog, 0, y1, 16, 16, "Uniform", 0);
  Items::uniform->center();
  y1 += 16 + 16;
  Items::sat_alpha = new CheckBox(Items::dialog, 0, y1, 16, 16, "Saturation to Alpha", 0);
  Items::sat_alpha->center();
  y1 += 16 + 16;
  Items::draw_edges = new CheckBox(Items::dialog, 0, y1, 16, 16, "Draw Edges", 0);
  Items::draw_edges->center();
  y1 += 16 + 16;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

