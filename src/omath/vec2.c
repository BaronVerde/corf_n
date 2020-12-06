
#include "vec2.h"
#include <stdio.h>

inline vec2f* vec2f_set_from( const vec2f* const other, vec2f* out ) {
	out->x = other->x; out->y = other->y;
	return out;
}

inline vec2d* vec2d_set_from( const vec2d* const other, vec2d* out ) {
	out->x = other->x; out->y = other->y;
	return out;
}

inline void vec2f_print( const vec2f* const v ) {
	printf( "(%f/%f)", v->x, v->y );
}

inline void vec2d_print( const vec2d* const v ) {
	printf( "(%lf/%lf)", v->x, v->y );
}
