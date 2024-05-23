/*
Copyright (c) 2024 Joe Davisson.

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

#include <FL/Fl.H>

#include <cstdio>
#include <cmath>
#include <climits>
#include <cstdint>

#include "Common.H"

static int rnd_seed = 12345;

// get rgba values all at once
rgba_type getRgba(const uint32_t n)
{
  un_rgba_type u;
  u.uint32_ = n;
  return u.rgba_;
}

// make RGB color with no alpha
int makeRgb(const int r, const int g, const int b)
{
  return r | g << 8 | b << 16 | 0xff000000;
}

// make RGBA color
int makeRgba(const int r, const int g, const int b, const int a)
{
  return r | g << 8 | b << 16 | a << 24;
}

// make RGB color, 24-bits only
int makeRgb24(const int r, const int g, const int b)
{
  return r | g << 8 | b << 16;
}

// get red channel
int getr(const int c)
{
  return c & 255;
}

// get green channel
int getg(const int c)
{
  return (c >> 8) & 255;
}

// get blue channel
int getb(const int c)
{
  return (c >> 16) & 255;
}

// get alpha channel
int geta(const int c)
{
  return (c >> 24) & 255;
}

// get value
int getv(const int c)
{
  return (getr(c) + getg(c) + getb(c)) / 3;
}

// get rec 601 luminance
int getl(const int c)
{
  return ((76 * getr(c)) + (150 * getg(c)) + (29 * getb(c))) / 255;

/*
  const rgba_type rgba = getRgba(c);

  return ((((rgba.r << 6) + (rgba.r << 3) + (rgba.r << 2)) +
           ((rgba.g << 7) + (rgba.g << 4) + (rgba.g << 2) + (rgba.g >> 1)) +
           ((rgba.b << 4) + (rgba.b << 3) + (rgba.b << 2) + (rgba.b)))) >> 8;
*/
}

// get rec 601 luminance from unpacked RGB values
int getlUnpacked(const int r, const int g, const int b)
{
  return (76 * r + 150 * g + 29 * b) / 255;
/*
  return ((((r << 6) + (r << 3) + (r << 2)) +
           ((g << 7) + (g << 4) + (g << 2) + (g >> 1)) +
           ((b << 4) + (b << 3) + (b << 2) + (b)))) >> 8;
*/
}

// 76 (r << 6) + (r << 3) + (r << 2) // 76
// 150 (g << 7) + (g << 4) + (g << 2) + (g >> 1)
// 29  (b << 4) + (b << 3) + (b << 2) + (b)


// scales a gray value to a particular level
int scaleVal(const int a, const int b)
{
  return (a * (255 - b) / 255) + b;
}

// 3D distance
int diff24(const int c1, const int c2)
{
  const struct rgba_type rgba1 = getRgba(c1);
  const struct rgba_type rgba2 = getRgba(c2);
  const int r = rgba1.r - rgba2.r;
  const int g = rgba1.g - rgba2.g;
  const int b = rgba1.b - rgba2.b;

  return r * r + g * g + b * b;
}

// 4D distance
int diff32(const int c1, const int c2)
{
  const struct rgba_type rgba1 = getRgba(c1);
  const struct rgba_type rgba2 = getRgba(c2);
  const int r = rgba1.r - rgba2.r;
  const int g = rgba1.g - rgba2.g;
  const int b = rgba1.b - rgba2.b;
  const int a = rgba1.a - rgba2.a;

  return r * r + g * g + b * b + a * a;
}

// convert internal RGB format to whatever the graphics card needs
// currently only supporting RGB/BGR as I don't think anything
// else really exists
int convertFormat(const int c, const bool bgr_order)
{
  if (bgr_order)
  {
    const int rb = c & 0x00ff00ff;
    const int ga = c & 0xff00ff00;

    return ((rb << 16) | (rb >> (32 - 16))) | ga;
  }

  return c;
}

// get an FLTK color
int getFltkColor(const int c)
{
  return (Fl::get_color(c) >> 8) | 0xff000000;
}

int clamp(const int value, const int ceiling)
{
  if(value < 0)
    return 0;
  else if(value > ceiling)
    return ceiling;
  else
    return value;
}

int range(const int value, const int floor, const int ceiling)
{
  if(value < floor)
    return floor;
  else if(value > ceiling)
    return ceiling;
  else
    return value;
}

// pseudo-random number
int rnd()
{
  rnd_seed ^= rnd_seed << 17;
  rnd_seed ^= rnd_seed >> 13;
  rnd_seed ^= rnd_seed <<  5;

  return rnd_seed;
}

// file access functions
uint8_t parseUint8(unsigned char *&buffer)
{
  uint8_t num = buffer[0];

  buffer += 1;
  return num;
}

uint16_t parseUint16(unsigned char *&buffer)
{
  uint16_t num;

#if ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )
  num = buffer[0] | buffer[1] << 8;
#elif ( __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ )
  num = buffer[1] | buffer[0] << 8;
#else
#error "unsupported endianness"
#endif

  buffer += 2;
  return num;
}

uint32_t parseUint32(unsigned char *&buffer)
{
  uint32_t num;

#if ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )
  num = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
#elif ( __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ )
  num = buffer[3] | buffer[2] << 8 | buffer[1] << 16 | buffer[0] << 24;
#else
#error "unsupported endianness"
#endif

  buffer += 4;
  return num;
}

void writeUint8(const uint8_t num, FILE *out)
{
  fputc(num, out);
}

void writeUint16(const uint16_t num, FILE *out)
{
#if ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
#else
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
#endif
}

void writeUint32(const uint32_t num, FILE *out)
{
#if ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 24) & 0xff, out);
#else
  fputc((num >> 24) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
#endif
}

