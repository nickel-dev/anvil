#include "base.h"
#include "core.h"
#include "ui.h"
#include "render.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

//
// fonts
//

struct font {
	texture_t texture;
	stbtt_packedchar chars[128];
	int32_t bitmap_width, bitmap_height;
};

font_o *font_create(uint8_t *data, int32_t bitmap_width, int32_t bitmap_height, texture_params_t params) {
	font_o *font = malloc(sizeof(font_o));
	font->bitmap_width = bitmap_width;
	font->bitmap_height = bitmap_height;
	
	stbtt_fontinfo font_into;
	if (!stbtt_InitFont(&font_into, data, 0)) {
		os_message(OS_MESSAGE_ERROR, "Failed to create font info");
		return NULL;
	}
	
	// create bitmap atlas
	uint8_t *bitmap = calloc(bitmap_width * bitmap_height, sizeof(uint8_t));
	
	float32_t scale = stbtt_ScaleForPixelHeight(&font_into, 24.0f);
	
	stbtt_pack_context pc;
	stbtt_PackBegin(&pc, bitmap, bitmap_width, bitmap_height, 0, 1, NULL);
	stbtt_PackFontRange(&pc, data, 0, 24.0f, 32, 96, font->chars);
	stbtt_PackEnd(&pc);
	
	// create texture
	font->texture = texture_create(bitmap, bitmap_width, bitmap_height, 1, params);
	
	return font;
}

font_o *font_load(string_t path, int32_t bitmap_width, int32_t bitmap_height, texture_params_t params) {
	uint8_t *data;
	FILE *file = fopen(path, "rb");
	if (!file) {
	__font_load_exit:
		os_message(OS_MESSAGE_ERROR, "Failed to open font file\nPath: %s", path);
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	uint64_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	data = malloc(size);
	if (!data) {
		goto __font_load_exit;
	}
	
	fread(data, 1, size, file);
	fclose(file);
	
	font_o *font = font_create(data, bitmap_width, bitmap_height, params);
	
	free(data);
	return font;
}

void font_delete(font_o *font) {
	if (font) {
		texture_delete(&font->texture);
		free(font);
	}
}


//
// ui
//

global ui_style_t _style;
global shader_t _shader_text, _shader_rect;
global mesh_t _mesh;
global matrix_t _projection;
global os_event_t *_event;

// style
ui_style_t ui_style_get() {
	return _style;
}

void ui_style_set(ui_style_t style) {
	_style = style;
}

// ui
void ui_init() {
	_style = (ui_style_t) {
#if OS_WINDOWS
		.font = font_load("C:/Windows/Fonts/Arialbd.ttf", 1024, 1024, ZERO_STRUCT(texture_params_t)),
#else
		.font = font_load("/usr/share/fonts/Adwaita/AdwaitaSans-Regular.ttf", 1024, 1024, ZERO_STRUCT(texture_params_t)),
#endif
		.text_color    = (vec4_t){ 1.0f, 1.0f, 1.0f, 1.0f },
		.fill_color    = (vec4_t){ 0.2f, 0.2f, 0.2f, 1.0f },
		.hover_color   = (vec4_t){ 0.2f, 0.2f, 0.4f, 1.0f },
		.outline_color = (vec4_t){ 1.0f, 1.0f, 1.0f, 1.0f },
		.margin        = 10.0f
	};
	
	const string_t _shader_text_source = "#ifdef VERTEX_SHADER\n\nlayout (location = 0) in vec3 position;\nlayout (location = 1) in vec2 uv0;\nlayout (location = 2) in vec4 color0;\nlayout (location = 3) in vec3 normal0;\n\nuniform mat4 projection;\nuniform vec2 offset;\n\nout vec2 uv;\nout vec4 color;\n\nvoid main() {\n	gl_Position = projection * vec4(position + vec3(offset, 0.0f), 1.0);\n	uv = uv0;\n	color = color0;\n}\n\n#else\n\nuniform sampler2D texture0;\n\nin vec2 uv;\nin vec4 color;\n\nvoid main() {\n	gl_FragColor = vec4(texture(texture0, uv).r) * color;\n}\n\n#endif";
	
	const string_t _shader_rect_source = "#ifdef VERTEX_SHADER\n\nlayout (location = 0) in vec3 position;\nlayout (location = 1) in vec2 uv0;\nlayout (location = 2) in vec4 color0;\nlayout (location = 3) in vec3 normal0;\n\nuniform mat4 projection;\n\nout vec4 color;\n\nvoid main() {\n	gl_Position = projection * vec4(position, 1.0);\n	color = color0;\n}\n\n#else\n\nin vec4 color;\n\nvoid main() {\n	gl_FragColor = color;\n}\n\n#endif";
	
	_shader_text = shader_create(_shader_text_source);
	_shader_rect = shader_create(_shader_rect_source);
	
	_mesh = mesh_create(4, 6);
}

void ui_close() {
	shader_delete(_shader_text);
	shader_delete(_shader_rect);
	mesh_delete(&_mesh);
}

void ui_event_push(os_event_t *event) {
	_event = event;
	_projection = matrix_projection_ortho(-_event->width / 2, _event->width / 2, -_event->height / 2, _event->height / 2, -1.0f, 1.0f);
}

// text
void ui_text(string_t text, vec2_t pos, float32_t scale, ui_anchor_e anchor) {
    if (!text || !*text) return;  // Early exit for empty strings
    
    render_state_t old_render_state = render_state_get();
    render_state_set((render_state_t){
        .depth_testing = false,
        .blending = true,
        .face_culling = true,
        .wireframe = false
    });
    
    shader_bind(_shader_text);
    texture_bind(&_style.font->texture, 0);
    shader_uniform_texture(_shader_text, "texture0", 0);
    shader_uniform_matrix(_shader_text, "projection", _projection);
    
    // Calculate total width (including advances between characters)
    float32_t total_width = 0.0f;
    for (string_t c = text; *c; ++c) {
        stbtt_packedchar *ch = &_style.font->chars[*c - 32];
        total_width += ch->xadvance * scale;
    }
    
    // Adjust horizontal position based on anchor
    switch (anchor) {
        case UI_ANCHOR_CENTER:
            pos.x -= total_width * 0.5f;
            break;
            
        case UI_ANCHOR_RIGHT:
            pos.x -= total_width;
            break;
            
        case UI_ANCHOR_LEFT:
        default:
            break;  // No adjustment needed for left anchor
    }
    
    // Render each character
    for (string_t c = text; *c; ++c) {
        stbtt_packedchar *ch = &_style.font->chars[*c - 32];
        
        if (*c == ' ') {  // Handle space characters efficiently
            pos.x += ch->xadvance * scale;
            continue;
        }
        
        // Calculate position (note: ypos calculation maintains bottom alignment)
        float32_t xpos = pos.x + ch->xoff * scale;
        float32_t ypos = pos.y - (ch->yoff + ch->y1 - ch->y0) * scale;
        
        // Calculate dimensions
        float32_t w = (ch->x1 - ch->x0) * scale;
        float32_t h = (ch->y1 - ch->y0) * scale;
        
        // Skip rendering for zero-sized characters
        if (w <= 0.0f || h <= 0.0f) {
            pos.x += ch->xadvance * scale;
            continue;
        }
        
        // Calculate UV coordinates
        float32_t u0 = ch->x0 / (float32_t)_style.font->bitmap_width;
        float32_t v0 = ch->y0 / (float32_t)_style.font->bitmap_height;
        float32_t u1 = ch->x1 / (float32_t)_style.font->bitmap_width;
        float32_t v1 = ch->y1 / (float32_t)_style.font->bitmap_height;
        
        // Define quad vertices
        uint32_t indices[6] = { 0, 1, 3, 1, 2, 3 };
        vertex_t vertices[4] = {
            { { xpos,     ypos + h, 0.0f }, { u0, v0 }, _style.text_color, {0} },
            { { xpos,     ypos,     0.0f }, { u0, v1 }, _style.text_color, {0} },
            { { xpos + w, ypos,     0.0f }, { u1, v1 }, _style.text_color, {0} },
            { { xpos + w, ypos + h, 0.0f }, { u1, v0 }, _style.text_color, {0} }
        };
        
        // Render the character
        mesh_clear(&_mesh);
        mesh_push_vertices(&_mesh, vertices, 4);
        mesh_push_indices(&_mesh, indices, 6);
        mesh_draw(&_mesh);
        
        // Advance to next character position
        pos.x += ch->xadvance * scale;
    }
    
    render_state_set(old_render_state);
}

// button
bool8_t ui_button(string_t text, vec2_t pos, vec2_t scale, ui_anchor_e button_anchor, ui_anchor_e text_anchor) {
	render_state_t old_render_state = render_state_get();
	render_state_set((render_state_t){ .depth_testing = false, .blending = true, .face_culling = true, .wireframe = false });
	
	float32_t m = _style.margin;
	
	// anchoring the button
	switch (button_anchor) {
	case UI_ANCHOR_CENTER:
		pos.x -= scale.x / 2;
		break;
		
	case UI_ANCHOR_RIGHT:
		pos.x -= scale.x;
		break;
		
	case UI_ANCHOR_LEFT:
	default:
		break;
	}
	
	// interaction
	vec2_t cursor = mul2(_event->cursor, (vec2_t){ _event->width / 2, _event->height / 2 });
	bool8_t hovering = ((cursor.x > pos.x - m && cursor.x < pos.x + m + scale.x) &&
						(cursor.y > pos.y - m && cursor.y < pos.y + m + scale.y));
	
	vec4_t color = (hovering ? _style.hover_color : _style.fill_color);
	
	// rendering
	shader_bind(_shader_rect);
	shader_uniform_matrix(_shader_rect, "projection", _projection);
	
	{
		uint32_t indices[6] = { 0, 1, 3, 1, 2, 3 };
		vertex_t vertices[4] = {
			{ (vec3_t){ pos.x - m,       pos.y + scale.y, 0.0f }, (vec2_t){ 0.0f, 0.0f }, color, ZERO_STRUCT(vec3_t) },
			{ (vec3_t){ pos.x - m,       pos.y - m,       0.0f }, (vec2_t){ 0.0f, 1.0f }, color, ZERO_STRUCT(vec3_t) },
			{ (vec3_t){ pos.x + scale.x, pos.y - m,       0.0f }, (vec2_t){ 1.0f, 1.0f }, color, ZERO_STRUCT(vec3_t) },
			{ (vec3_t){ pos.x + scale.x, pos.y + scale.y, 0.0f }, (vec2_t){ 1.0f, 0.0f }, color, ZERO_STRUCT(vec3_t) }
		};
		
		mesh_clear(&_mesh);
		mesh_push_vertices(&_mesh, (vertex_t *)vertices, 4);
		mesh_push_indices(&_mesh, (uint32_t *)indices, 6);
		mesh_draw(&_mesh);
	}
	
	// outline
	{
		vec4_t color = _style.outline_color;
		uint32_t indices[6] = { 0, 1, 2, 3 };
        vertex_t vertices[4] = {
            { (vec3_t){ pos.x - m,       pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x - m,       pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x + scale.x, pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x + scale.x, pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) }
        };

		mesh_clear(&_mesh);
        _mesh.mode = RENDER_MODE_LINE_LOOP;
		mesh_push_vertices(&_mesh, (vertex_t *)vertices, 4);
		mesh_push_indices(&_mesh, (uint32_t *)indices, 4);
		mesh_draw(&_mesh);
		_mesh.mode = RENDER_MODE_NONE;
	}
	
	if (text) {
		switch (text_anchor) {
		case UI_ANCHOR_CENTER:
			ui_text(text, (vec2_t){ pos.x + scale.x / 2, pos.y }, 1.0f, UI_ANCHOR_CENTER);
			break;
			
		case UI_ANCHOR_RIGHT:
			ui_text(text, (vec2_t){ pos.x + scale.x, pos.y }, 1.0f, UI_ANCHOR_RIGHT);
			break;
			
		case UI_ANCHOR_LEFT:
		default:
			ui_text(text, pos, 1.0f, UI_ANCHOR_LEFT);
			break;
		};
	}
	
	render_state_set(old_render_state);
	return mouse_button_pressed(MOUSE_BUTTON_LEFT) && hovering;
}

bool8_t ui_slider(string_t text, vec2_t pos, vec2_t scale, float32_t *value, float32_t min, float32_t max, ui_anchor_e slider_anchor, ui_anchor_e text_anchor) {
    render_state_t old_render_state = render_state_get();
    render_state_set((render_state_t){ .depth_testing = false, .blending = true, .face_culling = true, .wireframe = false });
    
	float32_t m = _style.margin;
	
    // anchoring the button
    switch (slider_anchor) {
	case UI_ANCHOR_CENTER:
        pos.x -= scale.x / 2;
        break;
        
	case UI_ANCHOR_RIGHT:
        pos.x -= scale.x;
        break;
        
	case UI_ANCHOR_LEFT:
	default:
        break;
    }
    
    // interaction
    vec2_t cursor = mul2(_event->cursor, (vec2_t){ _event->width / 2, _event->height / 2 });
    bool8_t hovering = ((cursor.x > pos.x - m - 5 && cursor.x < pos.x + m + scale.x + 5) &&
                        (cursor.y > pos.y - m - 5 && cursor.y < pos.y + m + scale.y + 5));
    
    // Calculate percentage based on the value range
    float32_t percentage = (*value - min) / (max - min);
    if (mouse_button_down(MOUSE_BUTTON_LEFT) && hovering) {
        float32_t offset = cursor.x - (pos.x - m);
        percentage = offset / (scale.x + m);
        *value = min + percentage * (max - min);  // Scale to the actual range
        CLAMP(*value, min, max);
    }
	
	CLAMP(percentage, 0.0f, 1.0f);
    
    // rendering
    shader_bind(_shader_rect);
    shader_uniform_matrix(_shader_rect, "projection", _projection);
    
    {
        uint32_t indices[6] = { 0, 1, 3, 1, 2, 3 };
        vertex_t vertices[4] = {
            { (vec3_t){ pos.x - m,       pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), _style.fill_color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x - m,       pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), _style.fill_color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x + scale.x, pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), _style.fill_color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x + scale.x, pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), _style.fill_color, ZERO_STRUCT(vec3_t) }
        };
        
        mesh_clear(&_mesh);
        mesh_push_vertices(&_mesh, (vertex_t *)vertices, 4);
        mesh_push_indices(&_mesh, (uint32_t *)indices, 6);
        mesh_draw(&_mesh);
    }
    
    {
        float32_t w = percentage * (scale.x + m);
        uint32_t indices[6] = { 0, 1, 3, 1, 2, 3 };
        vertex_t vertices[4] = {
            { (vec3_t){ pos.x - m,     pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), _style.hover_color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x - m,     pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), _style.hover_color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x - m + w, pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), _style.hover_color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x - m + w, pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), _style.hover_color, ZERO_STRUCT(vec3_t) }
        };
        
        mesh_clear(&_mesh);
        mesh_push_vertices(&_mesh, (vertex_t *)vertices, 4);
        mesh_push_indices(&_mesh, (uint32_t *)indices, 6);
        mesh_draw(&_mesh);
    }
    
	// outline
	{
		vec4_t color = _style.outline_color;
		uint32_t indices[6] = { 0, 1, 2, 3 };
        vertex_t vertices[4] = {
            { (vec3_t){ pos.x - m,       pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x - m,       pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x + scale.x, pos.y - m,       0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) },
            { (vec3_t){ pos.x + scale.x, pos.y + scale.y, 0.0f }, ZERO_STRUCT(vec2_t), color, ZERO_STRUCT(vec3_t) }
        };
        
        mesh_clear(&_mesh);
        _mesh.mode = RENDER_MODE_LINE_LOOP;
		mesh_push_vertices(&_mesh, (vertex_t *)vertices, 4);
		mesh_push_indices(&_mesh, (uint32_t *)indices, 4);
		mesh_draw(&_mesh);
		_mesh.mode = RENDER_MODE_NONE;
	}
	
    if (text) {
        switch (text_anchor) {
		case UI_ANCHOR_CENTER:
            ui_text(text, (vec2_t){ pos.x + scale.x / 2, pos.y }, 1.0f, UI_ANCHOR_CENTER);
            break;
            
		case UI_ANCHOR_RIGHT:
            ui_text(text, (vec2_t){ pos.x + scale.x, pos.y }, 1.0f, UI_ANCHOR_RIGHT);
            break;
            
		case UI_ANCHOR_LEFT:
		default:
            ui_text(text, pos, 1.0f, UI_ANCHOR_LEFT);
            break;
        };
    }
    
    render_state_set(old_render_state);
    return mouse_button_down(MOUSE_BUTTON_LEFT) && hovering;
}
