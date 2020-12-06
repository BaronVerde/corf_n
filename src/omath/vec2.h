
#pragma once

#include "common.h"

typedef struct vec2f {
	float x;
	float y;
} vec2f;

typedef struct vec2d {
	double x;
	double y;
} vec2d;

extern vec2f* vec2f_set_from( const vec2f* const other, vec2f* out );
extern vec2d* vec2d_set_from( const vec2d* const other, vec2d* out );
#define vec2_set_from( A, B )		\
	_Generic( (A),					\
			vec2f*: vec2f_set_from,	\
			vec2d*: vec2d_set_from	\
			) ( (A), (B) )

extern void vec2f_print( const vec2f* const v );
extern void vec2d_print( const vec2d* const v );
#define vec2_print( A )			\
	_Generic( (A),				\
			vec2f*: vec2f_print,\
			vec2d*: vec2d_print	\
			) ( (A) )
