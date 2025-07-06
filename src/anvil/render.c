#include "base.h"
#include "core.h"
#include "render.h"
#include <glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define _RENDER_FILTER(x) (((x) == TEXTURE_FILTER_NEAREST) ? (GL_NEAREST) : (GL_LINEAR))
#define _RENDER_WRAP(x) (((x) == TEXTURE_WRAP_REPEAT) ? (GL_REPEAT) : (((x) == TEXTURE_WRAP_CLAMP_TO_EDGE) ? (GL_CLAMP_TO_EDGE) : (GL_MIRRORED_REPEAT)))

//
// render
//

#define MAX_VERTEX_COUNT 4096
#define MAX_INDEX_COUNT  4096

global uint32_t vao, vbo, ebo;
global render_statistics_t *_statistics;
global render_state_t _state;
global os_event_t *_event;

void render_init(os_event_t *event) {
    _event = event;
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(vertex_t), NULL, GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDEX_COUNT * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
	
	// vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, normal));
	glEnableVertexAttribArray(3);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_close() {
    glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void render_clear(vec3_t color) {
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    if (_statistics) {
        ZERO_MEMORY(_statistics);
    }
}

void render_state_set(render_state_t state) {
    (state.depth_testing ? glEnable : glDisable) (GL_DEPTH_TEST);
    (state.blending ? glEnable : glDisable) (GL_BLEND);
    (state.face_culling ? glEnable : glDisable) (GL_CULL_FACE);
	glPolygonMode( GL_FRONT_AND_BACK, (state.wireframe ? GL_LINE : GL_FILL));
    _state = state;
}

render_state_t render_state_get() {
    return _state;
}

void render_statistics_monitor(render_statistics_t *stats) {
    _statistics = stats;
}

void render_statistics_stop() {
    _statistics = NULL;
}

void render_point_size(float32_t size) {
	glPointSize(size);
}

void render_line_width(float32_t width) {
	glLineWidth(width);
}


//
// texture
//

texture_t texture_create(uint8_t *data, int32_t width, int32_t height, int32_t channels, texture_params_t params) {
	texture_t t = { 0, width, height, channels, params };
	
	if (!data) {
		os_message(OS_MESSAGE_WARNING, "Texture data cannot be NULL");
		return t;
	}
	
	uint32_t mode = 0;
	switch (channels) {
		case 1: mode = GL_RED;  break;
		case 2: mode = GL_RG;   break;
		case 3: mode = GL_RGB;  break;
		default:
		case 4: mode = GL_RGBA; break;
	}
	
	glGenTextures(1, &t.id);
	glBindTexture(GL_TEXTURE_2D, t.id);
	
	glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, data);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _RENDER_FILTER(params.min_filter));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _RENDER_FILTER(params.mag_filter));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _RENDER_WRAP(params.wrap_s));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _RENDER_WRAP(params.wrap_t));
	
	if (params.generate_mipmaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return t;
}

texture_t texture_load(string_t path, texture_params_t params) {
	int32_t width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	uint8_t *data = stbi_load(path, &width, &height, &channels, 0);
	
	if (!data) {
		os_message(OS_MESSAGE_WARNING, "Failed to load texture\nPath: %s", path);
		return ZERO_STRUCT(texture_t);
	}
	
	return texture_create(data, width, height, channels, params);
}

void texture_delete(texture_t *texture) {
	if (texture->id) {
		glDeleteTextures(1, &texture->id);
		ZERO_MEMORY(texture);
	}
}

void texture_bind(texture_t *texture, uint32_t slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture->id);
}

void texture_unbind(uint32_t slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_unpack_alignment(uint32_t alignment) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}


//
// mesh
//

mesh_t mesh_create(uint32_t vertex_count, uint32_t index_count) {
    mesh_t m = { 0 };
    m.vertex_count = vertex_count;
    m.index_count  = index_count;
    m.vertices = malloc(vertex_count * sizeof(vertex_t));
    m.indices  = malloc(index_count  * sizeof(uint32_t));
    return m;
}

internal mesh_t _process_node(struct aiNode *node, const struct aiScene *scene) {
	UNUSED(node);
	struct aiMesh *ai_mesh = scene->mMeshes[0];
	
	uint32_t index_count = 0;
	for (uint32_t i = 0; i < ai_mesh->mNumFaces; ++i) {
		index_count += ai_mesh->mFaces[i].mNumIndices;
	}
	
	mesh_t mesh = mesh_create(ai_mesh->mNumVertices, index_count);
	
	// Vertices
	for (uint32_t i = 0; i < ai_mesh->mNumVertices; i++) {
		vertex_t vertex = { 0 };
		
		vertex.pos = (vec3_t) {
			ai_mesh->mVertices[i].x,
			ai_mesh->mVertices[i].y,
			ai_mesh->mVertices[i].z
		};
		
		vertex.normal = (vec3_t) {
			ai_mesh->mNormals[i].x,
			ai_mesh->mNormals[i].y,
			ai_mesh->mNormals[i].z
		};
		
		if (ai_mesh->mColors[0]) {
			vertex.color = (vec4_t) {
				ai_mesh->mColors[0][i].r,
				ai_mesh->mColors[0][i].g,
				ai_mesh->mColors[0][i].b,
				ai_mesh->mColors[0][i].a
			};
		} else {
			vertex.color = vec4_scalar(1.0f);
		}
		
		if (ai_mesh->mTextureCoords[0]) {
			vertex.uv = (vec2_t) {
				ai_mesh->mTextureCoords[0][i].x,
				ai_mesh->mTextureCoords[0][i].y
			};
		} else {
			vertex.uv = ZERO_STRUCT(vec2_t);
		}
		
		mesh_push_vertex(&mesh, vertex);
	}
	
	// Indices
	for (uint32_t i = 0; i < ai_mesh->mNumFaces; ++i) {
		struct aiFace face = ai_mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; ++j) {
			mesh_push_index(&mesh, (uint32_t)face.mIndices[j]);
		}
	}
	
	return mesh;
}

mesh_t mesh_load(string_t path) {
	const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
											   aiProcess_OptimizeMeshes | aiProcess_GenNormals |
											   aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		os_message(OS_MESSAGE_ERROR, "Failed to load mesh\nPath: %s", path);
		return ZERO_STRUCT(mesh_t);
	}
	
	mesh_t m = _process_node(scene->mRootNode, scene);
	aiReleaseImport(scene);
	
	return m;
}

void mesh_delete(mesh_t *mesh) {
	free(mesh->vertices);
	free(mesh->indices);
}

void mesh_clear(mesh_t *mesh) {
	mesh->curr_index = 0;
	mesh->curr_vertex = 0;
}

void mesh_draw(mesh_t *mesh) {
	uint32_t mode = (uint32_t)mesh->mode;
	if (!mode) {
		mode = GL_TRIANGLES;
	} else {
		mode += GL_POINTS - 1;
	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->curr_vertex * sizeof(vertex_t), (void *)mesh->vertices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->curr_index * sizeof(uint32_t), (void *)mesh->indices);
	glDrawElements(mode, mesh->curr_index, GL_UNSIGNED_INT, NULL);
	
	if (_statistics) {
		++_statistics->draw_calls;
		_statistics->vertices += mesh->vertex_count;
		_statistics->indices += mesh->index_count;
	}
}

void mesh_draw_instanced(mesh_t *mesh, uint32_t count) {
	uint32_t mode = (uint32_t)mesh->mode;
	if (!mode) {
		mode = GL_TRIANGLES;
	} else {
		mode += GL_POINTS - 1;
	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->curr_vertex * sizeof(vertex_t), (void *)mesh->vertices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->curr_index * sizeof(uint32_t), (void *)mesh->indices);
	glDrawElementsInstanced(GL_TRIANGLES, mesh->curr_index, GL_UNSIGNED_INT, NULL, count);
	
	if (_statistics) {
		++_statistics->draw_calls;
		_statistics->vertices += mesh->vertex_count * count;
		_statistics->indices += mesh->index_count * count;
	}
}

void mesh_draw_vertices(mesh_t *mesh) {
	uint32_t mode = (uint32_t)mesh->mode;
	if (!mode) {
		mode = GL_TRIANGLES;
	} else {
		mode += GL_POINTS - 1;
	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->curr_vertex * sizeof(vertex_t), (void *)mesh->vertices);
	glDrawArrays(mode, 0, mesh->curr_index);
	
	if (_statistics) {
		++_statistics->draw_calls;
		_statistics->vertices += mesh->vertex_count;
		_statistics->indices += mesh->vertex_count;
	}
}

void mesh_push_vertex(mesh_t *mesh, vertex_t vertex) {
	mesh->vertices[mesh->curr_vertex] = vertex;
	++mesh->curr_vertex;
}

void mesh_push_index(mesh_t *mesh, uint32_t index) {
	mesh->indices[mesh->curr_index] = index;
	++mesh->curr_index;
}

void mesh_push_vertices(mesh_t *mesh, vertex_t *vertices, uint32_t count) {
	memcpy(&mesh->vertices[mesh->curr_vertex], vertices, sizeof(vertex_t) * count);
	mesh->curr_vertex += count;
}

void mesh_push_indices(mesh_t *mesh, uint32_t *indices, uint32_t count) {
	memcpy(&mesh->indices[mesh->curr_index], indices, sizeof(uint32_t) * count);
	mesh->curr_index += count;
}

//
// shaders
//

internal inline int32_t gl_location(shader_t shader, string_t name) {
	return glGetUniformLocation(shader, name);
}

shader_t shader_create(string_t source) {
	shader_t program = 0;
	int32_t error = 0;
	
	// create 2 programs from a single shader source
	const string_t vert_source[2] = {"#version 330 core\n#define VERTEX_SHADER 1\n", source};
	const string_t frag_source[2] = {"#version 330 core\n#define FRAGMENT_SHADER 1\n", source};
	
	// create the shader modules
	shader_t vert_module = glCreateShader(GL_VERTEX_SHADER);
	shader_t frag_module = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(vert_module, 2, (const GLchar *const*)vert_source, NULL);
	glShaderSource(frag_module, 2, (const GLchar *const*)frag_source, NULL);
	glCompileShader(vert_module);
	glCompileShader(frag_module);
	
	// vertex shader
	glGetShaderiv(vert_module, GL_COMPILE_STATUS, &error);
	if (!error) {
		int32_t length = 0;
		glGetShaderiv(vert_module, GL_INFO_LOG_LENGTH, &length);
		
		GLchar* info = (GLchar*)malloc(length * sizeof(GLchar));
		glGetShaderInfoLog(vert_module, length * sizeof(GLchar), NULL, info);
		
		fprintf(stderr, "%s\n", info);
		free(info);
	}
	
	// fragment shader
	glGetShaderiv(frag_module, GL_COMPILE_STATUS, &error);
	if (!error) {
		int32_t length = 0;
		glGetShaderiv(frag_module, GL_INFO_LOG_LENGTH, &length);
		
		GLchar* info = (GLchar*)malloc(length * sizeof(GLchar));
		glGetShaderInfoLog(frag_module, length * sizeof(GLchar), NULL, info);
		
		fprintf(stderr, "%s\n", info);
		free(info);
	}
	
	// create the OpenGL shader
	program = glCreateProgram();
	
	glAttachShader(program, vert_module);
	glAttachShader(program, frag_module);
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &error);
	if (!error) {
		int32_t length = 0;
		glGetProgramiv(frag_module, GL_INFO_LOG_LENGTH, &length);
		
		GLchar* info = (GLchar*)malloc(length * sizeof(GLchar));
		glGetProgramInfoLog(frag_module, length * sizeof(GLchar), NULL, info);
		
		fprintf(stderr, "%s\n", info);
		free(info);
	}
	
	// cleanup
	glDetachShader(program, vert_module);
	glDetachShader(program, frag_module);
	glDeleteShader(vert_module);
	glDeleteShader(frag_module);
	
	return program;
}

shader_t shader_load(string_t path) {
	string_t source = os_read_entire_file(path);
	shader_t shader = shader_create(source);
	free(source);
	return shader;
}

void shader_delete(shader_t shader) {
	glDeleteProgram(shader);
}

void shader_bind(shader_t shader) {
	glUseProgram(shader);
}

void shader_unbind() {
	glUseProgram(0);
}

// uniforms
void shader_uniform_matrix(shader_t shader, string_t name, matrix_t matrix) {
	glUniformMatrix4fv(gl_location(shader, name), 1, GL_FALSE, matrix.elements[0]);
}

void shader_uniform_texture(shader_t shader, string_t name, uint32_t slot) {
	glUniform1i(gl_location(shader, name), slot);
}

void shader_uniform_vec3(shader_t shader, string_t name, vec3_t vec) {
	glUniform3fv(gl_location(shader, name), 1, &vec.x);
}

void shader_uniform_vec2(shader_t shader, string_t name, vec2_t vec) {
	glUniform2fv(gl_location(shader, name), 1, &vec.x);
}


//
// framebuffer
//

framebuffer_t framebuffer_create(int32_t width, int32_t height, texture_params_t params, framebuffer_type_e type) {
	framebuffer_t framebuffer = { 0, width, height, params, type, ZERO_STRUCT(texture_t) };
	
	glGenFramebuffers(1, &framebuffer.id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
	
	texture_t texture = { 0 };
	texture.width = width;
	texture.height = height;
	texture.params = params;
	
	uint32_t internal_format, format, attachment_type;
	
	switch (type) {
		default:
		case FRAMEBUFFER_COLOR:
		internal_format = GL_RGBA8;
		format = GL_RGBA;
		attachment_type = GL_COLOR_ATTACHMENT0;
		break;
		
		case FRAMEBUFFER_DEPTH:
		internal_format = GL_DEPTH_COMPONENT;
		format = GL_DEPTH_COMPONENT;
		attachment_type = GL_DEPTH_ATTACHMENT;
		break;
		
		case FRAMEBUFFER_STENCIL:
		internal_format = GL_STENCIL_INDEX8;
		format = GL_STENCIL_INDEX;
		attachment_type = GL_STENCIL_ATTACHMENT;
		break;
	}
	
	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _RENDER_FILTER(texture.params.min_filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _RENDER_FILTER(texture.params.mag_filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _RENDER_WRAP(texture.params.wrap_s));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _RENDER_WRAP(texture.params.wrap_t));
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, texture.id, 0);
	
	framebuffer.texture = texture;
	
	// check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		os_message(OS_MESSAGE_ERROR, "Framebuffer is not complete");
		framebuffer_delete(&framebuffer);
		return ZERO_STRUCT(framebuffer_t);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return framebuffer;
}

void framebuffer_delete(framebuffer_t *framebuffer) {
	if (framebuffer->id) {
		glDeleteFramebuffers(1, &framebuffer->id);
		texture_delete(&framebuffer->texture);
		ZERO_MEMORY(framebuffer);
	}
}

void framebuffer_bind(framebuffer_t *framebuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
}

void framebuffer_unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _event->width, _event->height);
}
