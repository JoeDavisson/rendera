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

#include "Bitmap.H"
#include "Brush.H"
#include "Group.H"
#include "Gui.H"
#include "Images.H"
#include "Inline.H"
#include "InputInt.H"
#include "PaintOptions.H"
#include "Picker.H"
#include "Project.H"
#include "Render.H"
#include "Separator.H"
#include "Stroke.H"
#include "Widget.H"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

namespace
{
  const int brush_sizes[16] =
  {
    1, 2, 3, 4, 6, 8, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100
  };

  void cb_sizeValue(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->sizeValue(); }

  void cb_size(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->size(); }

  void cb_shape(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->shape(); }

  void cb_stroke(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->stroke(); }

  void cb_mode(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->mode(); }

  void cb_coarseEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->coarseEdge(); }

  void cb_fineEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->fineEdge(); }

  void cb_blurryEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->blurryEdge(); }

  void cb_watercolorEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->watercolorEdge(); }

  void cb_chalkEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->chalkEdge(); }

  void cb_textureEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->textureEdge(); }

  void cb_textureMarb(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->textureMarb(); }

  void cb_textureTurb(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->textureTurb(); }

  void cb_averageEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->averageEdge(); }
}

PaintOptions::PaintOptions(int x, int y, int w, int h, const char *l)
: Group(x, y, w, h, l)
{
  int pos = Group::title_height + Gui::SPACING;

  paint_brush_preview = new Widget(this, 8, pos, 160, 160,
                           "Brush Preview", 0, 0, 0);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  pos += 160 + 8;

  paint_size_value = new InputInt(this, 8, pos, 160, 32,
                       "", 0, 1, Brush::max_size);
  paint_size_value->textsize(16);
  paint_size_value->callback(cb_sizeValue, (void *)this);
  pos += 32 + 8;

  paint_size = new Widget(this, 8, pos, 160, 32,
                          "Brush Size", images_size_png, 10, 32,
                          0);
  paint_size->callback(cb_size, (void *)this);
  pos += 32 + 8;

  paint_shape = new Widget(this, 8, pos, 160, 40,
                           "Shape Adjust", images_shape_png, 10, 40,
                           0);
  paint_shape->callback(cb_shape, (void *)this);
  pos += 40 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  paint_stroke = new Widget(this, 8, pos, 160, 80,
                            "Brushstroke Type", images_stroke_png, 40, 40,
                            0);
  paint_stroke->callback(cb_stroke, (void *)this);

  pos += 80 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  paint_mode = new Fl_Choice(8, pos, 160, 32, "");
  paint_mode->tooltip("Rendering Mode");
  paint_mode->textsize(10);
  paint_mode->resize(this->x() + 8, this->y() + pos, 160, 32);
  paint_mode->add("Solid");
  paint_mode->add("Antialiased");
  paint_mode->add("Coarse");
  paint_mode->add("Fine");
  paint_mode->add("Blur");
  paint_mode->add("Watercolor");
  paint_mode->add("Chalk");
  paint_mode->add("Texture");
  paint_mode->add("Average");
  paint_mode->value(0);
  paint_mode->textsize(16);
  paint_mode->callback(cb_mode, (void *)this);
  pos += 32 + 8;

  paint_coarse_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32, 0);
  paint_coarse_edge->callback(cb_coarseEdge, (void *)this);

  paint_fine_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32, 0);
  paint_fine_edge->callback(cb_fineEdge, (void *)this);

  paint_blurry_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32, 0);
  paint_blurry_edge->callback(cb_blurryEdge, (void *)this);

  paint_watercolor_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_watercolor_edge_png, 20, 32, 0);
  paint_watercolor_edge->callback(cb_watercolorEdge, (void *)this);

  paint_chalk_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_chalk_edge_png, 20, 32, 0);
  paint_chalk_edge->callback(cb_chalkEdge, (void *)this);

  paint_texture_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32, 0);
  paint_texture_edge->callback(cb_textureEdge, (void *)this);

  paint_texture_marb = new Widget(this, 8, pos + 40, 160, 32,
                          "Marbleize", images_marbleize_png, 20, 32, 0);
  paint_texture_marb->callback(cb_textureMarb, (void *)this);

  paint_texture_turb = new Widget(this, 8, pos + 80, 160, 32,
                          "Turbulence", images_turbulence_png, 20, 32, 0);
  paint_texture_turb->callback(cb_textureTurb, (void *)this);

  paint_average_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32, 0);
  paint_average_edge->callback(cb_averageEdge, (void *)this);

  resizable(0);
  end();

  mode();

  paint_size->var = 0;
  paint_size->do_callback();
  paint_coarse_edge->var = 3;
  paint_coarse_edge->do_callback();
  paint_fine_edge->var = 3;
  paint_fine_edge->do_callback();
  paint_watercolor_edge->var = 3;
  paint_watercolor_edge->do_callback();
  paint_chalk_edge->var = 2;
  paint_chalk_edge->do_callback();
  paint_average_edge->var = 2;
  paint_average_edge->do_callback();
}

PaintOptions::~PaintOptions()
{
}

void PaintOptions::mode()
{
  Project::brush->aa = 0;
  paint_coarse_edge->hide();
  paint_fine_edge->hide();
  paint_blurry_edge->hide();
  paint_watercolor_edge->hide();
  paint_chalk_edge->hide();
  paint_texture_edge->hide();
  paint_texture_marb->hide();
  paint_texture_turb->hide();
  paint_average_edge->hide();
    
  switch (paint_mode->value())
  {
    case Render::SOLID:
      break;
    case Render::ANTIALIASED:
      Project::brush->aa = 1;
      break;
    case Render::COARSE:
      paint_coarse_edge->show();
      break;
    case Render::FINE:
      paint_fine_edge->show();
      break;
    case Render::BLURRY:
      paint_blurry_edge->show();
      break;
    case Render::WATERCOLOR:
      paint_watercolor_edge->show();
      break;
    case Render::CHALK:
      paint_chalk_edge->show();
      break;
    case Render::TEXTURE:
      paint_texture_edge->show();
      paint_texture_marb->show();
      paint_texture_turb->show();
      break;
    case Render::AVERAGE:
      paint_average_edge->show();
      break;
  }
}

void PaintOptions::changeSize(int new_size)
{
  Brush *brush = Project::brush;
  float round = (float)(15 - paint_shape->var) / 15;

  brush->make(new_size, round);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);

  const double aspect = 128.0 / new_size;

  for (int i = 0; i < brush->solid_count; i++)
  {
    int temp_x = brush->solidx[i];
    int temp_y = brush->solidy[i];

    if (new_size > 128)
    {
      temp_x *= aspect;
      temp_y *= aspect;
    }

    temp_x += paint_brush_preview->w() / 2;
    temp_y += paint_brush_preview->h() / 2;

    paint_brush_preview->bitmap->setpixelSolid(temp_x, temp_y,
                     convertFormat(getFltkColor(FL_FOREGROUND_COLOR), true), 0);
  }

  paint_brush_preview->redraw();

  paint_size_value->value(new_size);
  paint_size_value->redraw();
}

void PaintOptions::size()
{
  int pos = paint_size->var;

  if (pos > 15)
    pos = 15;

  int new_size = brush_sizes[pos];
  changeSize(new_size);
}

void PaintOptions::sizeValue()
{
  changeSize(paint_size_value->value());
}

void PaintOptions::shape()
{
  changeSize(Project::brush->size);
}

void PaintOptions::stroke()
{
  Project::stroke->type = paint_stroke->var;
}

void PaintOptions::coarseEdge()
{
  Project::brush->coarse_edge = paint_coarse_edge->var;
}

void PaintOptions::fineEdge()
{
  Project::brush->fine_edge = paint_fine_edge->var;
}

void PaintOptions::blurryEdge()
{
  Project::brush->blurry_edge = paint_blurry_edge->var;
}

void PaintOptions::watercolorEdge()
{
  Project::brush->watercolor_edge = paint_watercolor_edge->var;
}

void PaintOptions::chalkEdge()
{
  Project::brush->chalk_edge = paint_chalk_edge->var;
}

void PaintOptions::textureEdge()
{
  Project::brush->texture_edge = paint_texture_edge->var;
}

void PaintOptions::textureMarb()
{
  Project::brush->texture_marb = paint_texture_marb->var;
}

void PaintOptions::textureTurb()
{
  Project::brush->texture_turb = paint_texture_turb->var;
}

void PaintOptions::averageEdge()
{
  Project::brush->average_edge = paint_average_edge->var;
}

void PaintOptions::updateBrush()
{
  paint_brush_preview->do_callback();
  paint_shape->do_callback();
}

int PaintOptions::getMode()
{
  return paint_mode->value();
}

