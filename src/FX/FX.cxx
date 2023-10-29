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

#include <cmath>
#include <vector>

#include "FX.H"

// most filters may be used internally by calling Gui::progressEnable(false)
// first to disable the progress bar, then calling apply() with the target
// bitmap and other parameters

void FX::drawPreview(Bitmap *src, Bitmap *dest)
{
  if (src->w >= src->h)
  {
    float aspect = (float)src->h / src->w;
    int height = (float)dest->w * aspect;

    dest->clear(getFltkColor(FL_BACKGROUND2_COLOR));
    src->pointStretchSS(dest, 0, 0, src->w, src->h,
                     0, (dest->h - height) / 2, dest->w, height, false);
    dest->rect(0, (dest->h - height) / 2, dest->w,
              ((dest->h - height) / 2) + height, makeRgb(0, 0, 0), 0);
  }
    else
  {
    float aspect = (float)src->w / src->h;
    int width = (float)dest->h * aspect;

    dest->clear(getFltkColor(FL_BACKGROUND2_COLOR));
    src->pointStretchSS(dest, 0, 0, src->w, src->h,
                     (dest->w - width) / 2, 0, width, dest->w, false);
    dest->rect((dest->w - width) / 2, 0,
              ((dest->w - width) / 2) + width, dest->w, makeRgb(0, 0, 0), 0);
  }

  dest->rect(0, 0, dest->w - 1, dest->h - 1, makeRgb(0, 0, 0), 128);
}

void FX::init()
{
  // call init functions for filters with dialogs
  RotateHue::init();
  GaussianBlur::init();
  Sharpen::init();
  UnsharpMask::init();
  BoxFilters::init();
  Sobel::init();
  Bloom::init();
  Restore::init();
  RemoveDust::init();
  StainedGlass::init();
  Painting::init();
  Marble::init();
  Dither::init();
}

