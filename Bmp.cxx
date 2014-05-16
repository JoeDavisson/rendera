#include "rendera.h"

Bmp::Bmp()
{
  main = new Bitmap(64, 64);
  main->clear(makecol(255, 255, 255));
}

Bmp::~Bmp()
{
}

