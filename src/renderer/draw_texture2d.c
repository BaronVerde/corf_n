
#include "draw_texture2d.h"
#include "shader_program.h"
#include "terrain/settings.h"

static struct {
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLuint shader;
} draw_info;

bool draw_texture2d_create() {
	if( !sp_create( "src/renderer/draw_texture2d.vert.glsl",
			"src/renderer/draw_texture2d.frag.glsl", &draw_info.shader ) )
		return false;
	// Positions and texture-coords
	GLfloat vertices[] = {
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
	};
	GLuint indices[] = { 0, 1, 3, 1, 2, 3 };
	glCreateVertexArrays( 1, &draw_info.vertex_array );
	glCreateBuffers( 1, &draw_info.vertex_buffer );
	glNamedBufferStorage(
			draw_info.vertex_buffer, (GLsizeiptr)sizeof(vertices), vertices, 0
	);
    const GLuint binding_index = 0;
    glVertexArrayVertexBuffer(
    		draw_info.vertex_array, binding_index, draw_info.vertex_buffer, 0, 5*sizeof(GLfloat)
    );
    // location 0 - position
    glVertexArrayAttribBinding( draw_info.vertex_array, 0, binding_index );
    glVertexArrayAttribFormat( draw_info.vertex_array, 0, 3, GL_FLOAT, GL_FALSE, 0 );
    glEnableVertexArrayAttrib( draw_info.vertex_array, 0 );
    // location 1 - texture coordinate
    glVertexArrayAttribBinding( draw_info.vertex_array, 1, binding_index );
    glVertexArrayAttribFormat( draw_info.vertex_array, 1, 2, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat) );
    glEnableVertexArrayAttrib( draw_info.vertex_array, 1 );
    glCreateBuffers( 1, &draw_info.index_buffer );
    glNamedBufferData(
    		draw_info.index_buffer, (GLsizeiptr)sizeof(indices), indices, GL_STATIC_DRAW
    );
    glVertexArrayElementBuffer( draw_info.vertex_array, draw_info.index_buffer );
    return true;
}

void draw_texture2d_render( const GLuint texture ) {
	glUseProgram(draw_info.shader);
	glBindTextureUnit(0, texture);
	glBindVertexArray(draw_info.vertex_array);
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
}

void draw_texture2d_delete() {
	if( glIsBuffer(draw_info.index_buffer) )
		glDeleteBuffers(1,&draw_info.index_buffer);
	if( glIsBuffer(draw_info.vertex_buffer) )
		glDeleteBuffers(1,&draw_info.vertex_buffer);
	if( glIsVertexArray(draw_info.vertex_array) )
		glDeleteVertexArrays(1, &draw_info.vertex_array);
	if( glIsShader(draw_info.shader) )
		glDeleteShader(draw_info.shader);
}
