/*
Copyright (c) 2024 Joe Davisson.

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

#include <cmath>
#include <vector>

#include <FL/fl_draw.H>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Choice.H>

#include "Bitmap.H"
#include "Blend.H"
#include "Brush.H"
#include "CheckBox.H"
#include "FilterMatrix.H"
#include "Dialog.H"
#include "DialogWindow.H"
#include "Editor.H"
#include "Gamma.H"
#include "Gui.H"
#include "Inline.H"
#include "InputFloat.H"
#include "InputInt.H"
#include "Map.H"
#include "Octree.H"
#include "Palette.H"
#include "Project.H"
#include "Quantize.H"
#include "Separator.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

#include "Normalize.H"
#include "Equalize.H"
#include "RotateHue.H"
#include "ValueStretch.H"
#include "Saturate.H"
#include "Desaturate.H"
#include "Colorize.H"
#include "PaletteColors.H"
#include "Invert.H"
#include "AlphaInvert.H"
#include "AlphaClear.H"
#include "AlphaColor.H"
#include "GaussianBlur.H"
#include "Sharpen.H"
#include "UnsharpMask.H"
#include "BoxFilters.H"
#include "Sobel.H"
#include "Bloom.H"
#include "Randomize.H"
#include "Restore.H"
#include "RemoveDust.H"
#include "StainedGlass.H"
#include "Painting.H"
#include "Marble.H"
#include "Dither.H"
#include "SideAbsorptions.H"
#include "Test.H"

class FX
{
public:
  static void drawPreview(Bitmap *, Bitmap *);
  static void init();

private:
  FX() { }
  ~FX() { }
};

#endif

