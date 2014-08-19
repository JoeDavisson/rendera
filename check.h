#ifndef CHECK_H
#define CHECK_H

#include "rendera.h"

void update_color(int);
void check_palette(Widget *, void *);
void check_zoom_in(Button *, void *);
void check_zoom_out(Button *, void *);
void check_zoom_fit(ToggleButton *, void *);
void check_zoom_one(Button *, void *);
void check_zoom();
void check_grid(ToggleButton *, void *);
void check_gridx(Field *, void *);
void check_gridy(Field *, void *);
void check_paint_size(Widget *, void *);
void check_paint_shape(Widget *, void *);
void check_paint_stroke(Widget *, void *);
void check_paint_edge(Widget *, void *);
void check_paint_smooth(Widget *, void *);
void check_tool(Widget *, void *);
void check_color(Widget *, void *);
void check_hue(Widget *, void *);
void check_sat(Widget *, void *);
void check_val(Widget *, void *);
void check_trans(Widget *, void *);
void check_blend(Widget *, void *);
void check_wrap(Widget *, void *);
void check_clone(Widget *, void *);
void check_mirror(Widget *, void *);
void check_origin(Widget *, void *);
void check_constrain(Widget *, void *);
void check_crop_do();
void check_crop_values();
void check_rgba();
void check_indexed();

#endif

