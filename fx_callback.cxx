/*
Copyright (c) 2014 Joe Davisson.

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

#include "rendera.h"

extern Gui *gui;
extern FX *fx;

// normalize
void normalize()
{
  Bitmap *bmp = Bitmap::main;
  int overscroll = bmp->overscroll;

  // search for highest & lowest RGB values
  int x, y;
  int r_high = 0;
  int g_high = 0;
  int b_high = 0;
  int r_low = 0xFFFFFF;
  int g_low = 0xFFFFFF;
  int b_low = 0xFFFFFF;

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = getr(c);
      int g = getg(c);
      int b = getb(c);

      if(r < r_low)
        r_low = r;
      if(r > r_high)
        r_high = r;
      if(g < g_low)
        g_low = g;
      if(g > g_high)
        g_high = g;
      if(b < b_low)
        b_low = b;
      if(b > b_high)
        b_high = b;
    }
  }
  if(!(r_high - r_low))
    r_high++;
  if(!(g_high - g_low))
    g_high++;
  if(!(b_high - b_low))
    b_high++;

  // scale image
  float r_scale = 255.0f / (r_high - r_low);
  float g_scale = 255.0f / (g_high - g_low);
  float b_scale = 255.0f / (b_high - b_low);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = (getr(c) - r_low) * r_scale;
      int g = (getg(c) - g_low) * g_scale;
      int b = (getb(c) - b_low) * b_scale;

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
      gui->view->draw_main(1);
  }
}

void equalize()
{
  Bitmap *bmp = Bitmap::main;
  int overscroll = bmp->overscroll;

  int *list_r = new int[256];
  int *list_g = new int[256];
  int *list_b = new int[256];

  int x, y;
  int i, j;

  for(i = 0; i < 256; i++)
  {
    list_r[i] = 0;
    list_g[i] = 0;
    list_b[i] = 0;
  }

  int size = (bmp->w - overscroll * 2) * (bmp->h - overscroll * 2);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = getr(c);
      int g = getg(c);
      int b = getb(c);

      list_r[r]++;
      list_g[g]++;
      list_b[b]++;
    }
  }

  for(j = 255; j >= 0; j--)
  {
    for(i = 0; i < j; i++)
    {
      list_r[j] += list_r[i];
      list_g[j] += list_g[i];
      list_b[j] += list_b[i];
    }
  }

  float scale = 255.0f / size;
  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = getr(c);
      int g = getg(c);
      int b = getb(c);

      r = list_r[r] * scale;
      g = list_g[g] * scale;
      b = list_b[b] * scale;

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
      gui->view->draw_main(1);
  }

  delete[] list_r;
  delete[] list_g;
  delete[] list_b;
}

// saturate
void saturate()
{
  Bitmap *bmp = Bitmap::main;
  int overscroll = bmp->overscroll;

  int *list_s = new int[256];

  int x, y;
  int i, j;

  for(i = 0; i < 256; i++)
    list_s[i] = 0;

  int size = (bmp->w - overscroll * 2) * (bmp->h - overscroll * 2);

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = getr(c);
      int g = getg(c);
      int b = getb(c);
      int h, s, v;
      Blend::rgb_to_hsv(r, g, b, &h, &s, &v);

      list_s[s]++;
    }
  }
  for(j = 255; j >= 0; j--)
    for(i = 0; i < j; i++)
      list_s[j] += list_s[i];

  float scale = 255.0f / size;

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      int r = getr(c);
      int g = getg(c);
      int b = getb(c);
      int h, s, v;
      Blend::rgb_to_hsv(r, g, b, &h, &s, &v);
      int temp = s;
      s = list_s[s] * scale;
      if(s < temp)
        s = temp;
      Blend::hsv_to_rgb(h, s, v, &r, &g, &b);

      bmp->setpixel(x, y, makecol(r, g, b), 0);
    }

    if(!(y % 64))
      gui->view->draw_main(1);
  }

  delete[] list_s;
}

// rotate hue
void show_rotate_hue()
{
  fx->rotate_hue->show();
}

void hide_rotate_hue()
{
  char s[8];

  int amount = atoi(fx->rotate_hue_amount->value());

  if(amount < 1)
  {
    snprintf(s, sizeof(s), "%d", 1);
    fx->rotate_hue_amount->value(s);
    return;
  }

  if(amount > 359)
  {
    snprintf(s, sizeof(s), "%d", 359);
    fx->rotate_hue_amount->value(s);
    return;
  }

  fx->rotate_hue->hide();
}

void cancel_rotate_hue()
{
  fx->rotate_hue->hide();
}

// invert
void invert()
{
  Bitmap *bmp = Bitmap::main;
  int overscroll = bmp->overscroll;

  int x, y;

  for(y = overscroll; y < bmp->h - overscroll; y++)
  {
    for(x = overscroll; x < bmp->w - overscroll; x++)
    {
      int c = bmp->getpixel(x, y);
      bmp->setpixel(x, y, Blend::invert(c, 0, 0), 0);
    }

    if(!(y % 64))
    {
      gui->view->draw_main(1);
    }
  }
}

// restore
void show_restore()
{
  fx->restore->show();
}

void hide_restore()
{
  fx->restore->hide();
}

void cancel_restore()
{
  fx->restore->hide();
}

// remove dust
void show_remove_dust()
{
  fx->remove_dust->show();
}

void hide_remove_dust()
{
  fx->remove_dust->hide();
}

void cancel_remove_dust()
{
  fx->remove_dust->hide();
}

// colorize
void colorize()
{
}

