/*
Copyright (c) 2025 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
state WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <cstdio>

#include "FL/Fl_Box.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Group.H"
#include "FL/Fl_Progress.H"

#include "Gui.H"
#include "Progress.H"
#include "View.H"

// hack to externally enable/disable progress indicator
// allows filters to be used internally
bool Progress::active = true;
float Progress::value = 0;
float Progress::step = 0;
int Progress::interval = 100;

void Progress::enable(bool state)
{
  active = state;
}

void Progress::hide()
{
  if (active == false)
    return;

  Gui::view->drawMain(true);
  Gui::progress->value(0);
  Gui::progress->copy_label("");
  Gui::progress->redraw();
  Gui::progress->hide();
  Gui::info->show();
  Gui::view->rendering = false;
}

// use default interval
void Progress::show(float max)
{
  if (active == false)
    return;

  if (max == 0)
    max = .001;

  Gui::view->rendering = true;
  value = 0;
  interval = max / 10;
  step = 100.0 / (max / interval);
  // keep progress bar on right side in case window was resized
  Gui::progress->resize(Gui::getStatus()->x() + Gui::getWindow()->w() - 256 - 8, Gui::getStatus()->y() + 4, 256, 24);
  Gui::info->hide();
  Gui::progress->show();
}

// custom interval
void Progress::show(float max, int new_interval)
{
  if (active == false)
    return;

  if (max == 0)
    max = .001;

  if (new_interval < 1)
     new_interval = 1;

  Gui::view->rendering = true;
  value = 0;
  interval = new_interval;
  step = 100.0 / (max / new_interval);
  // keep progress bar on right side in case window was resized
  Gui::progress->resize(Gui::getStatus()->x() + Gui::getWindow()->w() - 256 - 8,
                        Gui::getStatus()->y() + 4, 256, 24);
  Gui::info->hide();
  Gui::progress->show();
}

int Progress::update(int y)
{
  if (active == false)
    return 0;

  // user cancelled operation
  if (Fl::get_key(FL_Escape))
  {
    hide();
    Gui::view->drawMain(true);
    return -1;
  }

  if (!(y % interval))
  {
    Gui::progress->value(value);
    char percent[16];
    snprintf(percent, sizeof(percent), "%d%%", (int)value);
    Gui::progress->copy_label(percent);
    value += step;
    Gui::view->drawMain(true);
  }

  return 0;
}

