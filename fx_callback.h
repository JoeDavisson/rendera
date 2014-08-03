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

#ifndef PLUGIN_CALLBACK_H
#define PLUGIN_CALLBACK_H

#include "rendera.h"

void show_normalize();
void normalize();

void show_equalize();
void equalize();

void show_value_stretch();
void value_stretch();

void show_saturate();
void saturate();

void show_rotate_hue();
void hide_rotate_hue();
void rotate_hue(int);
void cancel_rotate_hue();

void show_invert();
void invert();

void show_restore();
void hide_restore();
void restore();
void cancel_restore();

void show_remove_dust();
void hide_remove_dust();
void remove_dust(int);
void cancel_remove_dust();

void show_colorize();
void colorize();

void show_correct();
void correct();

#endif

