#ifndef RENDERA_H
#define RENDERA_H

#define makecol(r, g, b)   ((b) | ((g) << 8) | ((r) << 16))
#define getb(c)            ((c) & 0xFF)
#define getg(c)            (((c) >> 8) & 0xFF)
#define getr(c)            (((c) >> 16) & 0xFF)

#define MIN(x, y)          (((x) < (y)) ? (x) : (y))
#define MAX(x, y)          (((x) > (y)) ? (x) : (y))
#define MID(a, b, c)       (MAX(a, MIN(b, c)))
#define SWAP(a, b)         { int c = (a); (a) = (b); (b) = c; }
#define SIGN(a)            (((a) > 0) ? 1 : -1)

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tooltip.H>

#include "inline.h"
#include "Bitmap.h"
#include "Var.h"
#include "Blend.h"
#include "Bmp.h"
#include "Map.h"
#include "View.h"
#include "Widget.h"
#include "Button.h"
#include "Field.h"
#include "Gui.h"

extern Gui *gui;
extern Var *var;
extern Bmp *bmp;
extern Blend *blend;

#endif

