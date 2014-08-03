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

// callbacks are in fx_callback.cxx
FX::FX()
{
  rotate_hue = new Fl_Double_Window(256, 184, "Rotate Hue");
  rotate_hue_amount = new Field(rotate_hue, 120, 32, 72, 24, "Amount:", 0);
  rotate_hue_amount->maximum_size(4);
  rotate_hue_amount->value("60");
  rotate_hue_preserve = new Fl_Check_Button(48, 80, 16, 16, "Preserve Luminance");
  new Separator(rotate_hue, 16, 126, 226, 2, "");
  rotate_hue_ok = new Fl_Button(96, 144, 64, 24, "OK");
  rotate_hue_ok->callback((Fl_Callback *)hide_rotate_hue);
  rotate_hue_cancel = new Fl_Button(176, 144, 64, 24, "Cancel");
  rotate_hue_cancel->callback((Fl_Callback *)cancel_rotate_hue);
  rotate_hue->set_modal();
  rotate_hue->end();

  restore = new Fl_Double_Window(256, 200, "Restore");
  restore_normalize = new Fl_Check_Button(48, 32, 16, 16, "Normalize First");
  restore_invert = new Fl_Check_Button(48, 64, 16, 16, "Invert First");
  restore_correct = new Fl_Check_Button(48, 96, 16, 16, "Correction Matrix");
  new Separator(restore, 16, 142, 226, 2, "");
  restore_ok = new Fl_Button(96, 160, 64, 24, "OK");
  restore_ok->callback((Fl_Callback *)hide_restore);
  restore_cancel = new Fl_Button(176, 160, 64, 24, "Cancel");
  restore_cancel->callback((Fl_Callback *)cancel_restore);
  restore->set_modal();
  restore->end();

  remove_dust = new Fl_Double_Window(256, 144, "Remove Dust");
  remove_dust_amount = new Field(remove_dust, 120, 32, 72, 24, "Amount:", 0);
  remove_dust_amount->value("4");
  new Separator(remove_dust, 16, 88, 226, 2, "");
  remove_dust_ok = new Fl_Button(96, 104, 64, 24, "OK");
  remove_dust_ok->callback((Fl_Callback *)hide_remove_dust);
  remove_dust_cancel = new Fl_Button(176, 104, 64, 24, "Cancel");
  remove_dust_cancel->callback((Fl_Callback *)cancel_remove_dust);
  remove_dust->set_modal();
  remove_dust->end();
}

FX::~FX()
{
}

