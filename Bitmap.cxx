#include "rendera.h"

Bitmap::Bitmap(int width, int height)
{
  int i;

  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int[width * height];
  row = new int [height];

  w = width;
  h = height;

  cx = 0;
  cy = 0;
  cw = width;
  ch = height;

  for(i = 0; i < height; i++)
    row[i] = width * i;
}

Bitmap::~Bitmap()
{
  delete row;
  delete data;
}

void Bitmap::clear(int c)
{
  int i;
  for(i = 0; i < w * h; i++)
    data[i] = c;
}

void Bitmap::setpixel_solid(int x, int y, int c2, int t)
{
  if(x < cx || x >= cx + cw || y < cy || y >= cy + ch)
    return;

  int c1 = data[row[y] + x];

  data[row[y] + x] = (*blend->mode)(c1, c2, t);
}

void Bitmap::setpixel_wrap(int x, int y, int c2, int t)
{
  while(x < cx)
    x += cw;
  while(x >= cx + cw)
    x -= cw;
  while(y < cy)
    y += ch;
  while(y >= cy + ch)
    y -= ch;

  int c1 = data[row[y] + x];

  data[row[y] + x] = (*blend->mode)(c1, c2, t);
}

void Bitmap::setpixel_clone(int x, int y, int c2, int t)
{
  if(x < cx || x >= cx + cw || y < cy || y >= cy + ch)
    return;

  int c1 = getpixel(x, y);

  int x1 = x - var->deltax;
  int y1 = y - var->deltay;

  int w1 = w - 1;
  int h1 = h - 1;

  switch(var->mirror)
  {
    case 0:
      x1 = x1;
      y1 = y1;
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = y1;
      break;
    case 2:
      x1 = x1;
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
  }

  if(x1 >= var->strokex1 && x1 < var->strokex2 && y1 >= var->strokey1)
    c2 = bmp->clone->getpixel(x1 - var->strokex1, y1 - var->strokey1);
  else
    c2 = bmp->main->getpixel(x1, y1);

  data[row[y] + x] = (*blend->mode)(c1, c2, t);
}

void Bitmap::setpixel_wrap_clone(int x, int y, int c2, int t)
{
  while(x < cx)
    x += cw;
  while(x >= cx + cw)
    x -= cw;
  while(y < cy)
    y += ch;
  while(y >= cy + ch)
    y -= ch;

  int c1 = data[row[y] + x];

  int x1 = x - var->deltax;
  int y1 = y - var->deltay;

  int w1 = w - 1;
  int h1 = h - 1;

  switch(var->mirror)
  {
    case 0:
      x1 = x1;
      y1 = y1;
      break;
    case 1:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = y1;
      break;
    case 2:
      x1 = x1;
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
    case 3:
      x1 = (w1 - x1) - (w1 - var->clonex * 2);
      y1 = (h1 - y1) - (h1 - var->cloney * 2);
      break;
  }

  if(x1 >= var->strokex1 && x1 < var->strokex2 && y1 >= var->strokey1)
    c2 = bmp->clone->getpixel(x1 - var->strokex1, y1 - var->strokey1);
  else
    c2 = bmp->clone->getpixel(x1, y1);

  data[row[y] + x] = (*blend->mode)(c1, c2, t);
}

int Bitmap::getpixel(int x, int y)
{
  if(var->wrap)
  {
    while(x < cx)
      x += cw;
    while(x >= cx + cw)
      x -= cw;
    while(y < cy)
      y += ch;
    while(y >= cy + ch)
      y -= ch;
  }
  else
  {
    if(x < cx)
      x = cx;
    if(x > cx + cw - 1)
      x = cx + cw - 1;
    if(y < cy)
      y = cy;
    if(y > cy + ch - 1)
      y = cy + ch - 1;
  }

  return data[row[y] + x];
}

void Bitmap::clip(int *x1, int *y1, int *x2, int *y2)
{
  if(*x1 < cx)
    *x1 = cx;
  if(*y1 < cy)
    *y1 = cy;
  if(*x2 >= cx + cw)
    *x2 = cx + cw - 1;
  if(*y2 >= cy + ch)
    *y2 = cy + ch - 1;
}

void Bitmap::set_clip_rect(int x, int y, int w, int h)
{
  cx = x;
  cy = y;
  cw = w;
  ch = h;
}

