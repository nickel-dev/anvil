#ifndef BASE_H
#define BASE_H

//
// config macros
//

#if defined(_WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#define OS_WIN32 1
#endif

#if defined(unix) || defined(__unix) || defined(__unix__) || defined(linux)
#define OS_LINUX 1
#endif

#ifndef NOMINMAX
# define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#ifdef UNICODE
# undef UNICODE
#endif

#ifdef _UNICODE
# undef _UNICODE
#endif

#ifdef STRICT
# undef STRICT
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif


//
// libc base libraries
//

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <float.h>


//
// keywords/macros
//

#define internal static
#define global   static

#ifndef offsetof
# define offsetof(a, b) ((uint64_t)(&(((a *)(0))->b)))
#endif

#define ZERO_STRUCT(x) ((x){ 0 })
#define ZERO_MEMORY(x) (memset((x), 0, sizeof(*(x))))
#define DUPLICATE(a, b) do {        \
(a) = malloc(sizeof(*(b)));     \
memcpy((a), (b), sizeof(*(b))); \
} while (0)

#define CRASH() ((*(volatile int32_t *)(0)) = (0))

#define UNUSED(x) ((void)(x))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define WRAP(x, a, b)  ((x) = (((x) < (a)) ? (b) : (((x) > (b)) ? (a) : (x))))
#define CLAMP(x, a, b) ((x) = (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x))))
#define PI 3.1415926535
#define DEG_TO_RAD(x) ((x) * PI / 180)
#define RAD_TO_DEG(x) ((x) * 180 / PI)

#define BIT(x) (1 << (x))

//
// base types
//

// string type
typedef char *string_t;

// fixed width int
#if 0
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
#endif

// fixed with float
typedef float float32_t;
typedef double float64_t;
typedef long double float128_t;
typedef float128_t floatmax_t;

// fixed width booleans
typedef int8_t  bool8_t;
typedef int16_t bool16_t;
typedef int32_t bool32_t;
typedef int64_t bool64_t;
typedef bool64_t boolmax_t;

// vector types
typedef struct vec2 {
    float32_t x, y;
} vec2_t;

typedef struct vec3 {
    float32_t x, y, z;
} vec3_t;

typedef struct vec4 {
    float32_t x, y, z, w;
} vec4_t;

// pritmitives
typedef struct transform {
    vec3_t pos, rot, scale;
} transform_t;

typedef struct circle {
    vec2_t pos;
    float32_t radius;
} circle_t;

// ranges
typedef struct rangef { float32_t min, max; } rangef_t;
typedef struct range2 { vec2_t min, max; } range2_t;
typedef struct range3 { vec3_t min, max; } range3_t;
typedef struct range4 { vec4_t min, max; } range4_t;

// vertex
typedef struct vertex {
    vec3_t pos;
    vec2_t uv;
    vec4_t color;
    vec3_t normal;
} vertex_t;

// matrix
typedef union matrix {
    float32_t elements[4][4];
	float32_t values[16];
} matrix_t;

#endif // BASE_H