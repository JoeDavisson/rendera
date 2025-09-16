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
#include "ImagesOptions.H"
#include "Inline.H"
#include "Palette.H"
#include "Project.H"
#include "Separator.H"
#include "View.H"
#include "Widget.H"

#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>

namespace
{
  Fl_Hold_Browser *images_browse;
  Fl_Input *images_rename;
  Button *images_close;
  Button *images_move_up;
  Button *images_move_down;
  Fl_Box *images_mem;

  void cb_browse(Fl_Widget *w, void *data) { ImagesOptions *temp = (ImagesOptions *)data; temp->browse(); }

  void cb_closeFile(Fl_Widget *w, void *data) { ImagesOptions *temp = (ImagesOptions *)data; temp->closeFile(); }

  void cb_moveUp(Fl_Widget *w, void *data) { ImagesOptions *temp = (ImagesOptions *)data; temp->moveUp(); }

  void cb_moveDown(Fl_Widget *w, void *data) { ImagesOptions *temp = (ImagesOptions *)data; temp->moveDown(); }

  void cb_rename(Fl_Widget *w, void *data) { ImagesOptions *temp = (ImagesOptions *)data; temp->rename(); }
}

ImagesOptions::ImagesOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)                     
{
  int pos = Group::title_height + Gui::SPACING;

  pos = Group::title_height + Gui::SPACING;

  images_browse = new Fl_Hold_Browser(8, pos, 160, 256);
  images_browse->textsize(14);
  images_browse->resize(this->x() + 8, this->y() + pos, 160, 384);
  images_browse->callback((Fl_Callback *)cb_browse);

  pos += images_browse->h() + Gui::SPACING;

  images_close = new Button(this, 8, pos, 48, 48,
                          "Close File (Delete)", images_close_png,
                          (Fl_Callback *)cb_closeFile);

  images_move_up = new Button(this, 8 + 48 + 8, pos, 48, 48,
                            "Move Up", images_up_large_png,
                            (Fl_Callback *)cb_moveUp);

  images_move_down = new Button(this, 8 + 48 + 8 + 48 + 8, pos,
                              48, 48,
                              "Move Down", images_down_large_png,
                              (Fl_Callback *)cb_moveDown);

  pos += 48 + Gui::SPACING;

  images_rename = new Fl_Input(8, pos, 160, 32, "");
  images_rename->value("");
  images_rename->when(FL_WHEN_ENTER_KEY);
  images_rename->resize(this->x() + 8, this->y() + pos, 160, 32);
  images_rename->callback((Fl_Callback *)cb_rename);
  pos += 32 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::IMAGES_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  images_mem = new Fl_Box(FL_FLAT_BOX,
                        this->x() + 8, this->y() + pos, 160, 64, "");

  images_mem->labelsize(14);
  images_mem->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);

  resizable(0);
  end();
}

ImagesOptions::~ImagesOptions()
{
}

void ImagesOptions::browse()
{
  const int line = images_browse->value();

  if (line > 0)
  {                       
    Project::switchImage(line - 1);

    // restore coords/zoom saved during navigation
    Gui::view->ox = Project::ox_list[Project::current];
    Gui::view->oy = Project::oy_list[Project::current];
    Gui::view->zoom = Project::zoom_list[Project::current];
    Gui::view->drawMain(true);
                              
    images_rename->value(images_browse->text(line));
  }                
}
  
void ImagesOptions::rename()
{ 
  const int line = images_browse->value();
  
  if (line > 0)
  {
    images_browse->text(line, images_rename->value());
    images_browse->redraw();
  }
}

void ImagesOptions::addFile(const char *name)
{
  images_browse->add(name, 0);
  images_browse->select(Project::current + 1);
  browse();
}

void ImagesOptions::closeFile()
{
  if (Project::removeImage() == false)
    return;

  images_browse->remove(Project::current + 1);

  if (Project::current > 0)
    images_browse->select(Project::current, 1);
  else
    images_browse->select(Project::current + 1, 1);

  if (Project::current > 0)
    Project::switchImage(Project::current - 1);
  else
    Project::switchImage(Project::current);

  images_rename->value(images_browse->text(images_browse->value()));

  if (Project::last == 0)
  {
    addFile("new");
    Project::last = 1;
  }

  // restore coords/zoom saved during navigation
  Gui::view->ox = Project::ox_list[Project::current];
  Gui::view->oy = Project::oy_list[Project::current];
  Gui::view->zoom = Project::zoom_list[Project::current];
  Gui::view->drawMain(true);
}

void ImagesOptions::duplicate()
{
  const int current = Project::current;
  const int last = Project::last;
  Bitmap **bmp_list = Project::bmp_list;
  Bitmap *bmp = Project::bmp_list[current];

  if (Project::enoughMemory(bmp->w, bmp->h) == false)
    return;

  Project::newImage(bmp->cw, bmp->ch);
  bmp_list[current]->blit(bmp_list[last], 0, 0, 0, 0, bmp->w, bmp->h);

  addFile("new");
}

void ImagesOptions::moveUp()
{
  int temp = Project::current;

  if (Project::swapImage(temp, temp - 1) == true)
  {
    images_browse->swap(temp + 1, temp); 
    images_browse->select(temp);
    browse();
  }
}

void ImagesOptions::moveDown()
{
  int temp = Project::current;

  if (Project::swapImage(temp, temp + 1) == true)
  {
    images_browse->swap(temp + 1, temp + 2); 
    images_browse->select(temp + 2);
    browse();
  }
}

void ImagesOptions::memLabel(const char *s)
{
  images_mem->copy_label(s);
}

