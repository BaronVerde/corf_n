
#include "common.h"
#include <tgmath.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>	// epsilon

inline double lerpd( const double v0, const double v1, const double t ) {
	return( ( 1 - t ) * v0 + t * v1 );
}

inline float lerpf( const float v0, const float v1, const float t ) {
	return( ( 1 - t ) * v0 + t * v1 );
}

inline double cubic_interpolated(
		const double n0, const double n1, const double n2, const double n3, const double a ) {
	const double p = ( n3 - n2 ) - ( n0 - n1 );
	const double q = ( n0 - n1 ) - p;
	const double r = n2 - n0;
	return p * a * a * a + q * a * a + r * a + n1;
}

inline float cubic_interpolatef(
		const float n0, const float n1, const float n2, const float n3, const float a ) {
	const float p = ( n3 - n2 ) - ( n0 - n1 );
	const float q = ( n0 - n1 ) - p;
	const float r = n2 - n0;
	return p * a * a * a + q * a * a + r * a + n1;
}

inline double clampd( const double number, const double minimum, const double maximum ) {
	return ( number < minimum ) ? minimum : ( number > maximum ) ? maximum : number;
}

inline float clampf( const float number, const float minimum, const float maximum ) {
	return ( number < minimum ) ? minimum : ( number > maximum ) ? maximum : number;
}

inline bool fcmp_rd( const double a, const double b, const double maxDiff, const double maxRelDiff ) {
	const double diff = fabs( a - b );
	if( diff <= maxDiff )
		return true;
	const double aa = fabs(a);
	const double ab = fabs(b);
	const double largest = ( ab > aa ) ? ab : aa;
	if( diff <= largest * maxRelDiff )
		return true;
	return false;
}

inline bool fcmp_rf( const float a, const float b, const float maxDiff, const float maxRelDiff ) {
	const float diff = fabs( a - b );
	if( diff <= maxDiff )
		return true;
	const float aa = fabs(a);
	const float ab = fabs(b);
	const float largest = ( ab > aa ) ? ab : aa;
	if( diff <= largest * maxRelDiff )
		return true;
	return false;
}

inline bool fcmpd( const double x, const double y ) {
	return fabs( x - y ) <= DBL_EPSILON * fmax( 1.0, fmax( fabs( x ), fabs( y ) ) );
}

inline bool fcmpf( const float x, const float y ) {
	return fabs( x - y ) <= FLT_EPSILON * fmax( 1.0, fmax( fabs( x ), fabs( y ) ) );
}

inline double radiansd( const double degrees ) {
	return degrees * 0.01745329251994329576923690768489;
}

inline float radiansf( const float degrees ) {
	return degrees * 0.017453292519f;
}

inline double degreesd( const double radians ) {
	return radians * 57.295779513082320876798154814105;
}

inline float degreesf( const float radians ) {
	return radians * 57.29577951308f;
}

inline void double_to_two_floats( const double d, float* high, float* low ) {
	*high = (float)d;
	*low = (float)( d - (double)(*high) );
}

inline bool is_pow2i( const int val ) {
	if( val < 1 )
		return false;
	return ( val & ( val - 1 ) ) == 0 ? true : false;
}

inline bool is_pow2u( const unsigned int val ) {
	if( val < 1 )
		return false;
	return ( val & ( val - 1 ) ) == 0 ? true : false;
}
