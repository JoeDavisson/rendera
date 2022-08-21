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

#include "InverseFFT.H"

void InverseFFT::apply()
{
  Bitmap *bmp = Project::bmp;
  int w = bmp->cw / 2;
  int h = bmp->ch;

  std::vector<float> real(w * h, 0);
  std::vector<float> imag(w * h, 0);
  std::vector<float> real_row(w, 0);
  std::vector<float> imag_row(w, 0);
  std::vector<float> real_col(h, 0);
  std::vector<float> imag_col(h, 0);

  Gui::showProgress(3);

  for(int channel = 0; channel < 3; channel++)
  {
    // convert from image
    for(int y = 0; y < h; y++)
    {
      for(int x = 0; x < w; x++)
      {
	int xx = (x + w / 2) % w;
	int yy = (y + h / 2) % h;
	xx += bmp->cl;
	yy += bmp->ct;

	int p1 = bmp->getpixel(xx, yy);
	int p2 = bmp->getpixel(xx + w, yy);
	float c1 = 0, c2 = 0;

	switch(channel)
	{
	  case 0:
	    c1 = getr(p1);
	    c2 = getr(p2);
	    break;
	  case 1:
	    c1 = getg(p1);
	    c2 = getg(p2);
	    break;
	  case 2:
	    c1 = getb(p1);
	    c2 = getb(p2);
	    break;
	}

	float mag = powf(10.0f, c1 / 32);
	float phase = c2 / 32 - 3.14159f;

	real[x + w * y] = mag * cosf(phase);
	imag[x + w * y] = mag * sinf(phase);
      }
    }

    // inverse horizontal pass
    for(int y = 0; y < h; y++)
    {
      for(int x = 0; x < w; x++)
      {
	real_row[x] = real[x + w * y];
	imag_row[x] = imag[x + w * y];
      }

      ExtraMath::inverseFFT(&real_row[0], &imag_row[0], w);

      for(int x = 0; x < w; x++)
      {
	real[x + w * y] = real_row[x];
	imag[x + w * y] = imag_row[x];
      }
    }

    // inverse vertical pass
    for(int x = 0; x < w; x++)
    {
      for(int y = 0; y < h; y++)
      {
	real_col[y] = real[x + w * y];
	imag_col[y] = imag[x + w * y];
      }

      ExtraMath::inverseFFT(&real_col[0], &imag_col[0], h);

      for(int y = 0; y < h; y++)
      {
	real[x + w * y] = real_col[y];
	imag[x + w * y] = imag_col[y];
      }
    }

    // convert to image
    for(int y = 0; y < h; y++)
    {
      int *p = bmp->row[y + bmp->ct] + bmp->cl;

      for(int x = 0; x < w; x++, p++)
      {
	float re = real[x + w * y];
	int val = clamp((int)re, 255);

	const rgba_type rgba = getRgba(*p);

	switch(channel)
	{
	  case 0:
	    *p = makeRgb(val, rgba.g, rgba.b);
	    break;
	  case 1:
	    *p = makeRgb(rgba.r, val, rgba.b);
	    break;
	  case 2:
	    *p = makeRgb(rgba.r, rgba.g, val);
	    break;
	}
      }
    }

    if(Gui::updateProgress(channel) < 0)
      return;
  }

  Gui::hideProgress();
  Project::resizeImage(w, h);
  bmp = Project::bmp;
  Gui::getView()->drawMain(true);
}

void InverseFFT::begin()
{
  int w = Project::bmp->cw;
  int h = Project::bmp->ch;

  if(ExtraMath::isPowerOfTwo(w) && ExtraMath::isPowerOfTwo(h))
  {
    Project::undo->push();
    apply();
  }
  else
  {
    Dialog::message("Error", "Image dimensions must\nbe powers of two.");
  }
}

