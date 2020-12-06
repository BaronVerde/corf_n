
#pragma once

#include "settings.h"
#include "omath/aabb.h"
#include "omath/vec3.h"
#include "omath/vec2.h"
#include "glad/glad.h"

struct terrain_tile_t {
	char filename[MAX_LEN_FILENAMES];
	char bb_file[MAX_LEN_FILENAMES];
	heightmap_t *heightmap;
	quadtree_t *quadtree;
	// Bounding box in world coords
	// @todo: this will have to give way to the cartesian bb, heightmap relative bb for now
	aabb_t aabb;
};

/* Pathname of the tile heightmap
 * Ellispoid is used to calculate world cartesian positions of posts from lower left corner
 * and anular distance between posts. Positions are stored as high/low floats in two textures.
 * two files needed: the 16 bit monochrome texture and the bounding box in world coords */
terrain_tile_t *terrain_tile_create(
		const char *texture_filename, const char *aabb_filename, const bool list_nodes, terrain_tile_t *tile );

extern terrain_tile_t *terrain_tile_delete( terrain_tile_t *tile );

/* Needs a pointer to the shader to pass in uniforms
 * Returns number of rendered nodes (x) and triangles (y) */
void terrain_tile_render(
		const GLuint shader, const gridmesh_t *const gridmesh, const unsigned int tile_index,
		const GLenum draw_mode, const terrain_tile_t *const tile, int *num_tris, int *num_nodes
);

// Center of the tile in world cartesian coords
//extern vec3f getWorldCenter();

// @todo: angular step between heightmap posts
//extern double getCellSize();
