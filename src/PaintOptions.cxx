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

  Widget *paint_brush_preview;
  Widget *paint_size;
  InputInt *paint_size_value;
  Widget *paint_stroke;
  Widget *paint_shape;
  Widget *paint_coarse_edge;
  Widget *paint_fine_edge;
  Widget *paint_blurry_edge;
  Widget *paint_watercolor_edge;
  Widget *paint_chalk_edge;
  Widget *paint_texture_edge;
  Widget *paint_texture_marb;
  Widget *paint_texture_turb;
  Widget *paint_average_edge;
  Fl_Choice *paint_mode;

  void cb_sizeValue(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->sizeValue((Widget *)w, data); }

  void cb_size(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->size((Widget *)w, data); }

  void cb_shape(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->shape((Widget *)w, data); }

  void cb_stroke(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->stroke((Widget *)w, data); }

  void cb_mode(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->mode(); }

  void cb_coarseEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->coarseEdge((Widget *)w, data); }

  void cb_fineEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->fineEdge((Widget *)w, data); }

  void cb_blurryEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->blurryEdge((Widget *)w, data); }

  void cb_watercolorEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->watercolorEdge((Widget *)w, data); }

  void cb_chalkEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->chalkEdge((Widget *)w, data); }

  void cb_textureEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->textureEdge((Widget *)w, data); }

  void cb_textureMarb(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->textureMarb((Widget *)w, data); }

  void cb_textureTurb(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->textureTurb((Widget *)w, data); }

  void cb_averageEdge(Fl_Widget *w, void *data) { PaintOptions *temp = (PaintOptions *)data; temp->averageEdge((Widget *)w, data); }
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
                       "", (Fl_Callback *)cb_sizeValue, 1, Brush::max_size);
  paint_size_value->textsize(16);
  pos += 32 + 8;

  paint_size = new Widget(this, 8, pos, 160, 32,
                          "Brush Size", images_size_png, 10, 32,
                          (Fl_Callback *)cb_size);
  pos += 32 + 8;

  paint_shape = new Widget(this, 8, pos, 160, 40,
                           "Shape Adjust", images_shape_png, 10, 40,
                           (Fl_Callback *)cb_shape);
  pos += 40 + Gui::SPACING;

  new Separator(this, 0, pos, Gui::OPTIONS_WIDTH, Separator::HORIZONTAL, "");
  pos += 4 + Gui::SPACING;

  paint_stroke = new Widget(this, 8, pos, 160, 80,
                            "Brushstroke Type", images_stroke_png, 40, 40,
                            (Fl_Callback *)cb_stroke);

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
  paint_mode->callback((Fl_Callback *)cb_mode);
  pos += 32 + 8;

  paint_coarse_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)cb_coarseEdge);
  paint_fine_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)cb_fineEdge);
  paint_blurry_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)cb_blurryEdge);
  paint_watercolor_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_watercolor_edge_png, 20, 32,
                          (Fl_Callback *)cb_watercolorEdge);
  paint_chalk_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_chalk_edge_png, 20, 32,
                          (Fl_Callback *)cb_chalkEdge);
  paint_texture_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)cb_textureEdge);
  paint_texture_marb = new Widget(this, 8, pos + 40, 160, 32,
                          "Marbleize", images_marbleize_png, 20, 32,
                          (Fl_Callback *)cb_textureMarb);
  paint_texture_turb = new Widget(this, 8, pos + 80, 160, 32,
                          "Turbulence", images_turbulence_png, 20, 32,
                          (Fl_Callback *)cb_textureTurb);
  paint_average_edge = new Widget(this, 8, pos, 160, 32,
                          "Edge", images_edge_png, 20, 32,
                          (Fl_Callback *)cb_averageEdge);

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

void PaintOptions::changeSize(int size)
{
  Brush *brush = Project::brush;
  float round = (float)(15 - paint_shape->var) / 15;

  brush->make(size, round);
  paint_brush_preview->bitmap->clear(convertFormat(getFltkColor(FL_BACKGROUND2_COLOR), true));
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);
  paint_brush_preview->bitmap->rect(0, 0,
                     paint_brush_preview->bitmap->w - 1,
                     paint_brush_preview->bitmap->h - 1,
                     getFltkColor(Project::fltk_theme_bevel_down), 0);

  const double aspect = 128.0 / size;

  for (int i = 0; i < brush->solid_count; i++)
  {
    int temp_x = brush->solidx[i];
    int temp_y = brush->solidy[i];

    if (size > 128)
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

  char s[32];
  snprintf(s, sizeof(s), "%d", (int)size);

  paint_size_value->value(s);
  paint_size_value->redraw();
}

void PaintOptions::size(Widget *, void *var)
{
  int new_size = brush_sizes[*(int *)var];
  changeSize(new_size);
  char s[16];
  snprintf(s, sizeof(s), "%d", (int)new_size);
  paint_size_value->value(s);
  paint_size_value->redraw();
}

void PaintOptions::sizeValue(Widget *, void *)
{
  int size;
  sscanf(paint_size_value->value(), "%d", &size);
  changeSize(size);
}

void PaintOptions::shape(Widget *, void *)
{
  changeSize(Project::brush->size);
}

void PaintOptions::stroke(Widget *, void *var)
{
  Project::stroke->type = *(int *)var;
}

void PaintOptions::coarseEdge(Widget *, void *var)
{
  Project::brush->coarse_edge = *(int *)var;
}

void PaintOptions::fineEdge(Widget *, void *var)
{
  Project::brush->fine_edge = *(int *)var;
}

void PaintOptions::blurryEdge(Widget *, void *var)
{
  Project::brush->blurry_edge = *(int *)var;
}

void PaintOptions::watercolorEdge(Widget *, void *var)
{
  Project::brush->watercolor_edge = *(int *)var;
}

void PaintOptions::chalkEdge(Widget *, void *var)
{
  Project::brush->chalk_edge = *(int *)var;
}

void PaintOptions::textureEdge(Widget *, void *var)
{
  Project::brush->texture_edge = *(int *)var;
}

void PaintOptions::textureMarb(Widget *, void *var)
{
  Project::brush->texture_marb = *(int *)var;
}

void PaintOptions::textureTurb(Widget *, void *var)
{
  Project::brush->texture_turb = *(int *)var;
}

void PaintOptions::averageEdge(Widget *, void *var)
{
  Project::brush->average_edge = *(int *)var;
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

