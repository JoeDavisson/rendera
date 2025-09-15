/*
Copyright (c) 2025 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
state WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "Button.H"
#include "Bitmap.H"
#include "CheckBox.H"
#include "Gui.H"
#include "Images.H"
#include "Project.H"
#include "RepeatButton.H"
#include "Selection.H"
#include "SelectionOptions.H"
#include "Separator.H"
#include "StaticText.H"
#include "Tool.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Group.H>

namespace
{
  StaticText *selection_x;
  StaticText *selection_y;
  StaticText *selection_w;
  StaticText *selection_h;
  Fl_Button *selection_reset;
  Fl_Button *selection_copy;
  CheckBox *selection_alpha;
  Button *selection_flip;
  Button *selection_mirror;
  Button *selection_rotate;
  Fl_Button *selection_paste;
  Fl_Button *selection_crop;

  void cb_selectAlpha(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectAlpha(); }

  void cb_selectCopy(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectCopy(); }

  void cb_selectPaste(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectPaste(); }

  void cb_selectCrop(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectCrop(); }

  void cb_selectFlipX(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectFlipX(); }

  void cb_selectFlipY(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectFlipY(); }

  void cb_selectReset(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectReset(); }

//  void cb_selectRotate180(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectRotate180(); }

  void cb_selectRotate90(Fl_Widget *w, void *data) { SelectionOptions *temp = (SelectionOptions *)data; temp->selectRotate90(); }
}

SelectionOptions::SelectionOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  new StaticText(this, 8, pos, 32, 32, "x:");
  selection_x = new StaticText(this, 32, pos, 96, 32, 0);
  pos += 24;

  new StaticText(this, 8, pos, 32, 32, "y:");
  selection_y = new StaticText(this, 32, pos, 96, 32, 0);
  pos += 24;

  new StaticText(this, 8, pos, 32, 32, "w:");
  selection_w = new StaticText(this, 32, pos, 96, 32, 0);
  pos += 24;

  new StaticText(this, 8, pos, 32, 32, "h:");
  selection_h = new StaticText(this, 32, pos, 96, 32, 0);
  pos += 24 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  selection_reset = new Fl_Button(this->x() + 8, this->y() + pos, 160, 48, "Reset");
  selection_reset->callback((Fl_Callback *)cb_selectReset);
  pos += 48 + Gui::SPACING;


  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  selection_alpha = new CheckBox(this, 8, pos, 16, 16, "Alpha Mask",
                                 (Fl_Callback *)cb_selectAlpha);
  selection_alpha->center();
  selection_alpha->value(1);
  pos += 24 + Gui::SPACING;

  selection_mirror = new Button(this, 8, pos, 48, 48, "Mirror", images_select_mirror_png, (Fl_Callback *)cb_selectFlipX);
  selection_flip = new Button(this, 8 + 48 + 8, pos, 48, 48, "Flip", images_select_flip_png, (Fl_Callback *)cb_selectFlipY);
  selection_rotate = new Button(this, 8 + 48 + 8 + 48 + 8, pos, 48, 48, "Rotate", images_select_rotate_png, (Fl_Callback *)cb_selectRotate90);
  pos += 48 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  selection_copy = new Fl_Button(this->x() + 8, this->y() + pos, 160, 40, "Copy");
  selection_copy->tooltip("Ctrl-C");
  selection_copy->callback((Fl_Callback *)cb_selectCopy);
  selection_copy->deactivate();
  pos += 40 + Gui::SPACING;

  selection_paste = new Fl_Button(this->x() + 8, this->y() + pos, 160, 40, "Paste");
  selection_paste->tooltip("Ctrl-V");
  selection_paste->callback((Fl_Callback *)cb_selectPaste);
  selection_paste->deactivate();
  pos += 40 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  selection_crop = new Fl_Button(this->x() + 8, this->y() + pos, 160, 48, "Crop");
  selection_crop->callback((Fl_Callback *)cb_selectCrop);
  selection_crop->deactivate();
  pos += 48 + Gui::SPACING;

  selectValues(0, 0, 0, 0);

  resizable(0);
  end();
}

SelectionOptions::~SelectionOptions()
{
}

void SelectionOptions::selectCopy()
{
  Project::tool->done(Gui::Gui::view, 0);
}

void SelectionOptions::selectCopyEnable(bool enable)
{
  if (enable == true)
    selection_copy->activate();
  else
    selection_copy->deactivate();

  selection_copy->redraw();
}

void SelectionOptions::selectPaste()
{
  Project::tool->done(Gui::Gui::view, 2);
}

void SelectionOptions::selectPasteEnable(bool enable)
{
  if (enable == true)
    selection_paste->activate();
  else
    selection_paste->deactivate();

  selection_paste->redraw();
}

void SelectionOptions::selectAlpha()
{
  Project::selection->redraw(Gui::view);
}

void SelectionOptions::selectCrop()
{
  Project::tool->done(Gui::view, 1);
}

void SelectionOptions::selectCropEnable(bool enable)
{
  if (enable == true)
    selection_crop->activate();
  else
    selection_crop->deactivate();

  selection_crop->redraw();
}

void SelectionOptions::selectFlipX()
{
  Project::select_bmp->flipHorizontal();
  Project::selection->redraw(Gui::view);
}

void SelectionOptions::selectFlipY()
{
  Project::select_bmp->flipVertical();
  Project::selection->redraw(Gui::view);
}

void SelectionOptions::selectRotate90()
{
  int w = Project::select_bmp->w;
  int h = Project::select_bmp->h;

  // make copy
  Bitmap temp(w, h);
  Project::select_bmp->blit(&temp, 0, 0, 0, 0, w, h);

  // create rotated image
  delete Project::select_bmp;
  Project::select_bmp = new Bitmap(h, w);

  int *p = &temp.data[0];

  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
       *(Project::select_bmp->row[x] + h - 1 - y) = *p++;
    }
  }

  Project::selection->reload();
  Project::selection->redraw(Gui::view);
}

void SelectionOptions::selectRotate180()
{
  Project::select_bmp->rotate180();
  Project::selection->redraw(Gui::view);
}

void SelectionOptions::selectReset()
{
  Project::tool->reset();
  Project::selection->redraw(Gui::view);
}

void SelectionOptions::selectValues(int x, int y, int w, int h)
{
  char s[256];

  snprintf(s, sizeof(s), "%d", x);
  selection_x->copy_label(s);
  selection_x->redraw();

  snprintf(s, sizeof(s), "%d", y);
  selection_y->copy_label(s);
  selection_y->redraw();

  snprintf(s, sizeof(s), "%d", w);
  selection_w->copy_label(s);
  selection_w->redraw();

  snprintf(s, sizeof(s), "%d", h);
  selection_h->copy_label(s);
  selection_h->redraw();
}

int SelectionOptions::selectGetAlpha()
{
  return selection_alpha->value();
}

