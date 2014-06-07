#include "rendera.h"

void load(Fl_Widget *, void *)
{
  Fl_File_Chooser *chooser = new Fl_File_Chooser(".", "Legacy Bitmap Files (*.bmp)", Fl_File_Chooser::SINGLE, "Load Image");
  chooser->show();
  while(chooser->shown())
  {
    Fl::wait();
  }
  delete chooser;
}

