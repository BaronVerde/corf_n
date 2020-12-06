
#include "shader_program.h"
#include <stdlib.h>
#include <stdio.h>
#include "base/logbook.h"

static bool sp_read_source_file( GLchar** out_source, const char* filename );
static bool sp_compile( const GLuint shader, const GLchar* shader_source );

bool sp_create(
		const char* vertex_shader_file, const char* fragment_shader_file, GLuint* out_program ) {
	char error_string[MAX_LEN_MESSAGES];
	snprintf(
			error_string, MAX_LEN_MESSAGES, "Loading shader '%s', '%s'",
			vertex_shader_file, fragment_shader_file
	);
	logbook_log( LOG_INFO, error_string );
	// loading and compilation
	GLchar *source_code = NULL;
	if( !sp_read_source_file( &source_code, vertex_shader_file ) ) {
		snprintf(
				error_string, MAX_LEN_MESSAGES, "Error reading shader file '%s'", vertex_shader_file
		);
		logbook_log( LOG_ERROR, error_string );
		return false;
	}
	GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
	if( !sp_compile( vertex_shader, source_code ) ) {
		free( source_code );
		glDeleteShader( vertex_shader );
		return false;
	}
	free( source_code );
	source_code = NULL;
	// Source assumed to be null-terminated, see read function below
	if( !sp_read_source_file( &source_code, fragment_shader_file ) ) {
		snprintf(
				error_string, MAX_LEN_MESSAGES, "Error reading shader file '%s'", fragment_shader_file
		);
		logbook_log( LOG_ERROR, error_string );
		glDeleteShader( vertex_shader );
		return false;
	};
	// Source assumed to be null-terminated, see read function below
	GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
	if( !sp_compile( fragment_shader, source_code ) ) {
		free( source_code );
		glDeleteShader( vertex_shader );
		glDeleteShader( fragment_shader );
		return false;
	}
	free( source_code );
	source_code = NULL;
	// attaching to program
	*out_program = glCreateProgram();
	if( !glIsProgram( *out_program ) ) {
		glDeleteShader( vertex_shader );
		glDeleteShader( fragment_shader );
		return false;
	}
	glAttachShader( *out_program, vertex_shader );
	glAttachShader( *out_program, fragment_shader );
	// Linking
	glLinkProgram( *out_program );
	GLint linked;
	glGetProgramiv( *out_program, GL_LINK_STATUS, &linked );
	if( GL_TRUE != linked ) {
		GLint len;
		glGetProgramiv( *out_program, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = malloc( sizeof(GLchar) * (size_t)(len + 1 ) );
		glGetProgramInfoLog( *out_program, len, &len, log );
		snprintf( error_string, MAX_LEN_MESSAGES, "Linker error: '%s'", log );
		logbook_log( LOG_ERROR, error_string );
		free( log );
		glDeleteProgram( *out_program );
		glDeleteShader( vertex_shader );
		glDeleteShader( fragment_shader );
		return false;
	}
	// shaders can be deleted once linked (no seperate programs for now)
	glDeleteShader( vertex_shader );
	glDeleteShader( fragment_shader );
	return true;
}

inline void sp_delete( GLuint program ) {
	if( glIsProgram( program ) )
		glDeleteProgram( program );
}

inline void sp_set_camera_position( const vec3f *const pos ) {
	glUniform3fv( SUL_CAMERA_POSITION_HIGH, 1, (float*)pos );
}

inline void sp_set_view_matrix( const mat4f *const m ) {
	glUniformMatrix4fv( SUL_VIEW_MATRIX, 1, GL_FALSE, m->data );
}

inline void sp_set_normal_matrix( const mat3f *const m ) {
	glUniformMatrix3fv( SUL_NORMAL_MATRIX, 1, GL_FALSE, m->data );
}

inline void sp_set_model_matrix( const mat4f *const m ) {
	glUniformMatrix4fv( SUL_MODEL_MATRIX, 1, GL_FALSE, m->data );
}

inline void sp_set_view_projection_matrix( const mat4f *const m ) {
	glUniformMatrix4fv( SUL_VIEW_PROJECTION_MATRIX, 1, GL_FALSE, m->data );
}

inline void sp_set_uniform_int( const GLuint program, const char *name, const GLint value ) {
	glUniform1i( glGetUniformLocation( program, name ), value );
}

inline void sp_set_uniform_uint( const GLuint program, const char *name, const GLuint value ) {
	glUniform1ui( glGetUniformLocation( program, name ), value );
}

inline void sp_set_uniform_float( const GLuint program, const char *name, const GLfloat value ) {
	glUniform1f( glGetUniformLocation( program, name ), value );
}

inline void sp_set_uniform_double( const GLuint program, const char *name, const GLdouble value ) {
	glUniform1d( glGetUniformLocation( program, name ), value );
}

inline void sp_set_uniform_vec2f( const GLuint program, const char *name, const vec2f *value ) {
	glUniform2fv( glGetUniformLocation( program, name ), 1, (float*)value );
}

inline void sp_set_uniform_vec3f( const GLuint program, const char *name, const vec3f *const value ) {
	glUniform3fv( glGetUniformLocation( program, name ), 1, (float*)value );
}

inline void sp_set_uniform_vec4f( const GLuint program, const char *name, const vec4f *const value ) {
	glUniform4fv( glGetUniformLocation( program, name ), 1, (float*)value );
}

inline void sp_set_uniform_mat4f( const GLuint program, const char *name, const mat4f *const m ) {
	glUniformMatrix4fv( glGetUniformLocation( program, name ), 1, GL_FALSE, m->data );
}

inline void sp_set_projection_matrix( const mat4f *const m ) {
	glUniformMatrix4fv( SUL_PROJECTION_MATRIX, 1, GL_FALSE, m->data );
}

inline void sp_set_model_view_matrix( const mat4f *const m ) {
	glUniformMatrix4fv( SUL_MODEL_VIEW_MATRIX, 1, GL_FALSE, m->data );
}

inline void sp_set_model_view_projection_matrix( const mat4f *const m ) {
	glUniformMatrix4fv( SUL_MODEL_VIEW_PROJECTION_MATRIX, 1, GL_FALSE, m->data );
}

// Pass NULL pointer and don't forget to free returned pointer in calling routine !
static bool sp_read_source_file( GLchar** out_source, const char* filename ) {
	FILE* shader_file = fopen( filename, "r" );
	if( !shader_file )
		return false;
	fseek( shader_file, 0, SEEK_END );
	long int file_length = ftell( shader_file );
	size_t file_size = (size_t)file_length;
	if( file_size > 1 ) {
		fseek( shader_file, 0, SEEK_SET );
		// +1 for trailing \0
		*out_source = malloc( sizeof(GLchar) * ( file_size + 1 ) );
		if( 1 == fread( *out_source, file_size, 1, shader_file ) ) {
			// Just to be sure
			(*out_source)[file_size] = '\0';
			return true;
		}
	}
	fclose( shader_file );
	if( NULL != *out_source )
		free( *out_source );
	return false;
}

static bool sp_compile( const GLuint shader, const GLchar* shader_source ) {
	const char* vsp = shader_source;
	glShaderSource( shader, 1, &vsp, NULL );
	glCompileShader( shader );
	GLint compiled;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
	if( !compiled ) {
		GLsizei len;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = malloc( (size_t)(len + 1) * sizeof(GLchar) );
		glGetShaderInfoLog( shader, len, &len, log );
		char error_string[MAX_LEN_MESSAGES];
		snprintf( error_string, MAX_LEN_MESSAGES, "Compiler error: '%s'", log );
		logbook_log( LOG_ERROR, error_string );
		free( log );
		glDeleteShader( shader );
		return false;
	}
	return true;
}
