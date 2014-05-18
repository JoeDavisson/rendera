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

Stroke::Stroke()
{
  backbuf = new Map(Fl::w(), Fl::h());
}

Stroke::~Stroke()
{
}

void Stroke::make_blitrect(int x1, int y1, int x2, int y2)
{
  int s = (brush->size + 1) / 2 + 1;

  s *= gui->view->zoom;

  x1 -= gui->view->ox;
  y1 -= gui->view->oy;
  x2 -= gui->view->ox;
  y2 -= gui->view->oy;

  x1 *= gui->view->zoom;
  y1 *= gui->view->zoom;
  x2 *= gui->view->zoom;
  y2 *= gui->view->zoom;

  x1 += gui->view->x();
  y1 += gui->view->y();
  x2 += gui->view->x();
  y2 += gui->view->y();

  if(x2 < x1)
    SWAP(x1, x2);
  if(y2 < y1)
    SWAP(y1, y2);

  x1 -= s;
  y1 -= s;
  x2 += s;
  y2 += s;

  blitx = x1;
  blity = y1;
  blitw = x2 - x1;
  blith = y2 - y1;

  if(blitw < 1)
    blitw = 1;
  if(blith < 1)
    blith = 1;
}

void freehand()
{
}

void filledarea()
{
}

void line()
{
}

void polygon()
{
}

void rect()
{
}

void rectfill()
{
}

void oval()
{
}

void ovalfill()
{
}

