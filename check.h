#ifndef CHECK_H
#define CHECK_H

#include "rendera.h"

void check_palette(Widget *, void *);
void check_zoom_in(Button *, void *);
void check_zoom_out(Button *, void *);
void check_zoom_fit(ToggleButton *, void *);
void check_zoom_one(Button *, void *);
void check_grid(ToggleButton *, void *);
void check_gridx(Field *, void *);
void check_gridy(Field *, void *);
void check_size(Widget *, void *);
void check_stroke(Widget *, void *);
void check_edge(Widget *, void *);
void check_smooth(Widget *, void *);
void check_color(Widget *, void *);
void check_wrap(Widget *, void *);
void check_clone(Widget *, void *);
void check_mirror(Widget *, void *);
void check_origin(Widget *, void *);
void check_constrain(Widget *, void *);

void show_about();
void hide_about();
void show_new_image();
void hide_new_image();
void cancel_new_image();

#endif

