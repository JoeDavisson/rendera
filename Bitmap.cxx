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

  set_clip(0, 0, w, h);

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
  if(x < cl || x > cr || y < ct || y > cb)
    return;

  int c1 = data[row[y] + x];

  data[row[y] + x] = (*blend->mode)(c1, c2, t);
}

void Bitmap::setpixel_wrap(int x, int y, int c2, int t)
{
  while(x < cl)
    x += cw;
  while(x > cr)
    x -= cw;
  while(y < ct)
    y += ch;
  while(y > cb)
    y -= ch;

  int c1 = data[row[y] + x];

  data[row[y] + x] = (*blend->mode)(c1, c2, t);
}

void Bitmap::setpixel_clone(int x, int y, int c2, int t)
{
  if(x < cl || x > cr || y < ct || y > cb)
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
  while(x < cl)
    x += cw;
  while(x > cr)
    x -= cw;
  while(y < ct)
    y += ch;
  while(y > cb)
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
    while(x < cl)
      x += cw;
    while(x > cr)
      x -= cw;
    while(y < ct)
      y += ch;
    while(y > cb)
      y -= ch;
  }
  else
  {
    if(x < cl)
      x = cl;
    if(x > cr)
      x = cr;
    if(y < ct)
      y = ct;
    if(y > cb)
      y = cb;
  }

  return data[row[y] + x];
}

void Bitmap::clip(int *x1, int *y1, int *x2, int *y2)
{
  if(*x1 < cl)
    *x1 = cl;
  if(*x2 > cr)
    *x2 = cr;
  if(*y1 < ct)
    *y1 = ct;
  if(*y2 > cb)
    *y2 = cb;
}

void Bitmap::set_clip(int x, int y, int w, int h)
{
  cl = x;
  ct = y;
  cr = w - 1;
  ch = h - 1;
  cw = w;
  ch = h;
}

void Bitmap::blit(Bitmap *dest, int sx, int sy, int dx, int dy, int ww, int hh)
{
  int x, y;

   if((sx >= w) || (sy >= h) ||
   (dx >= dest->cr) || (dy >= dest->cb))
     return;

   // clip src left
   if(sx < 0)
   {
     ww += sx;
     dx -= sx;
     sx = 0;
   }

   // clip src top
   if(sy < 0)
   {
     hh += sy;
     dy -= sy;
     sy = 0;
   }

   // clip src right
   if((sx + ww) > w)
     ww = w - sx;

   // clip src bottom
   if((sy + hh) > h)
     hh = h - sy;

   // clip dest left
   if(dx < dest->cl)
   {
     dx -= dest->cl;
     ww += dx;
     sx -= dx;
     dx = dest->cl;
   }

   // clip dest top
   if(dy < dest->ct)
   {
     dy -= dest->ct;
     hh += dy;
     sy -= dy;
     dy = dest->ct;
   }

   // clip dest right
   if((dx + ww - 1) > dest->cr)
     ww = dest->cr - dx;

   // clip dest bottom
   if((dy + hh - 1) > dest->cb)
     hh = dest->cb - dy;

   if(ww < 1 || hh < 1)
     return;

   int sy1 = sy;
   int dy1 = dy;
   for(y = 0; y < hh; y++)
   {
     int sx1 = sx + row[sy1];
     int dx1 = dx + dest->row[dy1];
     for(x = 0; x < ww; x++, sx1++, dx1++)
       dest->data[dx1] = data[sx1];
     sy1++;
     dy1++;
   }
 }

