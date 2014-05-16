#include "rendera.h"

Bmp::Bmp()
{
  main = new Bitmap(800, 600);
  main->clear(makecol(255, 255, 255));
}

Bmp::~Bmp()
{
}

