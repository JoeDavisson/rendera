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

#include "rendera.h"

class FX
{
public:
  static void init();

  static void showNormalize();
  static void doNormalize();

  static void showEqualize();
  static void doEqualize();

  static void showValueStretch();
  static void doValueStretch();

  static void showSaturate();
  static void doSaturate();

  static void showRotateHue();
  static void hideRotateHue();
  static void doRotateHue(int);
  static void cancelRotateHue();

  static void showInvert();
  static void doInvert();

  static void showRestore();
  static void hideRestore();
  static void doRestore();
  static void cancelRestore();

  static void showRemoveDust();
  static void hideRemoveDust();
  static void doRemoveDust(int);
  static void cancelRemoveDust();

  static void showColorize();
  static void doColorize();

  static void showCorrect();
  static void doCorrect();

  static Fl_Double_Window *rotate_hue;
  static Field *rotate_hue_amount;
  static Fl_Check_Button *rotate_hue_preserve;
  static Fl_Button *rotate_hue_ok;
  static Fl_Button *rotate_hue_cancel;

  static Fl_Double_Window *restore;
  static Fl_Check_Button *restore_normalize;
  static Fl_Check_Button *restore_invert;
  static Fl_Check_Button *restore_correct;
  static Fl_Button *restore_ok;
  static Fl_Button *restore_cancel;

  static Fl_Double_Window *remove_dust;
  static Field *remove_dust_amount;
  static Fl_Check_Button *remove_dust_invert;
  static Fl_Button *remove_dust_ok;
  static Fl_Button *remove_dust_cancel;
};

#endif

