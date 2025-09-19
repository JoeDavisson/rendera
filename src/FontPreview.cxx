/*
Copyright (c) 2025 Joe Davisson.

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

#include "DialogWindow.H"
#include "FontPreview.H"
#include "Separator.H"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

namespace
{
  DialogWindow *preview_win;
  Fl_Box *preview_text;
  Fl_Button *preview_done;
}

void FontPreview::init()
{
  int pos = 8;

  preview_win = new DialogWindow(528, 460, "Font Preview");
  preview_text = new Fl_Box(8, 8, 512, 384, "The quick brown\nfox jumps over\nthe lazy dog. ");
  preview_text->box(FL_DOWN_BOX);
  preview_text->align(FL_ALIGN_CENTER);
  preview_text->labelsize(48);

  pos += 384 + 8;

  new Separator(preview_win, 0, pos, 528, Separator::HORIZONTAL, "");
  pos += 12;

  preview_done = new Fl_Button(424, pos, 96, 40, "Done (F)");
  preview_done->shortcut('f');
  preview_done->callback((Fl_Callback *)FontPreview::close);
  preview_win->set_non_modal();
  preview_win->end();
}

void FontPreview::toggle()
{
  if (preview_win->shown() == 0)
    preview_win->show();
  else
    preview_win->hide();
}

void FontPreview::close()
{
  preview_win->hide();
}

void FontPreview::update(const int font)
{
  preview_text->labelfont(font);
  preview_text->redraw();
}

