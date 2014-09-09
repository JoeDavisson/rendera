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

#ifndef FX_H
#define FX_H

class Field;

#include "Rendera.H"

namespace FX
{
  void init();

  void showNormalize();
  void doNormalize();

  void showEqualize();
  void doEqualize();

  void showValueStretch();
  void doValueStretch();

  void showSaturate();
  void doSaturate();

  void showRotateHue();
  void hideRotateHue();
  void doRotateHue(int);
  void cancelRotateHue();

  void showInvert();
  void doInvert();

  void showRestore();
  void hideRestore();
  void doRestore();
  void cancelRestore();

  void showRemoveDust();
  void hideRemoveDust();
  void doRemoveDust(int);
  void cancelRemoveDust();

  void showColorize();
  void doColorize();

  void showCorrect();
  void doCorrect();

  void showApplyPalette();
  void hideApplyPalette();
  void doApplyPaletteNormal();
  void doApplyPaletteDither();
  void cancelApplyPalette();
}

#endif

