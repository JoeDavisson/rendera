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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "rendera.h"

class Plugin
{
public:
  Plugin();
  virtual ~Plugin();

  Fl_Double_Window *rotate_hue;
  Field *rotate_hue_amount;
  Fl_Button *rotate_hue_ok;
  Fl_Button *rotate_hue_cancel;
};

#endif

