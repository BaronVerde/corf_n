
#include "gridmesh.h"
#include "base/logbook.h"
#include "omath/vec3.h"
#include <stdlib.h>
#include <stdio.h>

gridmesh_t *gridmesh_create( const unsigned int dimension, gridmesh_t *gridmesh ) {
	if( !is_pow2u(dimension) || dimension < 16 || dimension > 1024 ) {
		logbook_log( LOG_ERROR, "gridmesh dimension must be power of 2 and between 16 and 1024" );
		return NULL;
	}
	gridmesh = malloc(sizeof(gridmesh_t));
	if( !gridmesh ) {
		logbook_log( LOG_ERROR, "error allocating gridmesh" );
		return NULL;
	}
	gridmesh->dimension = dimension;
	unsigned int total_vertices = ( dimension + 1 ) * ( dimension + 1 );
	gridmesh->num_indices = (GLsizei)(dimension * dimension * 2 * 3);
	vec3f vertices[total_vertices];
	unsigned int vertex_dimension = dimension + 1;
	for( unsigned int y = 0; y < vertex_dimension; ++y )
		for( unsigned int x = 0; x < vertex_dimension; ++x ) {
			vec3f v = { (float)x / (float)dimension, 0.0f, (float)y / (float)dimension };
			vertices[x + vertex_dimension * y] = v;
		}
	glCreateVertexArrays( 1, &gridmesh->vertex_array );
	glCreateBuffers( 1, &gridmesh->vertex_buffer );
	// total_vertices * sizeof(vec3f)
	glNamedBufferData( gridmesh->vertex_buffer, (GLsizeiptr)sizeof(vertices), vertices, GL_STATIC_DRAW );
	glVertexArrayVertexBuffer(
			// array, buffer binding index, buffer, offset, stride
			gridmesh->vertex_array, 0, gridmesh->vertex_buffer, 0, sizeof(vec3f)
	);
	const GLuint attrib_index = 0, binding_index = 0;
	glVertexArrayAttribBinding( gridmesh->vertex_array, attrib_index, binding_index );
	glVertexArrayAttribFormat( gridmesh->vertex_array, attrib_index, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( gridmesh->vertex_array, attrib_index );
	GLuint indices[gridmesh->num_indices];
	unsigned int index = 0;
	unsigned int half_d = vertex_dimension / 2;
	//Top Left
	for( unsigned int y = 0; y < half_d; ++y ) {
		for( unsigned int x = 0; x < half_d; ++x ) {
			indices[index++] = x + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * (y + 1);
		}
	}
	gridmesh->end_index_tl = (GLsizei)index;
	//Top Right
	for( unsigned int y = 0; y < half_d; ++y ) {
		for( unsigned int x = half_d; x < dimension; ++x ) {
			indices[index++] = x + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * (y + 1);
		}
	}
	gridmesh->end_index_tr = (GLsizei)index;
	//Bottom Left
	for( unsigned int y = half_d; y < dimension; ++y ) {
		for( unsigned int x = 0; x < half_d; ++x ) {
			indices[index++] = x + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * (y + 1);
		}
	}
	gridmesh->end_index_bl = (GLsizei)index;
	//Bottom Right
	for( unsigned int y = half_d; y < dimension; ++y ) {
		for( unsigned int x = half_d; x < dimension; ++x ) {
			indices[index++] = x + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = (x + 1) + vertex_dimension * y;
			indices[index++] = x + vertex_dimension * (y + 1);
			indices[index++] = (x + 1) + vertex_dimension * (y + 1);
		}
	}
	gridmesh->end_index_br = (GLsizei)index;
	if( gridmesh->num_indices != (GLsizei)index-- ) {
		char msg[MAX_LEN_MESSAGES-1];
		sprintf( msg, "Gridmesh: number of indices (%d) != precalc number (%d)", gridmesh->num_indices, index-- );
		logbook_log( LOG_ERROR, msg );
		free(gridmesh);
		return NULL;
	}
	glCreateBuffers( 1, &gridmesh->index_buffer );
	// sizeof * gridmesh->num_indices
	glNamedBufferData( gridmesh->index_buffer, (GLsizeiptr)sizeof(indices), indices, GL_STATIC_DRAW );
	glVertexArrayElementBuffer( gridmesh->vertex_array, gridmesh->index_buffer );
	char msg[MAX_LEN_MESSAGES-1];
	sprintf( msg, "Gridmesh dimension %d created", gridmesh->dimension );
	logbook_log( LOG_INFO, msg );
	return gridmesh;
}

inline gridmesh_t *gridmesh_delete( gridmesh_t *gridmesh ) {
	glDisableVertexArrayAttrib( gridmesh->vertex_array, 0 );
	glDeleteBuffers( 1, &gridmesh->index_buffer );
	glDeleteBuffers( 1, &gridmesh->vertex_buffer );
	glDeleteVertexArrays( 1, &gridmesh->vertex_array );
	char msg[MAX_LEN_MESSAGES-1];
	sprintf( msg, "Gridmesh dimension %d destroyed", gridmesh->dimension );
	logbook_log( LOG_INFO, msg );
	free(gridmesh);
	gridmesh = NULL;
	return gridmesh;
}

inline void gridmesh_bind( const gridmesh_t *const gridmesh ) {
	glBindVertexArray( gridmesh->vertex_array );
}
