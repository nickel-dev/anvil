#ifndef UI_H
#define UI_H

#include "base.h"
#include "core.h"
#include "render.h"

//
// fonts
//

typedef struct font font_o;

font_o *font_create(uint8_t *data, int32_t bitmap_width, int32_t bitmap_height, texture_params_t params);
font_o *font_load(string_t path, int32_t bitmap_width, int32_t bitmap_height, texture_params_t params);
void font_delete(font_o *font);


//
// ui
//

typedef struct ui_style {
	font_o *font;
	vec4_t text_color, fill_color, hover_color, outline_color;
	float32_t margin;
} ui_style_t;

ui_style_t ui_style_get();
void ui_style_set(ui_style_t style);

void ui_init();
void ui_close();
void ui_event_push(os_event_t *event);

typedef enum ui_anchor {
	UI_ANCHOR_LEFT,
	UI_ANCHOR_CENTER,
	UI_ANCHOR_RIGHT
} ui_anchor_e;

void ui_text(string_t text, vec2_t pos, float32_t scale, ui_anchor_e anchor);
bool8_t ui_button(string_t text, vec2_t pos, vec2_t scale, ui_anchor_e button_anchor, ui_anchor_e text_anchor);
bool8_t ui_slider(string_t text, vec2_t pos, vec2_t scale, float32_t *value, float32_t min, float32_t max, ui_anchor_e slider_anchor, ui_anchor_e text_anchor);

#endif // UI_H
