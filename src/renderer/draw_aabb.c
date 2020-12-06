
#include "draw_aabb.h"
#include "omath/mat4.h"
#include "omath/vec3.h"
#include "terrain/settings.h"
#include "shader_program.h"

static struct {
	GLuint box_vertex_array;
	GLuint box_vertex_buffer;
	GLuint box_index_buffer;
	GLuint shader;
} draw_aabb_info;

bool draw_aabb_create() {
	// Load shaders
	if( !sp_create( "src/renderer/draw_aabb.vert.glsl",
			"src/renderer/draw_aabb.frag.glsl", &draw_aabb_info.shader ) )
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
	glCreateVertexArrays( 1, &draw_aabb_info.box_vertex_array );
	glCreateBuffers( 1, &draw_aabb_info.box_vertex_buffer);
	glNamedBufferStorage(
			draw_aabb_info.box_vertex_buffer, (GLsizeiptr)sizeof(box_vertices), box_vertices, 0
	);
	const GLuint binding_index = 0;
	glVertexArrayVertexBuffer(
			draw_aabb_info.box_vertex_array, binding_index, draw_aabb_info.box_vertex_buffer, 0, sizeof(vec3f)
	);
	const GLuint attrib_location = 0;
	glVertexArrayAttribBinding( draw_aabb_info.box_vertex_array, attrib_location, binding_index );
	glVertexArrayAttribFormat( draw_aabb_info.box_vertex_array, attrib_location, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( draw_aabb_info.box_vertex_array, attrib_location );
	glCreateBuffers( 1, &draw_aabb_info.box_index_buffer );
	glNamedBufferData(
			draw_aabb_info.box_index_buffer, (GLsizeiptr)sizeof(box_loop_indices), box_loop_indices, GL_STATIC_DRAW
	);
	glVertexArrayElementBuffer( draw_aabb_info.box_vertex_array, draw_aabb_info.box_index_buffer );
	return true;
}

void draw_aabb( const aabbf *const bb, const color_t *const color ) {
	vec3f c, s;
	aabbf_get_center( bb, &c );
	aabbf_get_size( bb, &s );
	mat4f model_m,
		  translation = { .data = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } },
		  scale = { .data = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } };
	mat4f_translate( &c, &translation );
	mat4f_scale( &s, &scale );
	mat4f_mul( &translation, &scale, &model_m );
	// * omath::scale( m, omath::vec3{ bb.get_size() } ) };
	sp_set_uniform_mat4f( draw_aabb_info.shader, "modelMatrix", &model_m );
	sp_set_uniform_vec4f( draw_aabb_info.shader, "debugColor", color );
	// Frame
	glBindVertexArray(draw_aabb_info.box_vertex_array);
	glDrawElements( GL_LINE_LOOP, 16, GL_UNSIGNED_INT, NULL );
}

inline void draw_aabb_delete() {
	if( glIsBuffer(draw_aabb_info.box_index_buffer) )
		glDeleteBuffers(1, &draw_aabb_info.box_index_buffer);
	if( glIsBuffer(draw_aabb_info.box_vertex_buffer) )
		glDeleteBuffers(1, &draw_aabb_info.box_vertex_buffer);
	if( glIsVertexArray(draw_aabb_info.box_vertex_array) )
		glDeleteVertexArrays(1, &draw_aabb_info.box_vertex_array);
}

inline GLuint draw_abb_get_program() {
	return draw_aabb_info.shader;
}
