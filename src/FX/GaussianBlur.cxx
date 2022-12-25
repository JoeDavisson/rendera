/*
Copyright (c) 2021 Joe Davisson.

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

#include "GaussianBlur.H"

namespace
{
  void extendBorders(Bitmap *bmp)
  {
    const int cl = bmp->cl;
    const int cr = bmp->cr;
    const int ct = bmp->ct;
    const int cb = bmp->cb;
    const int w = bmp->w;
    const int h = bmp->h;

    bmp->setClip(0, 0, bmp->w - 1, bmp->h - 1);

    // left
    for(int y = ct; y <= cb; y++)
      bmp->hline(0, y, cl - 1, bmp->getpixel(cl, y), 0);

    // right
    for(int y = ct; y <= cb; y++)
      bmp->hline(cr + 1, y, w - 1, bmp->getpixel(cr, y), 0);

    // top
    for(int x = cl; x <= cr; x++)
      bmp->vline(0, x, ct - 1, bmp->getpixel(x, ct), 0);

    // bottom
    for(int x = cl; x <= cr; x++)
      bmp->vline(cb + 1, x, h - 1, bmp->getpixel(x, cb), 0);

    // upper-left
    bmp->rectfill(0, 0, cl - 1, ct - 1, bmp->getpixel(cl, ct), 0);

    // upper-right
    bmp->rectfill(cr, 0, w - 1, ct - 1, bmp->getpixel(cr, ct), 0);

    // lower-left
    bmp->rectfill(0, cb + 1, cl - 1, h - 1, bmp->getpixel(cl, cb), 0);

    // lower-right
    bmp->rectfill(cr + 1, cb + 1, w - 1, h - 1, bmp->getpixel(cr, cb), 0);
  }
}

void redrawBorder(Bitmap *bmp)
{
    const int cl = bmp->overscroll;
    const int cr = bmp->w - bmp->overscroll - 1;
    const int ct = bmp->overscroll;
    const int cb = bmp->h - bmp->overscroll - 1;
    const int w = bmp->w;
    const int h = bmp->h;
    const int c = convertFormat(getFltkColor(FL_BACKGROUND_COLOR), true);

    bmp->setClip(0, 0, w - 1, h - 1);
    bmp->rectfill(0, 0, cl - 1, h - 1, c, 0);
    bmp->rectfill(cr + 1, 0, w - 1, h - 1, c, 0);
    bmp->rectfill(cl, 0, cr, ct - 1, c, 0);
    bmp->rectfill(cl, cb + 1, cr, h - 1, c, 0);
    bmp->setClip(bmp->overscroll, bmp->overscroll,
                 bmp->w - bmp->overscroll - 1, bmp->h - bmp->overscroll - 1);
}

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *size;
    InputInt *blend;
    Fl_Choice *mode;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void GaussianBlur::apply(Bitmap *bmp, int size, int blend, int mode)
{
  if(size < 1)
    return;

  Bitmap temp(bmp->w, bmp->h);

  extendBorders(bmp);

  bmp->setClip(0, 0, bmp->w - 1, bmp->h - 1);
  temp.setClip(0, 0, temp.w - 1, temp.h - 1);

  if(size > bmp->overscroll - 2)
    size = bmp->overscroll - 2;

  if((size & 1) == 0)
  {
    size++;
    blend = scaleVal(blend, 128);
  }

  const int larger = bmp->w > bmp->h ? bmp->w : bmp->h;

  std::vector<int> buf_r(larger);
  std::vector<int> buf_g(larger);
  std::vector<int> buf_b(larger);
  std::vector<int> buf_a(larger);

  Gui::showProgress(6, 1);

  int pass_count = 0;

  for(int pass = 0; pass < 3; pass++)
  {
    // x direction
    if(Gui::updateProgress(pass_count++))
      break;

    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(bmp->getpixel(x, y));
        buf_r[x] = Gamma::fix(rgba.r);
        buf_g[x] = Gamma::fix(rgba.g);
        buf_b[x] = Gamma::fix(rgba.b);
        buf_a[x] = rgba.a;
      }

      int accum_r = 0;
      int accum_g = 0;
      int accum_b = 0;
      int accum_a = 0;
      int div = 1;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        const int mx = x - div;

        accum_r -= buf_r[mx];
        accum_g -= buf_g[mx];
        accum_b -= buf_b[mx];
        accum_a -= buf_a[mx];

        accum_r += buf_r[x];
        accum_g += buf_g[x];
        accum_b += buf_b[x];
        accum_a += buf_a[x];

        div++;

        if(div > size)
          div = size;

        const int c = makeRgba(Gamma::unfix(accum_r / div),
                               Gamma::unfix(accum_g / div),
                               Gamma::unfix(accum_b / div),
                               accum_a / div);

        temp.setpixel(x - size / 2, y - size / 2, c);
      }
    }

    // y direction
    if(Gui::updateProgress(pass_count++))
      break;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        rgba_type rgba = getRgba(temp.getpixel(x, y));
        buf_r[y] = Gamma::fix(rgba.r);
        buf_g[y] = Gamma::fix(rgba.g);
        buf_b[y] = Gamma::fix(rgba.b);
        buf_a[y] = rgba.a;
      }

      int accum_r = 0;
      int accum_g = 0;
      int accum_b = 0;
      int accum_a = 0;
      int div = 1;

      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        const int my = y - div;

        accum_r -= buf_r[my];
        accum_g -= buf_g[my];
        accum_b -= buf_b[my];
        accum_a -= buf_a[my];

        accum_r += buf_r[y];
        accum_g += buf_g[y];
        accum_b += buf_b[y];
        accum_a += buf_a[y];

        div++;

        if(div > size)
          div = size;

        int c1 = bmp->getpixel(x, y);

        const int c2 = makeRgba(Gamma::unfix(accum_r / div),
                                Gamma::unfix(accum_g / div),
                                Gamma::unfix(accum_b / div),
                                accum_a / div);

        if(mode == 0)
        {
          bmp->setpixel(x, y, Blend::trans(c1, c2, blend));
        }
        else if(mode == 1)
        {
          bmp->setpixel(x, y,
                        Blend::trans(c1, Blend::keepLum(c2, getl(c1)), blend));
        }
        else if(mode == 2)
        {
          bmp->setpixel(x, y, Blend::transAlpha(c1, c2, blend));
        }
      }
    }
  }

  redrawBorder(bmp);
  Gui::hideProgress();
}

void GaussianBlur::close()
{
  Items::dialog->hide();
  Project::undo->push();

  int size = atof(Items::size->value()) + 1;
  int blend = 255 - atoi(Items::blend->value()) * 2.55;
  int mode = Items::mode->value();

  apply(Project::bmp, size, blend, mode);
}

void GaussianBlur::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void GaussianBlur::begin()
{
  Items::dialog->show();
}

void GaussianBlur::init()
{
  int y1 = 8;
  int ww = 0;
  int hh = 0;

  Items::dialog = new DialogWindow(256, 0, "Gaussian Blur");
  Items::size = new InputInt(Items::dialog, 0, y1, 96, 24, "Size (1-60)", 0, 1, 60);
  y1 += 24 + 8;
  Items::size->value("1");
  Items::size->center();
  Items::blend = new InputInt(Items::dialog, 0, y1, 96, 24, "Blend %", 0, 0, 100);
  Items::blend->value("100");
  Items::blend->center();
  y1 += 24 + 8;
  Items::mode = new Fl_Choice(0, y1, 96, 24, "Mode:");
  Items::mode->labelsize(12);
  Items::mode->textsize(12);
  Items::mode->add("Normal");
  Items::mode->add("Color Only");
  Items::mode->add("Alpha Only");
  Items::mode->value(0);
  Items::mode->align(FL_ALIGN_LEFT);
  Items::mode->measure_label(ww, hh);
  Items::mode->resize(Items::dialog->x() + Items::dialog->w() / 2 - (Items::mode->w() + ww) / 2 + ww, Items::mode->y(), Items::mode->w(), Items::mode->h());
  y1 += 24 + 8;
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

