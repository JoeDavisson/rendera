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

#ifndef DIALOG_CALLBACK_H
#define DIALOG_CALLBACK_H

#include "rendera.h"

void show_about();
void hide_about();
void show_new_image();
void hide_new_image();
void cancel_new_image();
void show_create_palette();
void hide_create_palette();
void cancel_create_palette();
void show_load_palette();
void show_editor();
void hide_editor();
void do_editor_palette(Widget *, void *);
void do_editor_rgbhsv();
void do_editor_get_rgb();
void do_editor_get_hsv();

#endif

