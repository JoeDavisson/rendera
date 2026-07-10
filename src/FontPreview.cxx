/*
Copyright (c) 2026 Joe Davisson.

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

#include "CheckBox.H"
#include "DialogWindow.H"
#include "FontPreview.H"
#include "Gui.H"
#include "Separator.H"
#include "TextOptions.H"
#include "Tool.H"
#include "ToolOptions.H"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Tile.H>

class PreviewBox;

namespace
{
  DialogWindow *preview_win;
  Fl_Tile *vertical;
  PreviewBox *preview_text;
  Fl_Button *preview_done;
  Fl_Hold_Browser *font_browse;
}

class PreviewBox : public Fl_Box
{
public:
  PreviewBox(int x, int y, int w, int h, const char *label)
    : Fl_Box(x, y, w, h, label) { };
  ~PreviewBox() { }

  void resize(int x, int y, int w, int h)
  {
    Fl_Box::resize(x, y, w, h);

    int ww = 0, hh = 0;
    double size = 100;

    for (int i = 0; i < 100; i++)
    {
      labelsize(size);
      measure_label(ww, hh);

      if (ww < w - 16 && hh < h - 16)
        break;

      size *= 0.9;

      if (size < 20)
        break;
    }
  }
};

void FontPreview::init()
{
  int pos = 8;

  preview_win = new DialogWindow(628, 524, "Select Font");

  vertical = new Fl_Tile(8, pos, 612, 128 + 320);
  vertical->box(FL_DOWN_BOX);

  // add font names
  font_browse = new Fl_Hold_Browser(8, pos, 612, 128);
  font_browse->labelsize(18);
  font_browse->textsize(18);
  font_browse->scrollbar_size(24);
  font_browse->has_scrollbar(Fl_Browser_::VERTICAL);

  for (int i = 0; i < Fl::set_fonts("*"); i++)
  {
    const char *name = Fl::get_font_name((Fl_Font)i, 0);
    font_browse->add(name);
  }

  font_browse->value(1);
  font_browse->callback((Fl_Callback *)changedFont);
  pos += 128;

  preview_text = new PreviewBox(8, pos, 612, 320, "The quick brown\nfox jumps over\nthe lazy dog. ");
  preview_text->labelsize(55);
  preview_text->box(FL_DOWN_BOX);
  preview_text->align(FL_ALIGN_CENTER);

  vertical->size_range(font_browse, 612, 128);
  vertical->size_range(preview_text, 612, 128);
  vertical->resizable(font_browse);
  vertical->end();
  pos += 320 + 8;

  new Separator(preview_win, 0, pos, 628, Separator::HORIZONTAL, "");
  pos += 12;

  preview_done = new Fl_Button(628 - 96 - 8, pos, 96, 40, "Done (F)");
  preview_done->labelsize(16);
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
  Gui::text->changedSize();
  preview_win->hide();
}

void FontPreview::update(const int font)
{
  preview_text->labelfont(font);
  preview_text->resize(preview_text->x(), preview_text->y(),
                       preview_text->w(), preview_text->h());
  preview_text->redraw();
}

void FontPreview::changedFont()
{
  int font = FontPreview::getFont();

  FontPreview::update(font - 1);

  if (Gui::tools->getTool() == Tool::TEXT)
  {
    Gui::text->changedSize();
  }
}

int FontPreview::getFont()
{
  return font_browse->value();
}

