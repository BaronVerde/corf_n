
// Debug drawing boxes

#pragma once

#include "omath/aabb.h"
#include "glad/glad.h"
#include "omath/vec3.h"
#include "omath/vec4.h"
#include "omath/mat4.h"
#include "renderer/shader_program.h"
#include "settings.h"
#include <stdbool.h>

// the element array is small
static const GLuint primitive_restart_index = 100;

static struct {
	GLuint box_vertex_array;
	GLuint box_vertex_buffer;
	GLuint box_index_buffer;
	GLuint shader;
} debug_drawing;

bool debug_drawing_create() {
	// Load shaders
	if( !sp_create( SHADER_PATH"debug_drawing.vert.glsl", SHADER_PATH"debug_drawing.frag.glsl", &debug_drawing.shader ) )
		return false;
	// vertex data and triangle loop indices for an box
	const vec3f box_vertices[8] = {
		{ -0.5, -0.5, -0.5 },
		{  0.5, -0.5, -0.5 },
		{  0.5,  0.5, -0.5 },
		{ -0.5,  0.5, -0.5 },
		{ -0.5, -0.5,  0.5 },
		{  0.5, -0.5,  0.5 },
		{  0.5,  0.5,  0.5 },
		{ -0.5,  0.5,  0.5 }
	};
	const GLuint box_loop_indices[16] = {
			0, 4, 0, 1, 5, 1, 2, 6, 2, 3, 7, 6, 5, 4, 7, 3
	};
	glCreateVertexArrays( 1, &debug_drawing.box_vertex_array );
	glCreateBuffers( 1, &debug_drawing.box_vertex_buffer);
	glNamedBufferStorage(
			debug_drawing.box_vertex_buffer, (GLsizeiptr)sizeof(box_vertices), box_vertices, 0
	);
	const GLuint binding_index = 0;
	glVertexArrayVertexBuffer(
			debug_drawing.box_vertex_array, binding_index, debug_drawing.box_vertex_buffer, 0, sizeof(vec3f)
	);
	const GLuint attrib_location = 0;
	glVertexArrayAttribBinding( debug_drawing.box_vertex_array, attrib_location, binding_index );
	glVertexArrayAttribFormat( debug_drawing.box_vertex_array, attrib_location, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( debug_drawing.box_vertex_array, attrib_location );
	glCreateBuffers( 1, &debug_drawing.box_index_buffer );
	glNamedBufferData(
			debug_drawing.box_index_buffer, (GLsizeiptr)sizeof(box_loop_indices), box_loop_indices, GL_STATIC_DRAW
	);
	glVertexArrayElementBuffer( debug_drawing.box_vertex_array, debug_drawing.box_index_buffer );
	return true;
}

extern void debug_drawing_render() {
	//glPrimitiveRestartIndex( primitive_restart_index );
	//glDrawElements()
}

void debug_drawing_draw_aabb( const aabb_t *const bb, const vec4f *const color ) {
	vec3f c, s;
	aabb_get_center( bb, &c );
	aabb_get_size( bb, &s );
	mat4f model_m,
		  translation = { .data = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } },
		  scale = { .data = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } };
	mat4f_translate( &c, &translation );
	mat4f_scale( &s, &scale );
	mat4f_mul( &translation, &scale, &model_m );
	// * omath::scale( m, omath::vec3{ bb.get_size() } ) };
	sp_set_uniform_mat4f( debug_drawing.shader, "modelMatrix", &model_m );
	sp_set_uniform_vec4f( debug_drawing.shader, "debugColor", color );
	// Frame
	glBindVertexArray(debug_drawing.box_vertex_array);
	glDrawElements( GL_LINE_LOOP, 16, GL_UNSIGNED_INT, NULL );
}

extern bool debug_drawing_reset() {
	return true;
}

void debug_drawing_delete() {
	if( glIsBuffer(debug_drawing.box_index_buffer) )
		glDeleteBuffers(1, &debug_drawing.box_index_buffer);
	if( glIsBuffer(debug_drawing.box_vertex_buffer) )
		glDeleteBuffers(1, &debug_drawing.box_vertex_buffer);
	if( glIsVertexArray(debug_drawing.box_vertex_array) )
		glDeleteVertexArrays(1, &debug_drawing.box_vertex_array);
}
