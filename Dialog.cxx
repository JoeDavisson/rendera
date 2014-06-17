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

#include "rendera.h"

Dialog::Dialog()
{
  about = new Fl_Double_Window(336, 112, "About");
  Widget *logo = new Widget(about, 8, 8, 320, 64, "Logo", "data/logo_large.png", 0, 0);
  Fl_Button *ok = new Fl_Button(336 / 2 - 32, 80, 64, 24, "OK");
  ok->callback((Fl_Callback *)hide_about);
  about->set_modal();
  about->end(); 
}

Dialog::~Dialog()
{
}

