
#include "heightmap.h"
#include "quadtree.h"
#include "gridmesh.h"
#include "lod_selection.h"
#include "terrain_tile.h"
#include "base/logbook.h"
#include "renderer/shader_program.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

terrain_tile_t *terrain_tile_create(
		const char *texture_filename, const char *aabb_filename, const bool list_nodes, terrain_tile_t *tile ) {
	if( tile ) {
		logbook_log( LOG_ERROR, "Non null pointer passed to terrain_tile_create" );
		return tile;
	}
	tile = malloc(sizeof(terrain_tile_t));
	if( !tile ) {
		logbook_log( LOG_ERROR, "Error allocating terrain tile memory" );
		return NULL;
	}
	tile->heightmap = NULL;
	tile->quadtree = NULL;
	strncpy( tile->filename, texture_filename, MAX_LEN_FILENAMES-1 );
	// Load the heightmap and tile relative and world min/max coords for the bounding boxes
	// @todo: check if size == terrain::TILE_SIZE !
	tile->heightmap = heightmap_create( texture_filename, tile->heightmap );
	char msg[MAX_LEN_MESSAGES];
	if( !tile->heightmap ) {
		snprintf( msg, MAX_LEN_MESSAGES-1, "Error loading heightmap texture '%s'", texture_filename );
		logbook_log( LOG_ERROR, msg );
		return terrain_tile_delete(tile);
	}
	FILE *bb = fopen( aabb_filename, "r" );
	if( !bb ) {
		snprintf( msg, MAX_LEN_MESSAGES-1,
				"Error loading heightmap bounding box file '%s'", aabb_filename );
		logbook_log( LOG_ERROR, msg );
		return terrain_tile_delete(tile);
		return NULL;
	}
	if( 6 != fscanf( bb, "%f %f %f %f %f %f",
			&tile->aabb.min.x, &tile->aabb.min.y, &tile->aabb.min.z,
			&tile->aabb.max.x, &tile->aabb.max.y, &tile->aabb.max.z ) ) {
		snprintf( msg, MAX_LEN_MESSAGES-1,
				"Error reading heightmap bounding box '%s'. Wrong format ?", aabb_filename );
		logbook_log( LOG_ERROR, msg );
		fclose(bb);
		return terrain_tile_delete(tile);
	}
	fclose(bb);

	// Build quadtree with nodes and their bounding boxes.
	tile->quadtree = quadtree_create( tile, list_nodes, tile->quadtree );
	if( !tile->quadtree ) {
		snprintf( msg, MAX_LEN_MESSAGES-1, "Error '%s' could not be loaded because quadtree error", tile->filename );
		logbook_log( LOG_ERROR, msg );
		return terrain_tile_delete(tile);
	}

	// report success
	snprintf( msg, MAX_LEN_MESSAGES-1,
			"Terrain tile '%s' loaded. Bounding box (%.2f/%.2f/%.2f)/(%.2f/%.2f/%.2f)",
			tile->filename, tile->aabb.min.x, tile->aabb.min.y, tile->aabb.min.z,
			tile->aabb.max.x, tile->aabb.max.y, tile->aabb.max.z );
	logbook_log( LOG_INFO, msg );
	return tile;
}

inline terrain_tile_t *terrain_tile_delete( terrain_tile_t *tile ) {
	if( tile ) {
		if( tile->heightmap )
			heightmap_delete( tile->heightmap );
		if( tile->quadtree )
			quadtree_delete( tile->quadtree );
		char msg[MAX_LEN_MESSAGES];
		snprintf( msg, MAX_LEN_MESSAGES-1, "terrain tile '%s' deleted/cleaned up", tile->filename );
		logbook_log( LOG_INFO, msg );
		free(tile); tile = NULL;
	}
	return tile;
}

/* Needs a pointer to the shader to pass in uniforms
 * Returns number of rendered nodes (x) and triangles (y) */
void terrain_tile_render(
		const GLuint shader, const gridmesh_t *const gridmesh, const unsigned int tile_index,
		const GLenum draw_mode, const terrain_tile_t *const tile, int *num_tris, int *num_nodes ) {
	*num_tris = 0; *num_nodes = 0;
	heightmap_bind( tile->heightmap );
	// Submeshes are evenly spaced in index buffer. Else calc offsets individually.
	const int half_d = gridmesh->end_index_tl;
	// Iterate through the lod selection's lod levels
	for( unsigned int i = lod_selection_get_min_level(); i <= lod_selection_get_max_level(); ++i ) {
		const unsigned int filter_lod_level = i;
		unsigned int prev_morph_const_level_set = 1000000;
		for( unsigned int i = 0; i < lod_selection_get_selection_count(); ++i ) {
			const selected_node_t *n = lod_selection_get_selected_node(i);
			// Only draw tiles of the currently bound heightmap; filter out nodes if not of the current level
			if( n->tile_index != tile_index || filter_lod_level != n->lod_level )
				continue;
			// Set LOD level specific consts if they have changed from last lod level
			if( prev_morph_const_level_set != n->lod_level ) {
				prev_morph_const_level_set = n->lod_level;
				const vec4f v = lod_selection_get_morph_consts( prev_morph_const_level_set-1 );
				sp_set_uniform_vec4f( shader, "g_morphConsts", &v );
			}
			bool draw_full = n->hasTL && n->hasTR && n->hasBL && n->hasBR;
			const aabb_t *const bb = &n->node->aabb;
			// .w holds the current lod level
			vec3f size;
			aabb_get_size(bb,&size);
			const vec4f nodeScale = { size.x, 0.0f, size.z, (float)n->lod_level };
			const vec3f nodeOffset = { bb->min.x, (bb->min.y+bb->max.y) * 0.5f, bb->min.z };
			sp_set_uniform_vec4f( shader, "g_nodeScale", &nodeScale );
			sp_set_uniform_vec3f( shader, "g_nodeOffset", &nodeOffset );
			if( draw_full ) {
				glDrawElements( draw_mode, gridmesh->num_indices, GL_UNSIGNED_INT, NULL );
				(*num_nodes)++;
				*num_tris += gridmesh->num_indices / 3;
			} else {
				const GLsizeiptr i_size = (GLsizeiptr)sizeof(GLuint);
				// can be optimized by combining calls
				if( n->hasTL ) {
					glDrawElements( draw_mode, half_d, GL_UNSIGNED_INT, NULL );
					(*num_nodes)++;
					*num_tris += half_d / 3;
				}
				if( n->hasTR ) {
					glDrawElements( draw_mode, half_d, GL_UNSIGNED_INT, (void *)(gridmesh->end_index_tl*i_size ) );
					(*num_nodes)++;
					*num_tris += half_d / 3;
				}
				if( n->hasBL ) {
					glDrawElements( draw_mode, half_d, GL_UNSIGNED_INT, (void *)(gridmesh->end_index_tr*i_size) );
					(*num_nodes)++;
					*num_tris += half_d / 3;
				}
				if( n->hasBR ) {
					glDrawElements( draw_mode, half_d, GL_UNSIGNED_INT, (void *)( gridmesh->end_index_bl*i_size) );
					(*num_nodes)++;
					*num_tris += half_d / 3;
				}
			}
		}
	}
}
