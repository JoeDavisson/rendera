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

#include "ForwardFFT.H"

void ForwardFFT::apply()
{
  Bitmap *bmp = Project::bmp;
  int w = bmp->cw;
  int h = bmp->ch;

  // resize image
  Project::resizeImage(w * 2, h);
  bmp = Project::bmp;

  std::vector<float> real(w * h, 0);
  std::vector<float> imag(w * h, 0);
  std::vector<float> real_row(w, 0);
  std::vector<float> imag_row(w, 0);
  std::vector<float> real_col(h, 0);
  std::vector<float> imag_col(h, 0);

  Gui::showProgress(3);

  for(int channel = 0; channel < 3; channel++)
  {
    // forward horizontal pass
    for(int y = 0; y < h; y++)
    {
      int *p = bmp->row[y + bmp->ct] + bmp->cl;
      for(int x = 0; x < w; x++, p++)
      {
	const rgba_type rgba = getRgba(*p);

	switch(channel)
	{
	  case 0:
	    real_row[x] = rgba.r;
	    break;
	  case 1:
	    real_row[x] = rgba.g;
	    break;
	  case 2:
	    real_row[x] = rgba.b;
	    break;
	}

	imag_row[x] = 0;
      }

      ExtraMath::forwardFFT(&real_row[0], &imag_row[0], w);

      for(int x = 0; x < w; x++)
      {
	real[x + w * y] = real_row[x];
	imag[x + w * y] = imag_row[x];
      }
    }

    // forward vertical pass
    for(int x = 0; x < w; x++)
    {
      for(int y = 0; y < h; y++)
      {
	real_col[y] = real[x + w * y];
	imag_col[y] = imag[x + w * y];
      }

      ExtraMath::forwardFFT(&real_col[0], &imag_col[0], h);

      for(int y = 0; y < h; y++)
      {
	real[x + w * y] = real_col[y];
	imag[x + w * y] = imag_col[y];
      }
    }

    // convert to image
    for(int y = 0; y < h; y++)
    {
      for(int x = 0; x < w; x++)
      {
	float re = real[x + w * y];
	float im = imag[x + w * y];
	float mag = log10f(sqrtf(re * re + im * im)) * 32;
	float phase = (atan2f(im, re) + 3.14159f) * 32;
	int val1 = clamp((int)mag, 255);
	int val2 = clamp((int)phase, 255);

	int xx = (x + w / 2) % w;
	int yy = (y + h / 2) % h;
	xx += bmp->cl;
	yy += bmp->ct;
	const rgba_type rgba1 = getRgba(bmp->getpixel(xx, yy));
	const rgba_type rgba2 = getRgba(bmp->getpixel(xx + w, yy));

	switch(channel)
	{
	  case 0:
	    bmp->setpixel(xx, yy, makeRgb(val1, rgba1.g, rgba1.b));
	    bmp->setpixel(xx + w, yy, makeRgb(val2, rgba2.g, rgba2.b));
	    break;
	  case 1:
	    bmp->setpixel(xx, yy, makeRgb(rgba1.r, val1, rgba1.b));
	    bmp->setpixel(xx + w, yy, makeRgb(rgba2.r, val2, rgba2.b));
	    break;
	  case 2:
	    bmp->setpixel(xx, yy, makeRgb(rgba1.r, rgba1.g, val1));
	    bmp->setpixel(xx + w, yy, makeRgb(rgba2.r, rgba2.g, val2));
	    break;
	}
      }
    }

    if(Gui::updateProgress(channel) < 0)
      return;
  }

  Gui::hideProgress();
  Gui::getView()->drawMain(true);
}

void ForwardFFT::begin()
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

