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

#ifndef COMMON_H
#define COMMON_H

// need to recompile program if this file changes

static int seed = 12345;

static inline int rnd32(void)
{
  seed = (seed << 17) ^ (seed >> 13) ^ (seed << 5);
  return seed;
}

static inline int makecol(const int r, const int g, const int b)
{
  return r | g << 8 | b << 16 | 0xFF000000;
}

static inline int makecola(const int r, const int g, const int b, const int a)
{
  return r | g << 8 | b << 16 | a << 24;
}

static inline int makecol_notrans(const int r, const int g, const int b)
{
  return r | g << 8 | b << 16;
}

static inline int geta(const int c)
{
  return (c >> 24) & 255;
}

static inline int getr(const int c)
{
  return c & 255;
}

static inline int getg(const int c)
{
  return (c >> 8) & 255;
}

static inline int getb(const int c)
{
  return (c >> 16) & 255;
}

static inline int getv(const int c)
{
  return (getr(c) + getg(c) + getb(c)) / 3;
}

static inline int getl(const int c)
{
  return ((54 * getr(c)) + (182 * getg(c)) + (19 * getb(c))) / 255;
}

static inline int SCALE(const int a, const int b)
{
  return (a * (255 - b) / 255) + b;
}

static inline int diff24(const int c1, const int c2)
{
  const int r = getr(c1) - getr(c2);
  const int g = getg(c1) - getg(c2);
  const int b = getb(c1) - getb(c2);

  return r * r + g * g + b * b;
}

static inline int blend_fast_solid(const int c1, const int c2, const int t)
{
  const int rb =
    (((((c1 & 0xFF00FF) - (c2 & 0xFF00FF)) * t) >> 8) + c2) & 0xFF00FF;
  const int g = (((((c1 & 0xFF00) - (c2 & 0xFF00)) * t) >> 8) + c2) & 0xFF00;

  return rb | g | 0xFF000000;
}

static inline int blend_fast_xor(const int c1, const int t)
{
  const int c2 = c1 ^ 0x00FFFFFF;
  const int rb =
    (((((c1 & 0xFF00FF) - (c2 & 0xFF00FF)) * t) >> 8) + c2) & 0xFF00FF;
  const int g = (((((c1 & 0xFF00) - (c2 & 0xFF00)) * t) >> 8) + c2) & 0xFF00;

  return rb | g | 0xFF000000;
}

static inline int convert_format(int c, int bgr_order)
{
  if(bgr_order)
    return makecol(getb(c), getg(c), getr(c));
  else
    return c;
}

static inline uint8_t parse_uint8(unsigned char *&buffer)
{
  uint8_t num = buffer[0];

  buffer += 1;
  return num;
}

static uint16_t parse_uint16(unsigned char *&buffer)
{
  uint16_t num;

//  #if BYTE_ORDER == BIG_ENDIAN
//  num = buffer[1] | buffer[0] << 8;
//  #else
  num = buffer[0] | buffer[1] << 8;
//  #endif

  buffer += 2;
  return num;
}

static uint32_t parse_uint32(unsigned char *&buffer)
{
  uint32_t num;

//  #if BYTE_ORDER == BIG_ENDIAN
//  num = buffer[3] | buffer[2] << 8 | buffer[1] << 16 | buffer[0] << 24;
//  #else
  num = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
//  #endif

  buffer += 4;
  return num;
}

static void write_uint8(uint8_t num, FILE *out)
{
  fputc(num, out);
}

static void write_uint16(uint16_t num, FILE *out)
{
//  #if BYTE_ORDER == BIG_ENDIAN
//  fputc((num >> 8) & 0xff, out);
//  fputc(num & 0xff, out);
//  #else
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
//  #endif
}

static void write_uint32(uint32_t num, FILE *out)
{
//  #if BYTE_ORDER == BIG_ENDIAN
//  fputc((num >> 24) & 0xff, out);
//  fputc((num >> 16) & 0xff, out);
//  fputc((num >> 8) & 0xff, out);
//  fputc(num & 0xff, out);
//  #else
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 24) & 0xff, out);
//  #endif
}

#endif

