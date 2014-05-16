#ifndef INLINE_H
#define INLINE_H

static int seed;

static inline int rnd32(void)
{
  seed = (seed << 17) ^ (seed >> 13) ^ (seed << 5);
  return seed;
}

static inline int makecol(const int r, const int g, const int b)
{
  return b | g << 8 | r << 16 | 0xFF << 24;
}

static inline int makecola(const int r, const int g, const int b, const int a)
{
  return b | g << 8 | r << 16 | a << 24;
}

static inline int geta(const int c)
{
  return (c >> 24) & 255;
}

static inline int getr(const int c)
{
  return (c >> 16) & 255;
}

static inline int getg(const int c)
{
  return (c >> 8) & 255;
}

static inline int getb(const int c)
{
  return c & 255;
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

static inline int blend_fast_ex(const int c1, const int c2, const int t)
{
  const int rb =
    (((((c1 & 0xFF00FF) - (c2 & 0xFF00FF)) * t) >> 8) + c2) & 0xFF00FF;
  const int g = (((((c1 & 0xFF00) - (c2 & 0xFF00)) * t) >> 8) + c2) & 0xFF00;

  return rb | g;
}

#endif

