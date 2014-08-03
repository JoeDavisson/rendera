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

extern FX *fx;

void show_rotate_hue()
{
  fx->rotate_hue->show();
}

void hide_rotate_hue()
{
  char s[8];

  int amount = atoi(fx->rotate_hue_amount->value());

  if(amount < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    fx->rotate_hue_amount->value(s);
    return;
  }

  if(amount > 359)
  {
    snprintf(s, sizeof(s), "%d", 359);
    fx->rotate_hue_amount->value(s);
    return;
  }

  fx->rotate_hue->hide();
}

void cancel_rotate_hue()
{
  fx->rotate_hue->hide();
}

