
#pragma once

#include "glad/glad.h"
#include <stdbool.h>
#include "omath/vec2.h"
#include "omath/vec3.h"
#include "omath/vec4.h"
#include "omath/mat4.h"

/* Shader uniform locations for standard uniforms to avoid many lookups.
 * If a shader uses one or more of these uniforms it must declare them
 * under these uniform locations in order to be directly accessible
 * via the below static functions.
 * SUL_STANDARD_BASE ist just the base for enumerating. Don't change ;-) */
#define SUL_CAMERA_POSITION_HIGH 5	// Also camera position
#define SUL_CAMERA_POSITION_LOW 6
#define SUL_MODEL_MATRIX 7
#define SUL_VIEW_MATRIX 8
#define SUL_MODEL_VIEW_MATRIX 9
#define SUL_MODEL_VIEW_PROJECTION_MATRIX 10
#define SUL_PROJECTION_MATRIX 11
#define SUL_SUN_DIRECTION 12
#define SUL_SUN_COLOR 13
#define SUL_DIFFUSE_SPECULAR_AMBIENT_SHININESS 14
#define SUL_VIEW_PROJECTION_MATRIX 15
#define SUL_TERRAIN_HEIGHT_HIGH 16
#define SUL_TERRAIN_HEIGHT_LOW 17
// model view projection matrix relative to eye, view matrix stripped of translation
#define SUL_MODEL_VIEW_PROJECTION_MATRIX_RTE 18
#define SUL_NORMAL_MATRIX 19

bool sp_create(
		const char* vertex_shader_file, const char* fragment_shader_file, GLuint* out_program
);

extern void sp_delete( GLuint program );

extern void sp_set_camera_position( const vec3f *const pos );

extern void sp_set_model_matrix( const mat4f *const m );

extern void sp_set_view_matrix( const mat4f *const m );

extern void sp_set_projection_matrix( const mat4f *const m );

extern void sp_set_view_projection_matrix( const mat4f *const m );

extern void sp_set_model_view_matrix( const mat4f *const m );

extern void sp_set_normal_matrix( const mat3f *const m );

extern void sp_set_model_view_projection_matrix( const mat4f *const m );

extern void sp_set_uniform_int( const GLuint program, const char *name, const GLint value );

extern void sp_set_uniform_uint( const GLuint program, const char *name, const GLuint value );

extern void sp_set_uniform_float( const GLuint program, const char *name, const GLfloat value );

extern void sp_set_uniform_double( const GLuint program, const char *name, const GLdouble value );

extern void sp_set_uniform_vec2f( const GLuint program, const char *name, const vec2f *const value );

extern void sp_set_uniform_vec3f( const GLuint program, const char *name, const vec3f *const value );

extern void sp_set_uniform_vec4f( const GLuint program, const char *name, const vec4f *const value );

extern void sp_set_uniform_mat4f( const GLuint program, const char *name, const mat4f *const m );
