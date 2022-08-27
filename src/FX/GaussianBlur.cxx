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

namespace GaussianBlur::Items
{
  DialogWindow *dialog;
  InputInt *size;
  InputInt *blend;
  Fl_Choice *mode;
  Fl_Button *ok;
  Fl_Button *cancel;
}

void GaussianBlur::apply()
{
  Bitmap *bmp = Project::bmp;
  Bitmap temp(bmp->cw, bmp->ch, bmp->overscroll);
  int size = atof(Items::size->value()) * 2;

  if((size & 1) == 0)
    size++;

  const int blend = 255 - atoi(Items::blend->value()) * 2.55;
  const int mode = Items::mode->value();
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
      int *p = bmp->row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        rgba_type rgba = getRgba(*p++);
        buf_r[x] = Gamma::fix(rgba.r);
        buf_g[x] = Gamma::fix(rgba.g);
        buf_b[x] = Gamma::fix(rgba.b);
        buf_a[x] = rgba.a;
      }

      p = temp.row[y] + bmp->cl;

      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int rr = 0;
        int gg = 0;
        int bb = 0;
        int aa = 0;
        int div = 0;

        for(int i = 0; i < size; i++) 
        {
          const int xx = x - size / 2 + i;

          if(xx < bmp->cl || xx > bmp->cr)
            continue;

          rr += buf_r[xx];
          gg += buf_g[xx];
          bb += buf_b[xx];
          aa += buf_a[xx];
          div++;
        }

        *p++ = makeRgba(Gamma::unfix(rr / div),
                        Gamma::unfix(gg / div),
                        Gamma::unfix(bb / div),
                        aa / div);
      }
    }

    // y direction
    if(Gui::updateProgress(pass_count++))
      break;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int *p = temp.row[bmp->ct] + x;

      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        rgba_type rgba = getRgba(*p);
        p += bmp->w;
        buf_r[y] = Gamma::fix(rgba.r);
        buf_g[y] = Gamma::fix(rgba.g);
        buf_b[y] = Gamma::fix(rgba.b);
        buf_a[y] = rgba.a;
      }

      p = bmp->row[bmp->ct] + x;

      for(int y = bmp->ct; y <= bmp->cb; y++)
      {
        int rr = 0;
        int gg = 0;
        int bb = 0;
        int aa = 0;
        int div = 0;

        for(int i = 0; i < size; i++) 
        {
          const int yy = y - size / 2 + i;

          if(yy < bmp->ct || yy > bmp->cb)
            continue;

          rr += buf_r[yy];
          gg += buf_g[yy];
          bb += buf_b[yy];
          aa += buf_a[yy];
          div++;
        }

        const int c3 = makeRgba(Gamma::unfix(rr / div),
                                Gamma::unfix(gg / div),
                                Gamma::unfix(bb / div),
                                aa / div);

        if(mode == 1)
          *p = Blend::trans(*p, Blend::keepLum(c3, getl(*p)), blend);
        else if(mode == 2)
          *p = Blend::transAlpha(*p, c3, blend);
        else
          *p = Blend::trans(*p, c3, blend);

        p += bmp->w;
      }
    }
  }

  Gui::hideProgress();
}

void GaussianBlur::close()
{
  Items::dialog->hide();
  Project::undo->push();
  apply();
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
  Items::size = new InputInt(Items::dialog, 0, y1, 96, 24, "Size (1-100)", 0, 1, 100);
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

