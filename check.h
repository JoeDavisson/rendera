#ifndef CHECK_H
#define CHECK_H

#include "rendera.h"

void update_color(int);
void check_palette(Widget *, void *);
void check_plus(Button *, void *);
void check_minus(Button *, void *);
void check_zoom_in(Button *, void *);
void check_zoom_out(Button *, void *);
void check_zoom_fit(ToggleButton *, void *);
void check_zoom_one(Button *, void *);
void check_zoom();
void check_grid(ToggleButton *, void *);
void check_gridx(Field *, void *);
void check_gridy(Field *, void *);
void check_paint_size(Widget *, void *);
void check_paint_stroke(Widget *, void *);
void check_airbrush_size(Widget *, void *);
void check_airbrush_stroke(Widget *, void *);
void check_airbrush_edge(Widget *, void *);
void check_airbrush_smooth(Widget *, void *);
void check_tool(Widget *, void *);
void check_color(Widget *, void *);
void check_wrap(Widget *, void *);
void check_clone(Widget *, void *);
void check_mirror(Widget *, void *);
void check_origin(Widget *, void *);
void check_constrain(Widget *, void *);
void check_crop();

void show_about();
void hide_about();
void show_new_image();
void hide_new_image();
void cancel_new_image();
void show_create_palette();
void hide_create_palette();
void cancel_create_palette();

#endif

