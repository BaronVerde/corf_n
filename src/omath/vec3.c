
#include "vec3.h"
#include <stdio.h>
#include <tgmath.h>

inline vec3f* vec3f_add( const vec3f* const a, const vec3f* const b, vec3f *out ) {
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
	return out;
}

inline vec3d* vec3d_add( const vec3d* const a, const vec3d* const b, vec3d *out ) {
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
	return out;
}

inline vec3f* vec3f_sub( const vec3f* const a, const vec3f* const b, vec3f *out ) {
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
	return out;
}

inline vec3d* vec3d_sub( const vec3d* const a, const vec3d* const b, vec3d *out ) {
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
	return out;
}

inline vec3f* vec3f_mul( const vec3f* const a, const vec3f* const b, vec3f *out ) {
	out->x = a->x * b->x;
	out->y = a->y * b->y;
	out->z = a->z * b->z;
	return out;
}

inline vec3d* vec3d_mul( const vec3d* const a, const vec3d* const b, vec3d *out ) {
	out->x = a->x * b->x;
	out->y = a->y * b->y;
	out->z = a->z * b->z;
	return out;
}

inline vec3f* vec3f_mul_s( const vec3f* const a, const float scalar, vec3f *out ) {
	out->x = a->x * scalar;
	out->y = a->y * scalar;
	out->z = a->z * scalar;
	return out;
}

inline vec3d* vec3d_mul_s( const vec3d* const a, const double scalar, vec3d *out ) {
	out->x = a->x * scalar;
	out->y = a->y * scalar;
	out->z = a->z * scalar;
	return out;
}

inline vec3f* vec3f_div( const vec3f* const a, const vec3f* const b, vec3f *out ) {
	out->x = a->x / b->x;
	out->y = a->y / b->y;
	out->z = a->z / b->z;
	return out;
}

inline vec3d* vec3d_div( const vec3d* const a, const vec3d* const b, vec3d *out ) {
	out->x = a->x / b->x;
	out->y = a->y / b->y;
	out->z = a->z / b->z;
	return out;
}

inline vec3f* vec3f_div_s( const vec3f* const a, const float scalar, vec3f *out ) {
	out->x = a->x / scalar;
	out->y = a->y / scalar;
	out->z = a->z / scalar;
	return out;
}

inline vec3d* vec3d_div_s( const vec3d* const a, const double scalar, vec3d *out ) {
	out->x = a->x / scalar;
	out->y = a->y / scalar;
	out->z = a->z / scalar;
	return out;
}

inline float vec3f_magnitude( const vec3f* const a ) {
	return sqrt( a->x * a->x + a->y * a->y + a->z * a->z );
}

inline double vec3d_magnitude( const vec3d* const a ) {
	return sqrt( a->x * a->x + a->y * a->y + a->z * a->z );
}

inline float vec3f_magnitude_sq( const vec3f* const a ) {
	return a->x * a->x + a->y * a->y + a->z * a->z;
}

inline double vec3d_magnitude_sq( const vec3d* const a ) {
	return a->x * a->x + a->y * a->y + a->z * a->z;
}

inline float vec3f_dot( const vec3f* const a, const vec3f* const b ) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

inline double vec3d_dot( const vec3d* const a, const vec3d* const b ) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

inline vec3f* vec3f_cross( const vec3f* const a, const vec3f* const b, vec3f *out ) {
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
	return out;
}

inline vec3d* vec3d_cross( const vec3d* const a, const vec3d* const b, vec3d *out ) {
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
	return out;
}

inline vec3f* vec3f_normalize( const vec3f* const a, vec3f *out ) {
	const float len = vec3f_magnitude( a );
#if ENABLE_SAFETY_CHECKS
	assert( !fcmpf( len, 0.0f ) );
#endif
	const float k = 1.0f / len;
	out->x = a->x * k;
	out->y = a->y * k;
	out->z = a->z * k;
	return out;
}

inline vec3d* vec3d_normalize( const vec3d* const a, vec3d *out ) {
	const double len = vec3d_magnitude( a );
#if ENABLE_SAFETY_CHECKS
	assert( !fcmpd( len, 0.0 ) );
#endif
	const double k = 1.0 / len;
	out->x = a->x * k;
	out->y = a->y * k;
	out->z = a->z * k;
	return out;
}

inline bool vec3f_fcmp( const vec3f* const a, const vec3f* const b ) {
	return fcmpf( a->x, b->x ) && fcmpf( a->y, b->y ) && fcmpf( a->z, b->z );
}

inline bool vec3d_fcmp( const vec3d* const a, const vec3d* const b ) {
	return fcmpd( a->x, b->x ) && fcmpd( a->y, b->y ) && fcmpd( a->z, b->z );
}

/*
inline vec3d* vec3d_lerp( const vec3d* const a, const vec3d* const b, double t ) {
	out->x = lerp( a->x, b->x, t );
	out->y = lerp( a->y, b->y, t );
	out->z = lerp( a->z, b->z, t );
	return out;
}

inline double vec3d_distance( const vec3d* const a, const vec3d* const b ) {
	vec3d d;
	return vec3d_magnitude( vec3d_sub( &d, a, b ) );
}

inline double vec3d_distance_sq( const vec3d* const a, const vec3d* const b ) {
	vec3d d;
	return vec3d_magnitude_sq( vec3d_sub( &d, a, b ) );
}

inline double vec3d_angle( const vec3d* const a, const vec3d* const b ) {
	double len = vec3d_magnitude( a ) * vec3d_magnitude( b );
#if ENABLE_SAFETY_CHECKS
	assert( !fcmp( len, 0.0 ) );
#endif
	return acos( vec3d_dot( a, b ) / len );
}

inline vec3d* vec3d_project_plane( const vec3d* const point, const vec3d* const normal ) {
	vec3d n = { 0, 0, 0 };
	// degenerated normal vector ?
	if( vec3d_magnitude_sq( normal ) < 0.00001 )
		return out;
	vec3d_normalize( &n, normal );
	vec3d_scale( &n, &n, -vec3d_dot( &n, point ) );
	return vec3d_add( out, point, &n );
}

inline vec3d* vec3d_reflect( const vec3d* const a, const vec3d* const normal ) {
	vec3d v;
	vec3d_scale( &v, normal, 2.0 * vec3d_dot( a, normal ) );
	return vec3d_sub( out, a, &v );
}

inline vec3d* vec3d_middle( const vec3d* const a, const vec3d* const b ) {
	vec3d v;
	vec3d_add( &v, a, b );
	vec3d_scale( out, &v, 0.5f );
	return out;
}

inline void vec3d_minmax( const vec3d* const a, vec3d* vMin, vec3d* vMax ) {
	if( a->x < vMin->x )
		vMin->x = a->x;
	if( a->y < vMin->y )
		vMin->y = a->y;
	if( a->z < vMin->z )
		vMin->z = a->z;
	if( a->x > vMax->x )
		vMax->x = a->x;
	if( a->y > vMax->y )
		vMax->y = a->y;
	if( a->z > vMax->z )
		vMax->z = a->z;
}

inline bool vec3d_samedirection( const vec3d* const a, const vec3d* const b ) {
	return vec3d_dot( a, b ) > 0.0;
}

bool de_vec3_try_normalize(de_vec3_t* out, const de_vec3_t* a) {
	float k;
	float len = de_vec3_len(a);
	if (len > FLT_EPSILON) {
		k = 1.0f / len;
		out->x = a->x * k;
		out->y = a->y * k;
		out->z = a->z * k;
		return true;
	}
	return false;
}

de_vec3_t* de_vec3_normalize_ex(de_vec3_t* out, const de_vec3_t* a, float* length) {
	float k;
	*length = de_vec3_len(a);
#if DE_MATH_CHECKS
	DE_CHECK_DENOMINATOR(*length);
#endif
	k = 1.0f / *length;
	out->x = a->x * k;
	out->y = a->y * k;
	out->z = a->z * k;
	return out;
}

de_vec3_t* de_vec3_transform(de_vec3_t* out, const de_vec3_t* a, const de_mat4_t* mat)
{
	de_vec3_t v;
	v.x = a->x * mat->f[0] + a->y * mat->f[4] + a->z * mat->f[8] + mat->f[12];
	v.y = a->x * mat->f[1] + a->y * mat->f[5] + a->z * mat->f[9] + mat->f[13];
	v.z = a->x * mat->f[2] + a->y * mat->f[6] + a->z * mat->f[10] + mat->f[14];
	out->x = v.x;
	out->y = v.y;
	out->z = v.z;
	return out;
}

de_vec3_t* de_vec3_transform_normal(de_vec3_t* out, const de_vec3_t* a, const de_mat4_t* mat)
{
	de_vec3_t v;
	v.x = a->x * mat->f[0] + a->y * mat->f[4] + a->z * mat->f[8];
	v.y = a->x * mat->f[1] + a->y * mat->f[5] + a->z * mat->f[9];
	v.z = a->x * mat->f[2] + a->y * mat->f[6] + a->z * mat->f[10];
	out->x = v.x;
	out->y = v.y;
	out->z = v.z;
	return out;
}

inline vec3d* vec3d_negate( const vec3d* const a ) {
	out->x = -a->x;
	out->y = -a->y;
	out->z = -a->z;
	return out;
}

inline vec3d* vec3d_scale( const vec3d* const a, const double s ) {
	out->x = a->x * s;
	out->y = a->y * s;
	out->z = a->z * s;
	return out;
}
*/

inline void vec3f_print( const vec3f* const v ) {
	printf( "( %f, %f, %f )\n ", v->x, v->y, v->z );
}

inline void vec3d_print( const vec3d* const v ) {
	printf( "( %lf, %lf, %lf )\n ", v->x, v->y, v->z );
}
