#include "rendera.h"

Fl_Menu_Item menuitems[] =
{
  { "&File", 0, 0, 0, FL_SUBMENU },
    { "&New", 0, 0, 0 },
    { 0 },
  { "&Edit", 0, 0, 0, FL_SUBMENU },
    { "&Copy", 0, 0, 0 },
    { "&Paste", 0, 0, 0 },
    { 0 },
  { "&Help", 0, 0, 0, FL_SUBMENU },
    { "&About", 0, 0, 0 },
    { 0 },
  { 0 }
};

Gui::Gui()
{
  int x1, y1;

  // window
  window = new Fl_Double_Window(800, 600, "Rendera");
  menubar = new Fl_Menu_Bar(0, 0, window->w(), 24);
  menubar->menu(menuitems);

  // top_left
  top_left = new Fl_Group(0, menubar->h(), 112, 40);
  top_left->box(FL_UP_FRAME);
  logo = new Widget(top_left, 8, 8, 96, 24, "", "data/logo.png", 0, 0);
  top_left->resizable(0);
  top_left->end();

  // top right
  top_right = new Fl_Group(top_left->w(), menubar->h(), window->w() - top_left->w(), 40);
  top_right->box(FL_UP_FRAME);
  x1 = 8;
  zoom_fit = new Button(top_right, x1, 8, 24, 24, "Fit", "data/zoom_fit.png");
  x1 += 24 + 8;
  zoom_one = new Button(top_right, x1, 8, 24, 24, "Actual Size", "data/zoom_one.png");
  x1 += 24 + 8;
  zoom_in = new Button(top_right, x1, 8, 24, 24, "Zoom In", "data/zoom_in.png");
  x1 += 24 + 8;
  zoom_out = new Button(top_right, x1, 8, 24, 24, "Zoom Out", "data/zoom_out.png");
  x1 += 24 + 8;
  display = new Widget(top_right, x1, 8, 72, 24, "Display Mode", "data/display.png", 24, 24);
  x1 += 72 + 8;
  gridx = new Field(top_right, x1, 8, 48, 24, "Grid X");
  x1 += 48 + 8;
  gridy = new Field(top_right, x1, 8, 48, 24, "Grid Y");
  top_right->resizable(0);
  top_right->end();

  // bottom
  bottom = new Fl_Group(112, window->h() - 40, window->w() - 224, 40);
  bottom->box(FL_UP_FRAME);
  x1 = 8;
  wrap = new Widget(bottom, x1, 8, 48, 24, "Wrap", "data/wrap.png", 24, 24);
  x1 += 48 + 8;
  clone = new Widget(bottom, x1, 8, 49, 24, "Clone", "data/clone.png", 24, 24);
  x1 += 48 + 8;
  mirror = new Widget(bottom, x1, 8, 96, 24, "Clone Mirroring", "data/mirror.png", 24, 24);
  x1 += 96 + 8;
  origin = new Widget(bottom, x1, 8, 48, 24, "Origin", "data/origin.png", 24, 24);
  x1 += 48 + 8;
  constrain = new Widget(bottom, x1, 8, 48, 24, "Constrain", "data/constrain.png", 24, 24);
  bottom->resizable(0);
  bottom->end();

  // left top
  left_top = new Fl_Group(0, top_right->h() + menubar->h(), 112, 264);
  left_top->box(FL_UP_FRAME);
  y1 = 8;
  brush = new Widget(left_top, 8, y1, 96, 96, "Brush Preview", 0, 0);
  y1 += 96 + 8;
  size = new Widget(left_top, 8, y1, 96, 24, "Size", "data/size.png", 8, 24);
  y1 += 24 + 8;
  stroke = new Widget(left_top, 8, y1, 96, 48, "Stroke", "data/stroke.png", 24, 24);
  y1 += 48 + 8;
  shape = new Widget(left_top, 8, y1, 96, 24, "Shape", "data/shape.png", 24, 24);
  y1 += 24 + 8;
  edge = new Widget(left_top, 8, y1, 96, 24, "Soft Edge", "data/edge.png", 8, 24);
  left_top->resizable(0);
  left_top->end();

  // left bottom
  left_bottom = new Fl_Group(0, top_right->h() + menubar->h() + left_top->h(), 112, window->h() - (menubar->h() + top_right->h() + left_top->h()));
  left_bottom->box(FL_UP_FRAME);
  y1 = 8;
  offset = new Button(left_bottom, 8, y1, 96, 24, "Offset", "data/offset.png");
  y1 += 24 + 8;
  getcolor = new Button(left_bottom, 8, y1, 96, 24, "Get Color", "data/getcolor.png");
  y1 += 24 + 8;
  crop = new Button(left_bottom, 8, y1, 96, 24, "Crop", "data/crop.png");
  left_bottom->resizable(0);
  left_bottom->end();

  // right
  right = new Fl_Group(window->w() - 112, top_right->h() + menubar->h(), 112, window->h() - top_right->h() - menubar->h());
  right->box(FL_UP_FRAME);
  y1 = 8;
  palette = new Widget(right, 8, y1, 96, 96, "Palette", 16, 16);
  y1 += 96 + 8;
  trans = new Widget(right, 8, y1, 96, 24, "Transparency", "data/transparency.png", 8, 24);
  y1 += 24 + 8;
  blend = new Widget(right, 8, y1, 96, 24, "Blending Mode", "data/blend.png", 24, 24);
  right->resizable(0);
  right->end();

  // view
  view = new Fl_Group(112, top_right->h() + menubar->h(), window->w() - 224, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  view->box(FL_UP_FRAME);
  view->resizable(0);
  view->end();

  // container for top panels
  group_top = new Fl_Group(0, menubar->h(), window->w() - top_left->w(), 40);
  group_top->add(top_left);
  group_top->add(top_right);
  group_top->resizable(top_right);
  group_top->end();

  // container for left panels
  group_left = new Fl_Group(0, top_right->h() + menubar->h(), 112, window->h() - (menubar->h() + top_right->h() + bottom->h()));
  group_left->add(left_top);
  group_left->add(left_bottom);
  group_left->resizable(left_bottom);
  group_left->end();

  window->size_range(640, 480, 0, 0, 0, 0, 0);
  window->resizable(view);
  window->end();
  window->show();
  Fl_Tooltip::enable(1);
  Fl_Tooltip::color(fl_rgb_color(192, 224, 248));
  Fl_Tooltip::textcolor(FL_BLACK);

}

Gui::~Gui()
{
}

