#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv0;
layout (location = 2) in vec4 color0;
layout (location = 3) in vec3 normal0;

uniform mat4 view_light;
uniform mat4 xform;

void main() {
	gl_Position = view_light * xform * vec4(position, 1.0);
}

#else

void main() {
	gl_FragDepth = gl_FragCoord.z;
}

#endif