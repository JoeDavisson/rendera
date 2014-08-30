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

#include "rendera.h"

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

  extern Fl_Double_Window *rotate_hue;
  extern Field *rotate_hue_amount;
  extern Fl_Check_Button *rotate_hue_preserve;
  extern Fl_Button *rotate_hue_ok;
  extern Fl_Button *rotate_hue_cancel;

  extern Fl_Double_Window *restore;
  extern Fl_Check_Button *restore_normalize;
  extern Fl_Check_Button *restore_invert;
  extern Fl_Check_Button *restore_correct;
  extern Fl_Button *restore_ok;
  extern Fl_Button *restore_cancel;

  extern Fl_Double_Window *remove_dust;
  extern Field *remove_dust_amount;
  extern Fl_Check_Button *remove_dust_invert;
  extern Fl_Button *remove_dust_ok;
  extern Fl_Button *remove_dust_cancel;

  extern Fl_Double_Window *apply_palette;
  extern Fl_Check_Button *apply_palette_dither;
  extern Fl_Button *apply_palette_ok;
  extern Fl_Button *apply_palette_cancel;
}

#endif

