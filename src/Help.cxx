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

#include <FL/Fl_Hold_Browser.H>
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
    Fl_Hold_Browser *index;
    Fl_Help_View *help_view;
    Fl_Button *ok;
  }

  const char *introduction =
    "<html>"
    "<body>"
    "Introduction"
    "<p>This is a work-in-progress.</p>"
    "</body>"
    "</html>";

  const char *basic_editing =
    "<html>"
    "<body>"
    "Basic Editing"
    "<p>Sample text.</p>"
    "</body>"
    "</html>";

  void index_cb()
  {
    switch (Items::index->value())
    {
      case 1:
        Items::help_view->value(introduction);
        break;
      case 2:
        Items::help_view->value(basic_editing);
        break;
    }
  }

}

void Help::init()
{
  int y1 = 16;

  Items::dialog = new DialogWindow(784 + 192 + 8, 0, "Rendera Manual");

  Items::index = new Fl_Hold_Browser(8, 8, 192, 512, "");
  Items::index->align(FL_ALIGN_TOP);
  Items::index->callback((Fl_Callback *)index_cb);
  Items::index->add("Introduction");
  Items::index->add("Basic Editing");
  Items::index->add("Photo Restoration");
  Items::index->add("Colorization");

  Items::help_view = new Fl_Help_View(8 + Items::index->w() + 8, 8, Items::dialog->w() - 24 - Items::index->w(), 512, "");
  Items::help_view->textsize(18);
  Items::help_view->textfont(FL_HELVETICA);
  Items::help_view->value(introduction);

  y1 += 512 + 8;

  Items::dialog->addOkButton(&Items::ok, &y1);
  Items::ok->callback((Fl_Callback *)hide);

  Items::dialog->set_modal();
  Items::dialog->end(); 
}

void Help::show()
{
  Items::dialog->show();
}

void Help::hide()
{
  Items::dialog->hide();
}
