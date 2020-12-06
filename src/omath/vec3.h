
#pragma once

#include "common.h"

/*typedef struct {
	union {
		struct {
			int x;
			int y;
		};
		int data[2];
	};
} point_t;*/

typedef struct vec3f {
	float x;
	float y;
	float z;
} vec3f;

typedef struct {
	double x;
	double y;
	double z;
} vec3d;

extern vec3f* vec3f_add( const vec3f* const a, const vec3f* const b, vec3f *out );
extern vec3d* vec3d_add( const vec3d* const a, const vec3d* const b, vec3d *out );
#define vec3_add( A, B, C )		\
	_Generic( (A),				\
			vec3f*: vec3f_add,	\
			vec3d*: vec3d_add	\
			) ( (A), (B), (C) )

extern vec3f* vec3f_sub( const vec3f* const a, const vec3f* const b, vec3f *out );
extern vec3d* vec3d_sub( const vec3d* const a, const vec3d* const b, vec3d *out );
#define vec3_sub( A, B, C )		\
	_Generic( (A),				\
			vec3f*: vec3f_sub,	\
			vec3d*: vec3d_sub	\
			) ( (A), (B), (C) )

extern vec3f* vec3f_mul( const vec3f* const a, const vec3f* const b, vec3f *out );
extern vec3d* vec3d_mul( const vec3d* const a, const vec3d* const b, vec3d *out );
#define vec3_mul( A, B, C )		\
	_Generic( (A),				\
			vec3f*: vec3f_mul,	\
			vec3d*: vec3d_mul	\
			) ( (A), (B), (C) )

extern vec3f* vec3f_mul_s( const vec3f* const a, const float scalar, vec3f *out );
extern vec3d* vec3d_mul_s( const vec3d* const a, const double scalar, vec3d *out );
#define vec3_mul_s( A, B, C )		\
	_Generic( (A),				\
			vec3f*: vec3f_mul_s,	\
			vec3d*: vec3d_mul_s	\
			) ( (A), (B), (C) )

extern vec3f* vec3f_div( const vec3f* const a, const vec3f* const b, vec3f *out );
extern vec3d* vec3d_div( const vec3d* const a, const vec3d* const b, vec3d *out );
#define vec3_div( A, B, C )		\
	_Generic( (A),				\
			vec3f*: vec3f_div,	\
			vec3d*: vec3d_div	\
			) ( (A), (B), (C) )

extern vec3f* vec3f_div_s( const vec3f* const a, const float scalar, vec3f *out );
extern vec3d* vec3d_div_s( const vec3d* const a, const double scalar, vec3d *out );
#define vec3_div_s( A, B, C )	\
	_Generic( (A),				\
			vec3f*: vec3f_div_s,\
			vec3d*: vec3d_div_s	\
			) ( (A), (B), (C) )

extern float vec3f_magnitude( const vec3f* const a );
extern double vec3d_magnitude( const vec3d* const a );
#define vec3_magnitude( A )			\
	_Generic( (A),					\
			vec3f*: vec3f_magnitude,\
			vec3d*: vec3d_magnitude	\
			) ( (A) )

extern float vec3f_magnitude_sq( const vec3f* const a );
extern double vec3d_magnitude_sq( const vec3d* const a );
#define vec3_magnitude_sq( A )			\
	_Generic( (A),						\
			vec3f*: vec3f_magnitude_sq,	\
			vec3d*: vec3d_magnitude_sq	\
			) ( (A) )

extern float vec3f_dot( const vec3f* const a, const vec3f* const b );
extern double vec3d_dot( const vec3d* const a, const vec3d* const b );
#define vec3_dot( A, B )		\
	_Generic( (A),				\
			vec3f*: vec3f_dot,	\
			vec3d*: vec3d_dot	\
			) ( (A), (B) )

extern vec3f* vec3f_cross( const vec3f* const a, const vec3f* const b, vec3f *out );
extern vec3d* vec3d_cross( const vec3d* const a, const vec3d* const b, vec3d *out );
#define vec3_cross( A, B, C )	\
	_Generic( (A),				\
			vec3f*: vec3f_cross,\
			vec3d*: vec3d_cross	\
			) ( (A), (B), (C) )

extern vec3f* vec3f_normalize( const vec3f* const a, vec3f *out );
extern vec3d* vec3d_normalize( const vec3d* const a, vec3d *out );
#define vec3_normalize( A, B )		\
	_Generic( (A),					\
			vec3f*: vec3f_normalize,\
			vec3d*: vec3d_normalize	\
			) ( (A), (B) )

extern bool vec3f_fcmp( const vec3f* const a, const vec3f* const b );
extern bool vec3d_fcmp( const vec3d* const a, const vec3d* const b );
#define vec3_fcmp( A, B )		\
	_Generic( (A),				\
			vec3f*: vec3f_fcmp,	\
			vec3d*: vec3d_fcmp	\
			) ( (A), (B) )

// like the unary -
/*extern vec3d* vec3d_negate( const vec3d* const a, vec3d *out );
extern vec3f* vec3f_negate( const vec3f* const a, vec3f *out );

extern vec3f* vec3f_lerp( const vec3f* const a, const vec3f* const b, float t, vec3f *out );
extern vec3d* vec3d_lerp( const vec3d* const a, const vec3d* const b, double t, vec3d *out );

extern double vec3d_distance( const vec3d* const a, const vec3d* const b );
extern float vec3f_distance( const vec3f* const a, const vec3f* const b );

extern double vec3d_distance_sq( const vec3d* const a, const vec3d* const b );
extern float vec3f_distance_sq( const vec3f* const a, const vec3f* const b, vec3f *out );

// Calculates angle in radians
extern double vec3d_angle( const vec3d* const a, const vec3d* const b );
extern float vec3f_angle( const vec3f* const a, const vec3f* const b );

// Projects point onto plane using normal vector of the plane
extern vec3d* vec3d_project_plane( const vec3d* const point, const vec3d* const normal, vec3d *out );
extern vec3f* vec3f_project_plane( const vec3f* const point, const vec3f* const normal, vec3f *out );

// Calculates reflection vector of a using normal vector of plane
extern vec3d* vec3d_reflect( const vec3d* const a, const vec3d* const normal, vec3d *out );
extern vec3f* vec3f_reflect( const vec3f* const a, const vec3f* const normal, vec3f *out );

// Calculates middle point between two points
extern vec3d* vec3d_middle( const vec3d* const a, const vec3d* const b, vec3d *out );
extern vec3f* vec3f_middle( const vec3f* const a, const vec3f* const b, vec3f *out );

// Calculates per-component min-max values. Can be used to calculate bounds of cloud of points
extern void vec3d_min_max( const vec3d* const a, vec3d* vMin, vec3d* vMax, vec3d *out );
extern void vec3f_min_max( const vec3f* const a, vec3f* vMin, vec3f* vMax, vec3f *out );

extern bool vec3d_same_direction( const vec3d* const a, const vec3d* const b );
extern bool vec3f_same_direction( const vec3f* const a, const vec3f* const b );

// Just a multiplication
extern vec3f* vec3f_scale( const vec3f* const a, const double s, vec3f *out );
extern vec3d* vec3d_scale( const vec3d* const a, const double s, vec3d *out );

// Calculates new position of point a using matrix mat.
// multiplies 4-D vector (a->x, a->y, a->z, 1.0) using matrix mat.
vec3d* vec3d_transform( const vec3d* a, const de_mat4_t* mat);

// Calculates new direction of vector @a using matrix @mat (multiplies vector using 3*3-matrix basis)
vec3d* vec3d_transform_normal( const vec3d* a, const de_mat4_t* mat);
*/

extern void vec3d_print( const vec3d* const v );
extern void vec3f_print( const vec3f* const v );
