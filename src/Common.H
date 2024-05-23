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

#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdint>

#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif

// stuff for extracting all RGBA values quickly
struct rgba_type
{
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  uint8_t a;
  uint8_t b;
  uint8_t g;
  uint8_t r;
#else
#error "unsupported endianness"
#endif
};

union un_rgba_type
{
  uint32_t uint32_;
  rgba_type rgba_;
};

rgba_type getRgba(const uint32_t);
int makeRgb(const int, const int, const int);
int makeRgba(const int, const int, const int, const int);
int makeRgb24(const int, const int, const int);
int getr(const int);
int getg(const int);
int getb(const int);
int geta(const int);
int getv(const int);
int getl(const int);
int getlUnpacked(const int, const int, const int);
int scaleVal(const int, const int);
int diff24(const int, const int);
int diff32(const int, const int);
int convertFormat(const int, const bool);
int getFltkColor(const int);
int clamp(const int, const int);
int range(const int, const int, const int);
int rnd();
uint8_t parseUint8(unsigned char *&);
uint16_t parseUint16(unsigned char *&);
uint32_t parseUint32(unsigned char *&);
void writeUint8(const uint8_t, FILE *);
void writeUint16(const uint16_t, FILE *);
void writeUint32(const uint32_t, FILE *);

#endif

