/* A rectangular, [0.0..1.0] clamped regular flat mesh.
 * X and Z are the horizontal dimensions. Y will be extruded by the heightmap. */

#pragma once

#include "settings.h"
#include "glad/glad.h"

struct gridmesh_t {
	unsigned int dimension;
	GLsizei end_index_tl;
	GLsizei end_index_tr;
	GLsizei end_index_bl;
	GLsizei end_index_br;
	GLsizei num_indices;
	GLuint vertex_array;
	GLuint index_buffer;
	GLuint vertex_buffer;
};

gridmesh_t *gridmesh_create( const unsigned int dimension, gridmesh_t *gridmesh );

extern gridmesh_t *gridmesh_delete( gridmesh_t *gridmesh );

extern void gridmesh_bind( const gridmesh_t *const gridmesh );
