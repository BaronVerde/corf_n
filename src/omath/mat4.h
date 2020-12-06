
#pragma once

#include "mat3.h"
#include "vec3.h"
//#include "quatf.h"
#include <stddef.h>

typedef struct {
	float data[16];
} mat4f;

typedef struct {
	double data[16];
} mat4d;

extern mat4f* mat4f_from( const mat4f *const other, mat4f *out );

// Returns matrix component at specified row and column. No checking
extern float mat4f_at( const mat4f *const m, size_t row, size_t column );

extern mat4f *mat4f_transpose( const mat4f *const m, mat4f *out );

extern mat4f *mat4f_mul( const mat4f* const a, const mat4f* const b, mat4f *out );

// Builds scaling matrix
extern mat4f *mat4f_scale( const vec3f* const v, mat4f *out );

// Builds perspective matrix
extern mat4f *mat4f_perspective(
		const float fov_radians, const float aspect, const float zNear, const float zFar, mat4f *out );

// Builds matrix for orthographics projection
extern mat4f *mat4f_ortho(
		const float left, const float right, const float bottom, const float top,
		const float zNear, const float zFar, mat4f *out );

// Builds translation matrix
extern mat4f *mat4f_translate( const vec3f* const v, mat4f *out );

// Builds "look-at" view matrix
extern mat4f *mat4f_lookat(
		const vec3f* const eye, const vec3f* const at, const vec3f* const up, mat4f *out );

// Inverses matrix, so A * A_inv = Identity
extern mat4f *mat4f_inverse( const mat4f *const a, mat4f *out );

// Rotation matrix from axis vector (unnormalized) and angle
//extern void mat4_rotate( const float angle, const vec3f* axis );
// Could also be an axis rotation angle * vector( yaw, pitch, roll ) in model coords
// Axis angles in radians, pls !
extern mat4f *mat4f_rotate( const float yaw_x, const float pitch_y, const float roll_z, mat4f *out );

// Builds rotate matrix around (1, 0, 0) axis
extern mat4f *mat4f_rotate_x( const float angle_radians, mat4f *out );

// Builds rotate matrix around (0, 1, 0) axis
extern mat4f *mat4f_rotate_y( const float angle_radians, mat4f *out );

// Builds rotate matrix around (0, 0, 1) axis
extern mat4f *mat4f_rotate_z( const float angle_radians, mat4f *out );

// Extracts "up" vector from basis of matrix m
extern vec3f* mat4f_up( const mat4f* const m, vec3f* up );

// Extracts "side" vector from basis of matrix m @todo right side ?
extern vec3f* mat4f_side( const mat4f* const m, vec3f* side );

// Extracts "front" vector from basis of matrix m
extern vec3f* mat4f_front( const mat4f* const m, vec3f *front );

extern vec3f *mat4f_get_translation( const mat4f *const m, vec3f* translation );

extern void mat4f_print( const mat4f *const m );

// Extracts basis as 3x3 matrix.
extern mat3f *mat4f_get_basis( const mat4f *const in, mat3f *out );

//extern quatd* mat4_to_quat( const mat4* const m, quatd* quat );

// Build rotate matrix from quaternion
//extern void mat4_rotate( const struct quatd* q );
