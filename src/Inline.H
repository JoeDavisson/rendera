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

#ifndef INLINE_H
#define INLINE_H

#include <FL/Fl.H>

#include <cmath>
#include <climits>
#include <stdint.h>

#if ! defined( __BYTE_ORDER__ )
#  error "__BYTE_ORDER__ undefined"
#endif
#if ! defined( __ORDER_LITTLE_ENDIAN__ )
#  error "__ORDER_LITTLE_ENDIAN__ undefined"
#endif
#if ! defined( __ORDER_BIG_ENDIAN__ )
#  error "__ORDER_BIG_ENDIAN__ undefined"
#endif

// stuff for extracting all RGBA values quickly
struct rgba_type
{
#if 0
  /*  */
#elif ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )
  uint8_t r ;
  uint8_t g ;
  uint8_t b ;
  uint8_t a ;
#elif ( __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ )
  uint8_t a ;
  uint8_t b ;
  uint8_t g ;
  uint8_t r ;
#else
#  error "unsupported endianness"
#endif
};

union un_rgba_type
{
  uint32_t uint32_ ;
  rgba_type rgba_ ;
};

// get rgba values all at once
inline rgba_type getRgba( uint32_t const &n )
{
  un_rgba_type u ;
  u.uint32_ = n ;
  return u.rgba_ ;
}

// make RGB color with no alpha
inline int makeRgb(const int &r, const int &g, const int &b)
{
  return r | g << 8 | b << 16 | 0xFF000000;
}

// make RGBA color
inline int makeRgba(const int &r, const int &g, const int &b, const int &a)
{
  return r | g << 8 | b << 16 | a << 24;
}

// make RGB color, 24-bits only
inline int makeRgb24(const int &r, const int &g, const int &b)
{
  return r | g << 8 | b << 16;
}

// get red channel
inline int getr(const int &c)
{
  return c & 255;
}

// get green channel
inline int getg(const int &c)
{
  return (c >> 8) & 255;
}

// get blue channel
inline int getb(const int &c)
{
  return (c >> 16) & 255;
}

// get alpha channel
inline int geta(const int &c)
{
  return (c >> 24) & 255;
}

// get value
inline int getv(const int &c)
{
  return (getr(c) + getg(c) + getb(c)) / 3;
}

// get rec 601 luminance
inline int getl(const int &c)
{
  return ((76 * getr(c)) + (150 * getg(c)) + (29 * getb(c))) / 255;
}

// get rec 601 luminance from unpacked RGB values
inline int getlUnpacked(const int &r, const int &g, const int &b)
{
  return (76 * r + 150 * g + 29 * b) / 255;
}

// scales a gray value to a particular level
inline int scaleVal(const int &a, const int &b)
{
  return (a * (255 - b) / 255) + b;
}

// get an FLTK color
inline int getFltkColor(const int &c)
{
  return (Fl::get_color(c) >> 8) | 0xFF000000;
}

// 3D quadratic distance
inline int diff24(const int &c1, const int &c2)
{
  const struct rgba_type rgba1 = getRgba(c1);
  const struct rgba_type rgba2 = getRgba(c2);
  const int r = rgba1.r - rgba2.r;
  const int g = rgba1.g - rgba2.g;
  const int b = rgba1.b - rgba2.b;

  return r * r + g * g + b * b;
}

// normal blend approximation (saves a multiply)
inline int blendFast(const int &c1, const int &c2, const int &t)
{
  const int rb =
    (((((c1 & 0xFF00FF) - (c2 & 0xFF00FF)) * t) >> 8) + c2) & 0xFF00FF;
  const int g = (((((c1 & 0xFF00) - (c2 & 0xFF00)) * t) >> 8) + c2) & 0xFF00;

  return rb | g | 0xFF000000;
}

// convert internal RGB format to whatever the graphics card needs
// currently only supporting RGB/BGR as I don't think anything
// else really exists
inline int convertFormat(const int &c, const bool &bgr_order)
{
  if(bgr_order)
    return makeRgb(getb(c), getg(c), getr(c));
  else
    return c;
}

inline int clampByte(const int &v)
{
  const int temp = v > 255 ? 255 : v;
  return temp < 0 ? 0 : temp;
}

// file access functions
inline uint8_t parseUint8(unsigned char *&buffer)
{
  uint8_t num = buffer[0];

  buffer += 1;
  return num;
}

inline uint16_t parseUint16(unsigned char *&buffer)
{
  uint16_t num;

#if 0
  /*  */
#elif ( BYTE_ORDER == LITTLE_ENDIAN )
  num = buffer[0] | buffer[1] << 8;
#elif ( BYTE_ORDER == BIG_ENDIAN )
  num = buffer[1] | buffer[0] << 8;
#else
#  error "unsupported endianness"
#endif

  buffer += 2;
  return num;
}

inline uint32_t parseUint32(unsigned char *&buffer)
{
  uint32_t num;

#if 0
  /*  */
#elif ( BYTE_ORDER == LITTLE_ENDIAN )
  num = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
#elif ( BYTE_ORDER == BIG_ENDIAN )
  num = buffer[3] | buffer[2] << 8 | buffer[1] << 16 | buffer[0] << 24;
#else
#  error "unsupported endianness"
#endif

  buffer += 4;
  return num;
}

inline void writeUint8(const uint8_t &num, FILE *out)
{
  fputc(num, out);
}

inline void writeUint16(const uint16_t &num, FILE *out)
{
#if 0
  /*  */
#elif ( BYTE_ORDER == LITTLE_ENDIAN )
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
#elif ( BYTE_ORDER == BIG_ENDIAN )
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
#else
#  error "unsupported endianness"
#endif
}

inline void writeUint32(const uint32_t &num, FILE *out)
{
#if 0
  /*  */
#elif ( BYTE_ORDER == LITTLE_ENDIAN )
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 24) & 0xff, out);
#elif ( BYTE_ORDER == BIG_ENDIAN )
  fputc((num >> 24) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
#else
#  error "unsupported endianness"
#endif
}

#endif

