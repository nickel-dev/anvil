#ifndef MATH_H
#define MATH_H

#include "base.h"


//
// random
//

void random_seed(int32_t seed);
int32_t random();
int32_t random_range(int32_t low, int32_t high);


//
// vectors
//

vec2_t vec2_scalar(float32_t s);
vec3_t vec3_scalar(float32_t s);
vec4_t vec4_scalar(float32_t s);

vec2_t add2(vec2_t a, vec2_t b);
vec2_t sub2(vec2_t a, vec2_t b);
vec2_t mul2(vec2_t a, vec2_t b);
vec2_t div2(vec2_t a, vec2_t b);

vec3_t add3(vec3_t a, vec3_t b);
vec3_t sub3(vec3_t a, vec3_t b);
vec3_t mul3(vec3_t a, vec3_t b);
vec3_t div3(vec3_t a, vec3_t b);

vec4_t add4(vec4_t a, vec4_t b);
vec4_t sub4(vec4_t a, vec4_t b);
vec4_t mul4(vec4_t a, vec4_t b);
vec4_t div4(vec4_t a, vec4_t b);

float32_t length2(vec2_t vec);
float32_t length3(vec3_t vec);
float32_t length4(vec4_t vec);

float32_t distance2(vec2_t a, vec2_t b);
float32_t distance3(vec3_t a, vec3_t b);
float32_t distance4(vec4_t a, vec4_t b);

vec2_t lerp2(vec2_t a, vec2_t b, float32_t t);
vec3_t lerp3(vec3_t a, vec3_t b, float32_t t);
vec4_t lerp4(vec4_t a, vec4_t b, float32_t t);

vec2_t normalize2(vec2_t vec);
vec3_t normalize3(vec3_t vec);
vec4_t normalize4(vec4_t vec);

float32_t cross2(vec2_t a, vec2_t b);
vec3_t cross3(vec3_t a, vec3_t b);

float32_t dot2(vec2_t a, vec2_t b);
float32_t dot3(vec3_t a, vec3_t b);
float32_t dot4(vec4_t a, vec4_t b);

vec2_t mix2(vec2_t a, vec2_t b, float32_t f);
vec3_t mix3(vec3_t a, vec3_t b, float32_t f);
vec4_t mix4(vec4_t a, vec4_t b, float32_t f);


//
// collision
//

bool8_t circle_vs_circle(circle_t a, circle_t b);
bool8_t point_vs_aabb(vec2_t a, range2_t b);


//
// matrices
//

#define IDENTITY_MATRIX (matrix_t){{ \
{ 1.0f, 0.0f, 0.0f, 0.0f }, \
{ 0.0f, 1.0f, 0.0f, 0.0f }, \
{ 0.0f, 0.0f, 1.0f, 0.0f }, \
{ 0.0f, 0.0f, 0.0f, 1.0f }  \
}}

#define matrix_scalar(s) (matrix_t){{ \
{ s, s, s, s }, \
{ s, s, s, s }, \
{ s, s, s, s }, \
{ s, s, s, s }  \
}}

matrix_t matrix_projection_ortho(float32_t left, float32_t right, float32_t bottom, float32_t top, float32_t znear, float32_t zfar);
matrix_t matrix_projection_perspective(float32_t fov, float32_t aspect, float32_t znear, float32_t zfar);

matrix_t matrix_mul(matrix_t a, matrix_t b);

// xform transformations
matrix_t xform_translate(matrix_t matrix, vec3_t translation);
matrix_t xform_scale(matrix_t matrix, vec3_t scale);
matrix_t xform_rotate(matrix_t matrix, vec3_t axis, float32_t radians);
matrix_t xform_lookat(vec3_t eye, vec3_t target, vec3_t up);
matrix_t xform_transform(transform_t transform);
matrix_t xform_camera(vec3_t pos, vec3_t rot);

#endif // MATH_H