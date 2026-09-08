/*
Copyright (c) 2026 Joe Davisson.

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

#ifndef PACKAGE_STRING
#  include "config.h"
#endif

#include <algorithm>

#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Help_View.H>

#include "DialogWindow.H"
#include "Help.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    Fl_Help_View *help_view;
    Fl_Button *ok;
  }
}

void Help::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(784, 0, "Rendera Manual");

  Items::help_view = new Fl_Help_View(8, 8, 768, 512, "");
  Items::help_view->color(fl_rgb_color(224, 224, 224));
  Items::help_view->textcolor(fl_rgb_color(0, 0, 0));
  Items::help_view->textsize(18);
  Items::help_view->textfont(FL_HELVETICA);
  Items::help_view->load("help_test.html");

  y1 += 512 + 16;

  Items::dialog->addOkButton(&Items::ok, &y1);
  Items::ok->callback((Fl_Callback *)hide);

  Items::dialog->set_modal();
  Items::dialog->end(); 
}

void Help::show()
{
  Items::help_view->load("help_test.html");
  Items::dialog->show();
}

void Help::hide()
{
  Items::dialog->hide();
}
