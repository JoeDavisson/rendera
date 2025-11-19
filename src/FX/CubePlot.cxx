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

#include "CubePlot.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    CheckBox *pal_colors;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void CubePlot::applyAll()
{
  Bitmap *bmp = Project::bmp;
  std::vector<float> histogram(16777216, 0);

  Project::newImage(512, 512);
  Gui::images->addFile("cube_plot");
  Project::undo->reset();

  for (int j = bmp->ct; j <= bmp->cb; j++)
  {
    for (int i = bmp->cl; i <= bmp->cr; i++)
    {
      rgba_type rgba = getRgba(bmp->getpixel(i, j));
      histogram[makeRgb24(rgba.r, rgba.g, rgba.b)] = 1;
    }
  }

  Progress::show(256);

  for (int b = 0; b < 256; b++)
  {
    for (int g = 0; g < 256; g++)
    {
      for (int r = 0; r < 256; r++)
      {
        int x = (r - g) * std::cos(30 * (M_PI / 180));
        int y = (r + g) * std::sin(30 * (M_PI / 180)) - b;
        int c = makeRgb(r, g, b);

        if (((r & 7) == 0) && ((g & 7) == 0) && ((b & 7) == 0))
          Project::bmp->setpixel(256 + x, 256 + y, c, 224);

        if (histogram[c & 0xffffff] == 1)
        {
          Project::bmp->setpixel(256 + x, 256 + y, c, 96);
        }
      }
    }

    Progress::update(b);
  }

  Progress::hide();
}

void CubePlot::applyPalette()
{
  Palette *pal = Project::palette;

  Project::newImage(512, 512);
  Gui::images->addFile("cube_plot");
  Project::undo->reset();

  for (int b = 0; b < 256; b++)
  {
    for (int g = 0; g < 256; g++)
    {
      for (int r = 0; r < 256; r++)
      {
        int x = (r - g) * std::cos(30 * (M_PI / 180));
        int y = (r + g) * std::sin(30 * (M_PI / 180)) - b;
        int c1 = makeRgb(r, g, b);
        int c2 = pal->data[pal->lookup(c1)];

        if (((r & 7) == 0) && ((g & 7) == 0) && ((b & 7) == 0))
          Project::bmp->setpixel(256 + x, 256 + y, c1, 224);

        int d = diff24(c1, c2);

        if (d < 8)
        {
          Project::bmp->setpixel(256 + x, 256 + y, c2, 96);
        }
      }
    }
  }
}

void CubePlot::close()
{
  Items::dialog->hide();

  if (Items::pal_colors->value())
    applyPalette();
  else
    applyAll();
}

void CubePlot::quit()
{
  Progress::hide();
  Items::dialog->hide();
}

void CubePlot::begin()
{
  Items::dialog->show();
}

void CubePlot::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(400, 0, "Cube Plot");

  Items::pal_colors = new CheckBox(Items::dialog, 0, y1, 16, 16, "Palette Colors", 0);
  y1 += 16 + 16;
  Items::pal_colors->center();

  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);

  Items::dialog->set_modal();
  Items::dialog->end();
}

