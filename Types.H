/* rendera/Types.H */


#ifndef ___RENDERA___TYPES_H___
#define ___RENDERA___TYPES_H___

struct point
{
  int x, y;
};

struct line
{
  point a, b;
};

bool operator==( point const&, point const& );

#endif//ndef ___RENDERA___TYPES_H___
