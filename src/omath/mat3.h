
#pragma once

#include "vec3.h"

typedef struct {
	float data[9];
} mat3f;

typedef struct {
	double data[9];
} mat3d;

//extern mat3f *mat3f_mul( const mat3f *const a, const mat3f *const b, mat3f *out );

//extern mat3f *mat3f_mul_s( const mat3f *const a, const float s, mat3f *out );

extern mat3f *mat3f_div_s( const mat3f *const a, const float s, mat3f *out );

extern mat3f *mat3f_inverse( const mat3f *const in, mat3f *out );

extern mat3f *mat3f_transpose( const mat3f *const in, mat3f *out );
