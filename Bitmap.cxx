#include "rendera.h"

Bitmap::Bitmap(int width, int height)
{
  int i;

  if(width < 1)
    width = 1;
  if(height < 1)
    height = 1;

  data = new int[width * height * 4];
  row = new int *[height * sizeof(int *)];

  x = 0;
  y = 0;
  w = width;
  h = height;

  cx = 0;
  cy = 0;
  cw = width;
  ch = height;

  for(i = 0; i < height; i++)
    row[i] = &data[width * i];
}

Bitmap::~Bitmap()
{
  delete row;
  delete data;
}

void Bitmap::clear(int color)
{
  int i;
  for(i = 0; i < w * h * 4; i++)
    data[i] = color;
}

