
#pragma once

#define _XOPEN_SOURCE	// for drand48 to be available
#define _GNU_SOURCE		// sincos() in mat4d.c
#include <stdbool.h>

//#define ENABLE_SAFETY_CHECKS 1

// Global consts. I don't like the math.h wording. Mind definitions in shaders !
#define SQRT2 1.4142135623730950488
#define SQRT3 1.7320508075688772935
#define PI 3.1415926535897932385
#define ONE_OVER_PI ( 1.0 / PI )
#define ONE_OVER_TWO_PI ( 1.0 / ( 2.0 * PI ) )
#define PI_OVER_TWO ( PI * 0.5 )
#define PI_OVER_THREE ( PI / 3.0 )
#define PI_OVER_FOUR ( PI / 4.0 )
#define PI_OVER_SIX ( PI / 6.0 )
#define THREE_PI_OVER_TWO ( 1.5 * PI )
#define TWO_PI ( 2.0 * PI )

// returns v0 when t == 0 and v1 when t == 1
extern double lerpd( const double v0, const double v1, const double t );
extern float lerpf( const float v0, const float v1, const float t );
#define lerp( A, B, C )		\
	_Generic( (A)+(B)+(C),	\
			float: lerpf,	\
			double: lerpd	\
			) ( (A), (B), (C) )

// Interpolates between n1 and n2 with given pre-n1 value n0 and post-n1 value n3
// If a is 0, function returns n1, if a is 1.0, function returns n2
// After libnoise ! Default value for a should be 0.5
extern double cubic_interpolated(
		const double n0, const double n1, const double n2, const double n3, const double a );
extern float cubic_interpolatef(
		const float n0, const float n1, const float n2, const float n3, const float a );
#define cubic_interpolate( A, B, C, D )	\
	_Generic( (A)+(B)+(C)+(D),			\
			float: cubic_interpolatef,	\
			double: cubic_interpolated	\
			) ( (A), (B), (C), (D) )

extern double clampd( const double number, const double minimum, const double maximum );
extern float clampf( const float number, const float minimum, const float maximum );
#define clamp( A, B, C )		\
	_Generic( (A)+(B)+(C),		\
			float: clampf,		\
			double: clampd		\
			) ( (A), (B), (C) )

// Compares two float numbers combining absolute and relative tolerance.
// http://www.realtimecollisiondetection.net/pubs/doubleolerances/
extern bool fcmp_rd( const double a, const double b, const double maxDiff, const double maxRelDiff );
extern bool fcmp_rf( const float a, const float b, const float maxDiff, const float maxRelDiff );
#define fcmp_r( A, B, C, D )	\
	_Generic( (A)+(B)+(C)+(D),	\
			float: fcmp_rf,		\
			double: fcmp_rd,	\
			) ( (A), (B), (C), (D) )

// Floating point absolute comparison
extern bool fcmpd( const double x, const double y );
extern bool fcmpf( const float x, const float y );
#define fcmp( A, B )		\
	_Generic( (A)+(B),		\
			float: fcmpf,	\
			double: fcmpd	\
			) ( (A), (B) )

// radians
extern double radiansd( const double degrees );
extern float radiansf( const float degrees );
#define radians( A )			\
	_Generic( (A),				\
			float: radiansf,	\
			double: radiansd	\
			) ( (A) )

// degrees
extern double degreesd( const double radians );
extern float degreesf( const float radians );
#define degrees( A )			\
	_Generic( (A),				\
			float: degreesf,	\
			double: degreesd	\
			) ( (A) )

extern bool is_pow2i( const int val );
extern bool is_pow2u( const unsigned int val );

//@todo extern int signum( const double val );
