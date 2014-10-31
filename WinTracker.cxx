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

#include "Win.H"
#include "WinTracker.H"

#define MAX_WINDOWS 64

namespace
{
  Win *win_pointer[MAX_WINDOWS];
  int count = 0;
}

void WinTracker::add(Win *win)
{
  win_pointer[count++] = win;
}

void WinTracker::free()
{
  int i;

  for(i = 0; i < count; i++)
    if(win_pointer[i])
      delete win_pointer[i];
}

