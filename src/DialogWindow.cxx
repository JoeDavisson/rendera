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

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>

#include "DialogWindow.H"
#include "Gui.H"
#include "Separator.H"

DialogWindow::DialogWindow(int w, int h, const char *l)
: Fl_Double_Window(w, h, l)
{
  first_show = true;
}

DialogWindow::~DialogWindow()
{
}

void DialogWindow::addOkButton(Fl_Button **ok, int *y1)
{
  new Separator(this, 4, *y1, w() - 8, 2, "");
  *y1 += 8;
  *ok = new Fl_Button(w() - 64 - 8, *y1, 64, 24, "OK");
  add(*ok);
  *y1 += 24 + 8;

  resize(x(), y(), w(), *y1);
}

void DialogWindow::addOkCancelButtons(Fl_Button **ok, Fl_Button **cancel, int *y1)
{
  new Separator(this, 4, *y1, w() - 8, 2, "");
  *y1 += 8;
  *cancel = new Fl_Button(w() - 64 - 8, *y1, 64, 24, "Cancel");
  add(*cancel);
  *ok = new Fl_Button((*cancel)->x() - 64 - 8, *y1, 64, 24, "Ok");
  *y1 += 24 + 8;
  add(*ok);
  resize(x(), y(), w(), *y1);
}

int DialogWindow::handle(int event)
{
  switch(event)
  {
    case FL_PUSH:
      xpos = x();
      ypos = y();
      break;
  }

  return Fl_Double_Window::handle(event);
}

void DialogWindow::show()
{
  if(first_show == true)
  {
    xpos = Gui::getWindow()->x() + (Gui::getWindow()->w() - w()) / 2;
    ypos = Gui::getWindow()->y() + (Gui::getWindow()->h() - h()) / 2;
    first_show = false;
  }

  resize(xpos, ypos, w(), h());
  Fl_Double_Window::show();
}

void DialogWindow::hide()
{
  xpos = x();
  ypos = y();

  Fl_Double_Window::hide();
}

