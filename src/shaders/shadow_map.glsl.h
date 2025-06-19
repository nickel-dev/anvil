#ifndef __SHADER_SOURCE_SHADOW_MAP_H_
#define __SHADER_SOURCE_SHADOW_MAP_H_
static const char *__shader_source_shadow_map = "#ifdef VERTEX_SHADER\n\nlayout (location = 0) in vec3 position;\nlayout (location = 1) in vec2 uv0;\nlayout (location = 2) in vec4 color0;\nlayout (location = 3) in vec3 normal0;\n\nuniform mat4 view_light;\nuniform mat4 xform;\n\nvoid main() {\n	gl_Position = view_light * xform * vec4(position, 1.0);\n}\n\n#else\n\nvoid main() {\n	gl_FragDepth = gl_FragCoord.z;\n}\n\n#endif";
#endif