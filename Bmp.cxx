#include "rendera.h"

Bmp::Bmp()
{
  main = new Bitmap(1000, 1000);
  main->clear(makecol(255, 255, 255));
  preview = new Bitmap(8, 8);
}

Bmp::~Bmp()
{
}

