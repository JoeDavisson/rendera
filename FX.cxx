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

// callbacks are in plugin_callback.cxx
FX::FX()
{
  rotate_hue = new Fl_Double_Window(256, 144, "Rotate Hue");
  rotate_hue_amount = new Field(rotate_hue, 120, 32, 72, 24, "Amount:", 0);
  rotate_hue_amount->value("60");
  new Separator(rotate_hue, 16, 88, 226, 2, "");
  rotate_hue_ok = new Fl_Button(96, 104, 64, 24, "OK");
  rotate_hue_ok->callback((Fl_Callback *)hide_rotate_hue);
  rotate_hue_cancel = new Fl_Button(176, 104, 64, 24, "Cancel");
  rotate_hue_cancel->callback((Fl_Callback *)cancel_rotate_hue);
  rotate_hue->set_modal();
  rotate_hue->end();
}

FX::~FX()
{
}

