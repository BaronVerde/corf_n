
#include "mat4.h"
#include <string.h>
#include <stdio.h>
#include <tgmath.h>

inline mat4f* mat4f_from( const mat4f* const other, mat4f *out ) {
	memcpy( &out->data[0], &other->data[0], sizeof( out->data ) );
	return out;
}

inline float mat4f_at( const mat4f* const m, size_t row, size_t column ) {
	return m->data[row + 4 * column];
}

inline mat4f *mat4f_transpose( const mat4f* const m, mat4f *out ) {
	for( int j = 0; j < 4; ++j )
		for( int i = 0; i < 4; ++i )
			out->data[( j * 4 ) + i] = m->data[(i * 4) + j];
	return out;
}

inline mat4f *mat4f_mul( const mat4f* const a, const mat4f* const b, mat4f *out ) {
	out->data[0] = a->data[0] * b->data[0] + a->data[4] * b->data[1] + a->data[8] * b->data[2] + a->data[12] * b->data[3];
	out->data[1] = a->data[1] * b->data[0] + a->data[5] * b->data[1] + a->data[9] * b->data[2] + a->data[13] * b->data[3];
	out->data[2] = a->data[2] * b->data[0] + a->data[6] * b->data[1] + a->data[10] * b->data[2] + a->data[14] * b->data[3];
	out->data[3] = a->data[3] * b->data[0] + a->data[7] * b->data[1] + a->data[11] * b->data[2] + a->data[15] * b->data[3];
	out->data[4] = a->data[0] * b->data[4] + a->data[4] * b->data[5] + a->data[8] * b->data[6] + a->data[12] * b->data[7];
	out->data[5] = a->data[1] * b->data[4] + a->data[5] * b->data[5] + a->data[9] * b->data[6] + a->data[13] * b->data[7];
	out->data[6] = a->data[2] * b->data[4] + a->data[6] * b->data[5] + a->data[10] * b->data[6] + a->data[14] * b->data[7];
	out->data[7] = a->data[3] * b->data[4] + a->data[7] * b->data[5] + a->data[11] * b->data[6] + a->data[15] * b->data[7];
	out->data[8] = a->data[0] * b->data[8] + a->data[4] * b->data[9] + a->data[8] * b->data[10] + a->data[12] * b->data[11];
	out->data[9] = a->data[1] * b->data[8] + a->data[5] * b->data[9] + a->data[9] * b->data[10] + a->data[13] * b->data[11];
	out->data[10] = a->data[2] * b->data[8] + a->data[6] * b->data[9] + a->data[10] * b->data[10] + a->data[14] * b->data[11];
	out->data[11] = a->data[3] * b->data[8] + a->data[7] * b->data[9] + a->data[11] * b->data[10] + a->data[15] * b->data[11];
	out->data[12] = a->data[0] * b->data[12] + a->data[4] * b->data[13] + a->data[8] * b->data[14] + a->data[12] * b->data[15];
	out->data[13] = a->data[1] * b->data[12] + a->data[5] * b->data[13] + a->data[9] * b->data[14] + a->data[13] * b->data[15];
	out->data[14] = a->data[2] * b->data[12] + a->data[6] * b->data[13] + a->data[10] * b->data[14] + a->data[14] * b->data[15];
	out->data[15] = a->data[3] * b->data[12] + a->data[7] * b->data[13] + a->data[11] * b->data[14] + a->data[15] * b->data[15];
	return out;
}

inline mat4f *mat4f_scale( const vec3f* const v, mat4f *out ) {
	out->data[0] = v->x;
	out->data[5] = v->y;
	out->data[10] = v->z;
	out->data[15] = 1.0f;
	return out;
}

inline mat4f *mat4f_perspective(
		const float fov_radians, const float aspect, const float zNear, const float zFar, mat4f *out ) {
	const float y_scale = 1.0f / tan( fov_radians * 0.5f );
	const float x_scale = y_scale / aspect;
	out->data[0] = x_scale;
	out->data[1] = 0;
	out->data[2] = 0;
	out->data[3] = 0;
	out->data[4] = 0;
	out->data[5] = y_scale;
	out->data[6] = 0;
	out->data[7] = 0;
	out->data[8] = 0;
	out->data[9] = 0;
	out->data[10] = zFar / (zNear - zFar);
	out->data[11] = -1;
	out->data[12] = 0;
	out->data[13] = 0;
	out->data[14] = zNear * zFar / (zNear - zFar);
	out->data[15] = 0;
	return out;
}

inline mat4f *mat4f_ortho( const float left, const float right,
		const float bottom, const float top, const float zNear, const float zFar, mat4f *out ) {
	out->data[0] = 2.0f / (right - left);
	out->data[1] = 0.0f;
	out->data[2] = 0.0f;
	out->data[3] = 0.0f;
	out->data[4] = 0.0f;
	out->data[5] = 2.0f / (top - bottom);
	out->data[6] = 0.0f;
	out->data[7] = 0.0f;
	out->data[8] = 0.0f;
	out->data[9] = 0.0f;
	out->data[10] = 1.0f / (zFar - zNear);
	out->data[11] = 0.0f;
	out->data[12] = (left + right) / (left - right);
	out->data[13] = (top + bottom) / (bottom - top);
	out->data[14] = zNear / (zNear - zFar);
	out->data[15] = 1.0f;
	return out;
}

inline mat4f* mat4f_translate( const vec3f* const v, mat4f *out ) {
	out->data[0] = 1.0f;
	out->data[1] = 0.0f;
	out->data[2] = 0.0f;
	out->data[3] = 0.0f;
	out->data[4] = 0.0f;
	out->data[5] = 1.0f;
	out->data[6] = 0.0f;
	out->data[7] = 0.0f;
	out->data[8] = 0.0f;
	out->data[9] = 0.0f;
	out->data[10] = 1.0f;
	out->data[11] = 0.0f;
	out->data[12] = v->x;
	out->data[13] = v->y;
	out->data[14] = v->z;
	out->data[15] = 1.0f;
	return out;
}

inline mat4f *mat4f_lookat(
		const vec3f* const eye, const vec3f* const at, const vec3f* const up, mat4f *out ) {
	vec3f zaxis, xaxis, yaxis;
	vec3f_normalize( vec3f_sub( eye, at, &zaxis ), &zaxis );
	vec3f_normalize( vec3f_cross( up, &zaxis, &xaxis ), &xaxis );
	vec3f_normalize( vec3f_cross( &zaxis, &xaxis, &yaxis ), &yaxis );
	out->data[0] = xaxis.x;
	out->data[1] = yaxis.x;
	out->data[2] = zaxis.x;
	out->data[3] = 0.0f;
	out->data[4] = xaxis.y;
	out->data[5] = yaxis.y;
	out->data[6] = zaxis.y;
	out->data[7] = 0.0f;
	out->data[8] = xaxis.z;
	out->data[9] = yaxis.z;
	out->data[10] = zaxis.z;
	out->data[11] = 0.0f;
	out->data[12] = -vec3f_dot( eye, &xaxis );
	out->data[13] = -vec3f_dot( eye, &yaxis );
	out->data[14] = -vec3f_dot( eye, &zaxis );
	out->data[15] = 1.0f;
	return out;
}

inline mat4f *mat4f_inverse( const mat4f *const a, mat4f *out ) {
	mat4f temp = { .data = {
			a->data[5] * a->data[10] * a->data[15] - a->data[5] * a->data[14] * a->data[11] - a->data[6] * a->data[9] * a->data[15] + a->data[6] * a->data[13] * a->data[11] + a->data[7] * a->data[9] * a->data[14] - a->data[7] * a->data[13] * a->data[10],
			-a->data[1] * a->data[10] * a->data[15] + a->data[1] * a->data[14] * a->data[11] + a->data[2] * a->data[9] * a->data[15] - a->data[2] * a->data[13] * a->data[11] - a->data[3] * a->data[9] * a->data[14] + a->data[3] * a->data[13] * a->data[10],
			a->data[1] * a->data[6] * a->data[15] - a->data[1] * a->data[14] * a->data[7] - a->data[2] * a->data[5] * a->data[15] + a->data[2] * a->data[13] * a->data[7] + a->data[3] * a->data[5] * a->data[14] - a->data[3] * a->data[13] * a->data[6],
			-a->data[1] * a->data[6] * a->data[11] + a->data[1] * a->data[10] * a->data[7] + a->data[2] * a->data[5] * a->data[11] - a->data[2] * a->data[9] * a->data[7] - a->data[3] * a->data[5] * a->data[10] + a->data[3] * a->data[9] * a->data[6],
			-a->data[4] * a->data[10] * a->data[15] + a->data[4] * a->data[14] * a->data[11] + a->data[6] * a->data[8] * a->data[15] - a->data[6] * a->data[12] * a->data[11] - a->data[7] * a->data[8] * a->data[14] + a->data[7] * a->data[12] * a->data[10],
			a->data[0] * a->data[10] * a->data[15] - a->data[0] * a->data[14] * a->data[11] - a->data[2] * a->data[8] * a->data[15] + a->data[2] * a->data[12] * a->data[11] + a->data[3] * a->data[8] * a->data[14] - a->data[3] * a->data[12] * a->data[10],
			-a->data[0] * a->data[6] * a->data[15] + a->data[0] * a->data[14] * a->data[7] + a->data[2] * a->data[4] * a->data[15] - a->data[2] * a->data[12] * a->data[7] - a->data[3] * a->data[4] * a->data[14] + a->data[3] * a->data[12] * a->data[6],
			a->data[0] * a->data[6] * a->data[11] - a->data[0] * a->data[10] * a->data[7] - a->data[2] * a->data[4] * a->data[11] + a->data[2] * a->data[8] * a->data[7] + a->data[3] * a->data[4] * a->data[10] - a->data[3] * a->data[8] * a->data[6],
			a->data[4] * a->data[9] * a->data[15] - a->data[4] * a->data[13] * a->data[11] - a->data[5] * a->data[8] * a->data[15] + a->data[5] * a->data[12] * a->data[11] + a->data[7] * a->data[8] * a->data[13] - a->data[7] * a->data[12] * a->data[9],
			-a->data[0] * a->data[9] * a->data[15] + a->data[0] * a->data[13] * a->data[11] + a->data[1] * a->data[8] * a->data[15] - a->data[1] * a->data[12] * a->data[11] - a->data[3] * a->data[8] * a->data[13] + a->data[3] * a->data[12] * a->data[9],
			a->data[0] * a->data[5] * a->data[15] - a->data[0] * a->data[13] * a->data[7] - a->data[1] * a->data[4] * a->data[15] + a->data[1] * a->data[12] * a->data[7] + a->data[3] * a->data[4] * a->data[13] - a->data[3] * a->data[12] * a->data[5],
			-a->data[0] * a->data[5] * a->data[11] + a->data[0] * a->data[9] * a->data[7] + a->data[1] * a->data[4] * a->data[11] - a->data[1] * a->data[8] * a->data[7] - a->data[3] * a->data[4] * a->data[9] + a->data[3] * a->data[8] * a->data[5],
			-a->data[4] * a->data[9] * a->data[14] + a->data[4] * a->data[13] * a->data[10] + a->data[5] * a->data[8] * a->data[14] - a->data[5] * a->data[12] * a->data[10] - a->data[6] * a->data[8] * a->data[13] + a->data[6] * a->data[12] * a->data[9],
			a->data[0] * a->data[9] * a->data[14] - a->data[0] * a->data[13] * a->data[10] - a->data[1] * a->data[8] * a->data[14] + a->data[1] * a->data[12] * a->data[10] + a->data[2] * a->data[8] * a->data[13] - a->data[2] * a->data[12] * a->data[9],
			-a->data[0] * a->data[5] * a->data[14] + a->data[0] * a->data[13] * a->data[6] + a->data[1] * a->data[4] * a->data[14] - a->data[1] * a->data[12] * a->data[6] - a->data[2] * a->data[4] * a->data[13] + a->data[2] * a->data[12] * a->data[5],
			a->data[0] * a->data[5] * a->data[10] - a->data[0] * a->data[9] * a->data[6] - a->data[1] * a->data[4] * a->data[10] + a->data[1] * a->data[8] * a->data[6] + a->data[2] * a->data[4] * a->data[9] - a->data[2] * a->data[8] * a->data[5]
	} };
	float det = a->data[0] * temp.data[0] + a->data[4] * temp.data[1] + a->data[8] * temp.data[2] + a->data[12] * temp.data[3];
	if( fabs(det) > 0.00001f )
		det = 1.0f / det;
	for( int i = 0; i < 16; ++i )
		out->data[i] = temp.data[i] * det;
	return out;
}

inline mat4f *mat4f_rotate( const float yaw_x, const float pitch_y, const float roll_z, mat4f *out ) {
	float cos_yaw, sin_yaw;
	//@todo sincos is GNU C
	sincosf( yaw_x, &sin_yaw, &cos_yaw );
	float cos_pitch, sin_pitch;
	sincosf( pitch_y, &sin_pitch, &cos_pitch );
	float cos_roll, sin_roll;
	sincosf( roll_z, &sin_roll, &cos_roll );
	out->data[0] = cos_roll * cos_yaw + sin_roll * sin_pitch * sin_yaw;
	out->data[1] = sin_roll * cos_pitch;
	out->data[2] = cos_roll * -sin_yaw + sin_roll * sin_pitch * cos_yaw;
	out->data[3] = 0.0f;
	out->data[4] = -sin_roll * cos_yaw + cos_roll * sin_pitch * sin_yaw;
	out->data[5] = cos_roll * cos_pitch;
	out->data[6] = sin_roll * sin_yaw + cos_roll * sin_pitch * cos_yaw;
	out->data[7] = 0.0f;
	out->data[8] = cos_pitch * sin_yaw;
	out->data[9] = -sin_pitch;
	out->data[10] = cos_pitch * cos_yaw;
	out->data[11] = 0.0f;
	out->data[12] = 0.0f; out->data[13] = 0.0f; out->data[14] = 0.0f; out->data[15] = 1.0f;
	return out;
}

inline mat4f *mat4f_rotate_x( const float angle_radians, mat4f *out ) {
	float c, s;
	sincosf( angle_radians, &s, &c );
	out->data[0] = 1.0f;
	out->data[1] = 0.0f;
	out->data[2] = 0.0f;
	out->data[3] = 0.0f;
	out->data[4] = 0.0f;
	out->data[5] = c;
	out->data[6] = s;
	out->data[7] = 0.0f;
	out->data[8] = 0.0f;
	out->data[9] = -s;
	out->data[10] = c;
	out->data[11] = 0.0f;
	out->data[12] = 0.0f;
	out->data[13] = 0.0f;
	out->data[14] = 0.0f;
	out->data[15] = 1.0f;
	return out;
}

inline mat4f *mat4f_rotate_y( const float angle_radians, mat4f *out ) {
	float c, s;
	sincosf( angle_radians, &c, &s );
	out->data[0] = c;
	out->data[1] = 0.0f;
	out->data[2] = -s;
	out->data[3] = 0.0f;
	out->data[4] = 0.0f;
	out->data[5] = 1.0f;
	out->data[6] = 0.0f;
	out->data[7] = 0.0f;
	out->data[8] = s;
	out->data[9] = 0.0f;
	out->data[10] = c;
	out->data[11] = 0.0f;
	out->data[12] = 0.0f;
	out->data[13] = 0.0f;
	out->data[14] = 0.0f;
	out->data[15] = 1.0f;
	return out;
}

inline mat4f *mat4_rotate_z( const float angle_radians, mat4f *out ) {
	float c, s;
	sincosf( angle_radians, &c, &s );
	out->data[0] = c;
	out->data[1] = s;
	out->data[2] = 0.0f;
	out->data[3] = 0.0f;
	out->data[4] = -s;
	out->data[5] = c;
	out->data[6] = 0.0f;
	out->data[7] = 0.0f;
	out->data[8] = 0.0f;
	out->data[9] = 0.0f;
	out->data[10] = 1.0f;
	out->data[11] = 0.0f;
	out->data[12] = 0.0f;
	out->data[13] = 0.0f;
	out->data[14] = 0.0f;
	out->data[15] = 1.0f;
	return out;
}

inline vec3f *mat4f_up( const mat4f *const m, vec3f *up ) {
	up->x = m->data[4];
	up->y = m->data[5];
	up->z = m->data[6];
	return up;
}

inline vec3f* mat4f_side( const mat4f* const m, vec3f* side ) {
	side->x = m->data[0];
	side->y = m->data[1];
	side->z = m->data[2];
	return side;
}

inline vec3f* mat4f_front( const mat4f* const m, vec3f *front ) {
	front->x = m->data[8];
	front->y = m->data[9];
	front->z = m->data[10];
	return front;
}

inline vec3f *mat4f_get_translation( const mat4f *const m, vec3f *translation ) {
	translation->x = m->data[12];
	translation->y = m->data[13];
	translation->z = m->data[14];
	return translation;
}

inline void mat4f_print( const mat4f* const m ) {
	printf( "( %.2f, %.2f, %.2f, %.2f,\n%.2f, %.2f, %.2f, %.2f,\n%.2f, %.2f, %.2f, %.2f,\n%.2f, %.2f, %.2f, %.2f )\n",
			m->data[0], m->data[1], m->data[2], m->data[3], m->data[4], m->data[5], m->data[6], m->data[7],
			m->data[8], m->data[9], m->data[10], m->data[11], m->data[12], m->data[13], m->data[14], m->data[15] );
}

inline mat3f *mat4f_get_basis( const mat4f *const in, mat3f *out ) {
	out->data[0] = in->data[0];
	out->data[1] = in->data[1];
	out->data[2] = in->data[2];
	out->data[3] = in->data[4];
	out->data[4] = in->data[5];
	out->data[5] = in->data[6];
	out->data[6] = in->data[8];
	out->data[7] = in->data[9];
	out->data[8] = in->data[10];
	return out;
}

/*
inline void mat4_rotate( mat4* out, const quatd* const q ) {
	float s = 2.0 / de_quat_sqr_len(q);
	float xs = q->x * s;
	float ys = q->y * s;
	float zs = q->z * s;
	float wx = q->w * xs;
	float wy = q->w * ys;
	float wz = q->w * zs;
	float xx = q->x * xs;
	float xy = q->x * ys;
	float xz = q->x * zs;
	float yy = q->y * ys;
	float yz = q->y * zs;
	float zz = q->z * zs;
	out->data[0] = 1.0f - (yy + zz);
	out->data[4] = xy - wz;
	out->data[8] = xz + wy;
	out->data[12] = 0.0f;
	out->data[1] = xy + wz;
	out->data[5] = 1.0f - (xx + zz);
	out->data[9] = yz - wx;
	out->data[13] = 0.0f;
	out->data[2] = xz - wy;
	out->data[6] = yz + wx;
	out->data[10] = 1.0f - (xx + yy);
	out->data[14] = 0.0f;
	out->data[3] = 0.0f;
	out->data[7] = 0.0f;
	out->data[11] = 0.0f;
	out->data[15] = 1.0f;
}
*/
