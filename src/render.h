#ifndef RENDER_H
#define RENDER_H

#include "base.h"
#include "core.h"

//
// render
//

typedef enum render_mode_e {
	RENDER_MODE_NONE,
	RENDER_MODE_POINTS,
	RENDER_MODE_LINES,
	RENDER_MODE_LINE_LOOP,
	RENDER_MODE_LINE_STRIP,
	RENDER_MODE_TRIANGLES,
	RENDER_MODE_TRIANGLE_STRIP,
	RENDER_MODE_TRIANGLE_FAN
} render_mode_e;

typedef struct render_state {
    bool8_t depth_testing, blendig, face_culling, wireframe;
} render_state_t;

typedef struct render_statistics {
    uint32_t draw_calls, vertices, indices;
} render_statistics_t;

void render_init(os_event_t *event);
void render_close();

void render_clear(vec3_t color);

void render_state_set(render_state_t state);
render_state_t render_state_get();

void render_statistics_monitor(render_statistics_t *stats);
void render_statistics_stop();


//
// texture
//

typedef enum texture_filter {
    TEXTURE_FILTER_NEAREST,
    TEXTURE_FILTER_LINEAR
} texture_filter_e;

typedef enum texture_wrap {
    TEXTURE_WRAP_REPEAT,
    TEXTURE_WRAP_CLAMP_TO_EDGE,
    TEXTURE_WRAP_MIRRORED_REPEAT
} texture_wrap_e;

typedef struct texture_params {
    texture_filter_e min_filter;
    texture_filter_e mag_filter;
    texture_wrap_e wrap_s;
    texture_wrap_e wrap_t;
    bool8_t generate_mipmaps;
} texture_params_t;

typedef struct texture {
    uint32_t id;
    int32_t width, height, channels;
    texture_params_t params;
} texture_t;

texture_t texture_create(uint8_t *data, int32_t width, int32_t height, int32_t channels, texture_params_t params);
texture_t texture_load(string_t path, texture_params_t params);
void texture_delete(texture_t *texture);
void texture_bind(texture_t *texture, uint32_t slot);
void texture_unbind(uint32_t slot);
void texture_unpack_alignment(uint32_t alignment);


//
// mesh
//

typedef struct mesh {
    uint32_t vertex_count, curr_vertex;
    vertex_t *vertices;
    uint32_t index_count, curr_index;
    uint32_t *indices;
    render_mode_e mode;
} mesh_t;

mesh_t mesh_create(uint32_t vertex_count, uint32_t index_count);
mesh_t mesh_load(string_t path);
void mesh_delete(mesh_t *mesh);

void mesh_clear(mesh_t *mesh);
void mesh_draw(mesh_t *mesh);
void mesh_draw_instanced(mesh_t *mesh, uint32_t count);
void mesh_draw_vertices(mesh_t *mesh);

void mesh_push_vertex(mesh_t *mesh, vertex_t vertex);
void mesh_push_index(mesh_t *mesh, uint32_t index);
void mesh_push_vertices(mesh_t *mesh, vertex_t *vertices, uint32_t count);
void mesh_push_indices(mesh_t *mesh, uint32_t *indices, uint32_t count);

//
// shaders
//

#define SHADER_SOURCE(n) (string_t)(__shader_source_##n)

typedef uint32_t shader_t;

shader_t shader_create(string_t source);
void shader_delete(shader_t shader);
void shader_bind(shader_t shader);
void shader_unbind();

void shader_uniform_matrix(shader_t shader, string_t name, matrix_t matrix);
void shader_uniform_texture(shader_t shader, string_t name, uint32_t slot);
void shader_uniform_vec3(shader_t shader, string_t name, vec3_t vec);
void shader_uniform_vec2(shader_t shader, string_t name, vec2_t vec);


//
// framebuffers
//

typedef enum framebuffer_type {
    FRAMEBUFFER_COLOR,
    FRAMEBUFFER_DEPTH,
    FRAMEBUFFER_STENCIL
} framebuffer_type_e;

typedef struct framebuffer {
    uint32_t id;
    int32_t width, height;
    texture_params_t params;
    framebuffer_type_e type;
    texture_t texture;
} framebuffer_t;

framebuffer_t framebuffer_create(int32_t width, int32_t height, texture_params_t params, framebuffer_type_e type);
void framebuffer_delete(framebuffer_t *framebuffer);
void framebuffer_bind(framebuffer_t *framebuffer);
void framebuffer_unbind();

#endif // RENDER_H