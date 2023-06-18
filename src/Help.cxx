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

#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Help_View.H>

#include "Gui.H"
#include "Help.H"

Fl_Help_Dialog *Help::browse;

void Help::init()
{
  browse = new Fl_Help_Dialog();
  browse->textsize(14);
  browse->resize(Gui::getWindow()->x() + 64,
                 Gui::getWindow()->y() + 64,
                 640, 480);
  browse->load("./help/rendera.html");
}

void Help::show()
{
  browse->resize(Gui::getWindow()->x() + 64,
                 Gui::getWindow()->y() + 64,
                 browse->w(), browse->h());

  browse->show();
}

