#include "base.h"
#include "math.h"

//
// random
//

global int32_t current_seed;

void random_seed(int32_t seed) {
    current_seed = seed;
}

int32_t random() {
    return (current_seed = (1664525 * current_seed + 1013904223) % 4294967296);
}

int32_t random_range(int32_t low, int32_t high) {
    low = MIN(low, high);
    high = MAX(low, high);
	return (random() % (high - low + 1)) + low;
}


//
// vectors
//

vec2_t vec2_scalar(float32_t s) { return (vec2_t){ s, s }; }
vec3_t vec3_scalar(float32_t s) { return (vec3_t){ s, s, s }; }
vec4_t vec4_scalar(float32_t s) { return (vec4_t){ s, s, s, s }; }

vec2_t add2(vec2_t a, vec2_t b) { return (vec2_t){ a.x + b.x, a.y + b.y }; }
vec2_t sub2(vec2_t a, vec2_t b) { return (vec2_t){ a.x - b.x, a.y - b.y }; }
vec2_t mul2(vec2_t a, vec2_t b) { return (vec2_t){ a.x * b.x, a.y * b.y }; }
vec2_t div2(vec2_t a, vec2_t b) { return (vec2_t){ a.x / b.x, a.y / b.y }; }

vec3_t add3(vec3_t a, vec3_t b) { return (vec3_t){ a.x + b.x, a.y + b.y, a.z + b.z }; }
vec3_t sub3(vec3_t a, vec3_t b) { return (vec3_t){ a.x - b.x, a.y - b.y, a.z - b.z }; }
vec3_t mul3(vec3_t a, vec3_t b) { return (vec3_t){ a.x * b.x, a.y * b.y, a.z * b.z }; }
vec3_t div3(vec3_t a, vec3_t b) { return (vec3_t){ a.x / b.x, a.y / b.y, a.z / b.z }; }

vec4_t add4(vec4_t a, vec4_t b) { return (vec4_t){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
vec4_t sub4(vec4_t a, vec4_t b) { return (vec4_t){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
vec4_t mul4(vec4_t a, vec4_t b) { return (vec4_t){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
vec4_t div4(vec4_t a, vec4_t b) { return (vec4_t){ a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }

float32_t length2(vec2_t vec) { return sqrtf(vec.x*vec.x + vec.y*vec.y); }
float32_t length3(vec3_t vec) { return sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z); }
float32_t length4(vec4_t vec) { return sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w); }

float32_t distance2(vec2_t a, vec2_t b) { return length2((vec2_t){ b.x - a.x, b.y - a.y }); }
float32_t distance3(vec3_t a, vec3_t b) { return length3((vec3_t){ b.x - a.x, b.y - a.y, b.z - a.z }); }
float32_t distance4(vec4_t a, vec4_t b) { return length4((vec4_t){ b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w }); }

// lerp
vec2_t lerp2(vec2_t a, vec2_t b, float32_t t) {
    vec2_t result;
    result.x = a.x + t * (b.x - a.x);
    result.y = a.y + t * (b.y - a.y);
    return result;
}

vec3_t lerp3(vec3_t a, vec3_t b, float32_t t) {
    vec3_t result;
    result.x = a.x + t * (b.x - a.x);
    result.y = a.y + t * (b.y - a.y);
    result.z = a.z + t * (b.z - a.z);
    return result;
}

vec4_t lerp4(vec4_t a, vec4_t b, float32_t t) {
    vec4_t result;
    result.x = a.x + t * (b.x - a.x);
    result.y = a.y + t * (b.y - a.y);
    result.z = a.z + t * (b.z - a.z);
    result.w = a.w + t * (b.w - a.w);
    return result;
}

// normalize
vec2_t normalize2(vec2_t vec) {
	float32_t l = length2(vec);
	return (vec2_t){ vec.x / l, vec.y / l };
}

vec3_t normalize3(vec3_t vec) {
	float32_t l = length3(vec);
	return (vec3_t){ vec.x / l, vec.y / l, vec.z / l };
}

vec4_t normalize4(vec4_t vec) {
	float32_t l = length4(vec);
	return (vec4_t){ vec.x / l, vec.y / l, vec.z / l, vec.w / l };
}

float32_t cross2(vec2_t a, vec2_t b) {
    return a.x * b.y - a.y * b.x;
}

// 3D cross product (returns a vector)
vec3_t cross3(vec3_t a, vec3_t b) {
    vec3_t result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// dot product
float32_t dot2(vec2_t a, vec2_t b) {
    return a.x * b.x + a.y * b.y;
}

float32_t dot3(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float32_t dot4(vec4_t a, vec4_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

vec2_t mix2(vec2_t a, vec2_t b, float32_t f) {
	f = fmaxf(0.0f, fminf(1.0f, f));
	return (vec2_t) {
        a.x * (1.0f - f) + b.x * f,
        a.y * (1.0f - f) + b.y * f
    };
}

vec3_t mix3(vec3_t a, vec3_t b, float32_t f) {
    f = fmaxf(0.0f, fminf(1.0f, f));
    return (vec3_t) {
        a.x * (1.0f - f) + b.x * f,
        a.y * (1.0f - f) + b.y * f,
        a.z * (1.0f - f) + b.z * f
    };
}

vec4_t mix4(vec4_t a, vec4_t b, float32_t f) {
    f = fmaxf(0.0f, fminf(1.0f, f));
    return (vec4_t) {
        a.x * (1.0f - f) + b.x * f,
        a.y * (1.0f - f) + b.y * f,
        a.z * (1.0f - f) + b.z * f,
        a.w * (1.0f - f) + b.w * f
    };
}


//
// collision
//

bool8_t circle_vs_circle(circle_t a, circle_t b) {
	return distance2(a.pos, b.pos) < a.radius + b.radius;
}

bool8_t point_vs_aabb(vec2_t a, range2_t b) {
	return (a.x < b.max.x && a.x > b.min.x) && (a.y < b.max.y && a.y > b.min.y);
}


//
// matrices
//

matrix_t matrix_projection_ortho(float32_t left, float32_t right, float32_t bottom, float32_t top,  float32_t znear, float32_t zfar) {
    matrix_t m = { 0 };
	
	float32_t width = right - left;
	float32_t height = zfar - znear;
	float32_t depth = top - bottom;
	
	if (width == 0.0f || height == 0.0f || depth == 0.0f) {
		return IDENTITY_MATRIX;
	}
	
	float32_t _1over_width = 1.0f / width;
	float32_t _1over_height = 1.0f / height;
	float32_t _1over_depth = 1.0f / depth;
	
	m.elements[0][0] = 2.0f * _1over_width;
	m.elements[1][1] = 2.0f * _1over_depth;
	m.elements[2][2] = -2.0f * _1over_height;
	
	m.elements[3][0] = -(right + left) *  _1over_width;
	m.elements[3][1]= -(top + bottom) *  _1over_depth;
	m.elements[3][2]= -(zfar + znear) * _1over_height;
	m.elements[3][3]= 1.0f;
	
	return m;
}

matrix_t matrix_projection_perspective(float32_t fov, float32_t aspect, float32_t znear, float32_t zfar) {
    matrix_t m = { 0 };
    float32_t f = 1.0f / tanf(DEG_TO_RAD(fov) / 2);
	
	m.elements[0][0] = f / aspect;
	m.elements[1][1] = f;
	m.elements[2][2] = ((zfar + znear) / (znear - zfar));
	m.elements[2][3] = -1.0f;
	m.elements[3][2] = (2.0f * zfar * znear) / (znear - zfar);
	
    return m;
}

matrix_t matrix_mul(matrix_t a, matrix_t b) {
	matrix_t m;
	
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			m.elements[i][j] =
				a.elements[i][0] * b.elements[0][j] +
				a.elements[i][1] * b.elements[1][j] +
				a.elements[i][2] * b.elements[2][j] +
				a.elements[i][3] * b.elements[3][j];
		}
	}
	
	return m;
}

// xform transformations
matrix_t xform_translate(matrix_t matrix, vec3_t translation) {
	matrix_t m = IDENTITY_MATRIX;
	
	m.elements[3][0] = translation.x;
	m.elements[3][1] = translation.y;
	m.elements[3][2] = translation.z;
    
	return matrix_mul(matrix, m);
}

matrix_t xform_scale(matrix_t matrix, vec3_t scale) {
	matrix_t m = IDENTITY_MATRIX;
	
    m.elements[0][0] = scale.x;
    m.elements[1][1] = scale.y;
    m.elements[2][2] = scale.z;
    m.elements[3][3] = 1.0f;
	
	return matrix_mul(matrix, m);
}

matrix_t xform_rotate(matrix_t matrix, vec3_t axis, float32_t radians) {
	matrix_t m = IDENTITY_MATRIX;
	
	float32_t c = cosf(radians);
    float32_t s = sinf(radians);
    float32_t t = 1.0f - c;
	
	m.elements[0][0] = c + axis.x * axis.x * t;
    m.elements[0][1] = axis.x * axis.y * t + axis.z * s;
    m.elements[0][2] = axis.x * axis.z * t - axis.y * s;
	
    m.elements[1][0] = axis.y * axis.x * t - axis.z * s;
    m.elements[1][1] = c + axis.y * axis.y * t;
    m.elements[1][2] = axis.y * axis.z * t + axis.x * s;
	
    m.elements[2][0] = axis.z * axis.x * t + axis.y * s;
    m.elements[2][1] = axis.z * axis.y * t - axis.x * s;
    m.elements[2][2] = c + axis.z * axis.z * t;
	
    m.elements[3][3] = 1.0f;
	
	return matrix_mul(matrix, m);
}

matrix_t xform_lookat(vec3_t eye, vec3_t target, vec3_t up) {
	matrix_t m = { 0 };
	vec3_t f = normalize3((vec3_t){ target.x - eye.x, target.y - eye.y, target.z - eye.z });
	vec3_t s = normalize3(cross3(f, up));
	
	up = cross3(s, f);
	
	m.elements[0][0] = s.x;
    m.elements[1][0] = s.y;
    m.elements[2][0] = s.z;
	
	m.elements[0][1] = up.x;
    m.elements[1][1] = up.y;
    m.elements[2][1] = up.z;
    
    m.elements[0][2] = -f.x;
    m.elements[1][2] = -f.y;
    m.elements[2][2] = -f.z;
    
    m.elements[3][3] = 1.0f;
	
	return xform_translate(m, (vec3_t){ -eye.x, -eye.y, -eye.z });
}

matrix_t xform_transform(transform_t transform) {
	matrix_t m = IDENTITY_MATRIX;
	m = xform_rotate(m, (vec3_t){ 1, 0, 0 }, transform.rot.x);
	m = xform_rotate(m, (vec3_t){ 0, 1, 0 }, transform.rot.y);
	m = xform_rotate(m, (vec3_t){ 0, 0, 1 }, transform.rot.z);
	m = xform_translate(m, transform.pos);
	m = xform_scale(m, transform.scale);
	return m;
}

matrix_t xform_camera(vec3_t pos, vec3_t rot) {
	matrix_t m = xform_translate(IDENTITY_MATRIX, (vec3_t) { -pos.x, -pos.y, -pos.z });
	
	m = xform_rotate(m, (vec3_t){ 0.0f, 0.0f, 1.0f }, DEG_TO_RAD(rot.z));
	m = xform_rotate(m, (vec3_t){ 0.0f, 1.0f, 0.0f }, DEG_TO_RAD(rot.y));
	m = xform_rotate(m, (vec3_t){ 1.0f, 0.0f, 0.0f }, DEG_TO_RAD(rot.x));
	
	return m;
}