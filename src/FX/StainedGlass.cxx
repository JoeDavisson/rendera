/*
Copyright (c) 2025 Joe Davisson.

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
    CheckBox *sat_alpha;
    CheckBox *draw_edges;
    Fl_Button *ok;
    Fl_Button *cancel;
  }

  int isSegmentEdge(Bitmap *b, const int x, const int y)
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
  int size = Items::detail->value() + 5;

  size = std::pow(size, 2.5);

  KDtree::node_type test_node;
  KDtree::node_type *root, *found;
  std::vector<KDtree::node_type> points(size);
  int best_distance;

  for (int i = 0; i < size; i++)
  {
    points[i].x[0] = rnd() % bmp->w; 
    points[i].x[1] = rnd() % bmp->h; 
    points[i].x[2] = 0;
    points[i].index = bmp->getpixel(points[i].x[0], points[i].x[1]);
  }

  root = KDtree::build(&points[0], size, 0);

  Progress::show(bmp->h);

  // draw segments
  for (int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for (int x = bmp->cl; x <= bmp->cr; x++)
    {
      test_node.x[0] = x;
      test_node.x[1] = y;
      test_node.x[2] = 0;
      found = 0;
      KDtree::nearest(root, &test_node, &found, &best_distance, 0);

      if (found)
      {
        if (Items::sat_alpha->value())
        {
          rgba_type rgba = getRgba(found->index);

          int h, s, v;

          Blend::rgbToHsv(rgba.r, rgba.g, rgba.b, &h, &s, &v);
          *p = makeRgba(rgba.r, rgba.g, rgba.b, std::min(192, s / 2 + 128));
        }
          else
        {
          *p = found->index;
        }
      }

      p++;
    }

    if (Progress::update(y) < 0)
      return;
  }

  // draw edges
  if (found && Items::draw_edges->value())
  {
    Map *map = Project::map;
    map->clear(0);

    for (int y = bmp->ct; y <= bmp->cb; y++)
    {
      for (int x = bmp->cl; x <= bmp->cr; x++)
      {
        if (isSegmentEdge(bmp, x, y))
          map->setpixel(x, y, 1);
      }
    }

    for (int y = bmp->ct; y <= bmp->cb; y++)
    {
      int *p = bmp->row[y] + bmp->cl;

      for (int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int c = map->getpixel(x, y);

        if (c)
          *p = makeRgb(0, 0, 0);

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

  Items::detail = new InputInt(Items::dialog, 0, y1, 128, 32, "Detail (1-100)", 0, 1, 100);
  Items::detail->value(25);
  Items::detail->center();
  y1 += 32 + 16;

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

