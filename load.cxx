#include "rendera.h"

void load(Fl_Widget *, void *)
{
  Fl_File_Chooser *fc = new Fl_File_Chooser(".", "JPEG Image (*.jpg)", Fl_File_Chooser::SINGLE, "Load Image");
  fc->show();
  while(fc->shown())
  {
    Fl::wait();
  }
  const char *fn = fc->value();
  puts(fn);
}

