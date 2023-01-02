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

#include "Painting.H"

namespace
{
  namespace Items
  {
    DialogWindow *dialog;
    InputInt *amount;
    Fl_Button *ok;
    Fl_Button *cancel;
  }
}

void Painting::apply()
{
  Bitmap *bmp = Project::bmp;
  int amount = atoi(Items::amount->value());

  Gui::showProgress(bmp->h);

  for(int y = bmp->ct; y <= bmp->cb; y++)
  {
    int *p = bmp->row[y] + bmp->cl;

    for(int x = bmp->cl; x <= bmp->cr; x++)
    {
      int r = 0;
      int g = 0;
      int b = 0;
      int count = 0;

      for(int j = -amount; j <= amount; j++) 
      {
	for(int i = -amount; i <= amount; i++) 
	{
	  const int c3 = *p;
	  const int c1 = bmp->getpixel(x + i, y + j);
	  const int c2 = bmp->getpixel(x - i, y - j);

	  if(diff24(c3, c1) < diff24(c3, c2))
	  {
	    r += getr(c1);
	    g += getg(c1);
	    b += getb(c1);
	  }
	  else
	  {
	    r += getr(c2);
	    g += getg(c2);
	    b += getb(c2);
	  }

	  count++;
	}
      }

      r /= count;
      g /= count;
      b /= count;

      *p = makeRgba(r, g, b, geta(*p));
      p++;
    }

    if(Gui::updateProgress(y) < 0)
      return;
  }

  Gui::hideProgress();
}

void Painting::close()
{
    Items::dialog->hide();
    Project::undo->push();
    apply();
}

void Painting::quit()
{
  Gui::hideProgress();
  Items::dialog->hide();
}

void Painting::begin()
{
  Items::dialog->show();
}

void Painting::init()
{
  int y1 = 8;

  Items::dialog = new DialogWindow(256, 0, "Painting");
  Items::amount = new InputInt(Items::dialog, 0, y1, 96, 24, "Amount (1-10)", 0, 1, 10);
  y1 += 24 + 8;
  Items::amount->value("3");
  Items::amount->center();
  Items::dialog->addOkCancelButtons(&Items::ok, &Items::cancel, &y1);
  Items::ok->callback((Fl_Callback *)close);
  Items::cancel->callback((Fl_Callback *)quit);
  Items::dialog->set_modal();
  Items::dialog->end();
}

