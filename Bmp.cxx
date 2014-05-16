#include "rendera.h"

Bmp::Bmp()
{
  main = new Bitmap(256, 256);
  main->clear(makecol(255, 255, 255));
}

Bmp::~Bmp()
{
}

