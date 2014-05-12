#ifndef GUI_H
#define GUI_H

#include "rendera.h"

class Gui
{
public:
  Gui();
  virtual ~Gui();

  // window
  Fl_Double_Window *window;
  Fl_Menu_Bar *menubar;

  // containers
  Fl_Group *group_top;
  Fl_Group *group_left;

  // panels
  Fl_Group *top_left;
  Fl_Group *top_right;
  Fl_Group *left_top;
  Fl_Group *left_bottom;
  Fl_Group *right;
  Fl_Group *bottom;
  Fl_Group *view;

  // top left
  Widget *logo;

  //top right
  Button *zoom_fit;
  Button *zoom_one;
  Button *zoom_in;
  Button *zoom_out;
  Widget *display;
  Field *gridx;
  Field *gridy;

  // left top
  Widget *brush;
  Widget *size;
  Widget *stroke;
  Widget *shape;
  Widget *edge;

  // left bottom
  Button *offset;
  Button *getcolor;
  Button *crop;

  // right
  Widget *palette;
  Widget *trans;
  Widget *blend;

  // bottom
  Widget *wrap;
  Widget *clone;
  Widget *mirror;
  Widget *origin;
  Widget *constrain;
};

#endif

