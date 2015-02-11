/*
Copyright (c) 2015 Joe Davisson.

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

#include <cmath>

#include "Math.H"

// fft routines adapted from those found here:
// http://www.dspguide.com/ch12/3.htm
void Math::forwardFFT(float *real, float *imag, int size)
{
  int j = size / 2;
  int i = 0, k = 0;

  for(i = 1; i <= size - 2; i++)
  {
    if(i < j)
    {
      const float tr = real[j];
      const float ti = imag[j];
      real[j] = real[i];
      imag[j] = imag[i];
      real[i] = tr;
      imag[i] = ti;
    }

    k = size / 2;

    while(k <= j)
    {
      j -= k;
      k /= 2;
    }

    j += k;
  }

  for(k = 1; k <= (int)(logf(size) / logf(2)); k++)
  {
    const int le = (int)(powf(2, k));
    const int le2 = le / 2;

    float ur = 1;
    float ui = 0;
    float sr = cosf(M_PI / le2);
    float si = -sinf(M_PI / le2);

    for(j = 1; j <= le2; j++)
    {
      for(i = j - 1; i < size; i += le)
      {
        const int ip = i + le2;
        const float tr = real[ip] * ur - imag[ip] * ui;
        const float ti = real[ip] * ui + imag[ip] * ur;
        real[ip] = real[i] - tr;
        imag[ip] = imag[i] - ti;
        real[i] += tr;
        imag[i] += ti;
      }

      const float tr = ur;
      ur = tr * sr - ui * si;
      ui = tr * si + ui * sr;
    }
  }
}

void Math::inverseFFT(float *real, float *imag, int size)
{
  for(int k = 0; k < size; k++)
    imag[k] = -imag[k];

  forwardFFT(real, imag, size);

  for(int i = 0; i < size; i++)
  {
    real[i] = real[i] / size;
    imag[i] = -imag[i] / size;
  }
}

