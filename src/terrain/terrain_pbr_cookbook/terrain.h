
#pragma once

#include <stdbool.h>
#include "settings.h"

typedef enum { requested, loading, ready } tile_status_t;

typedef struct tiles_t {
	terrain_tile_t *tile;
	unsigned int tile_index;
	tile_status_t status;
} tiles_t;

struct terrain_t {
	gridmesh_t *gridmesh;
	// @todo data structure, loading and unloading
	unsigned int num_tiles;
	terrain_tile_t *tiles[TERRAIN_MAX_TILES];
	GLuint shader;
	// Is identity
	//mat4f model_matrix;
	// shader uniform locations
	GLint u_height_factor;
	GLint u_tile_offset;
	GLint u_tile_scale;
	GLint u_tile_max;
	GLint u_tile_to_texture;
	GLint u_heightmap_texture_info;
	GLint u_griddim;
	GLint u_node_offset;
	GLint u_node_scale;
	GLint u_morph_consts;
	GLint u_camera_position;
	// Is identical to vp-matrix GLint u_model_view_projection_matrix;
	GLint u_view_projection_matrix;
	GLint u_model_view_matrix;
	// lighting
	GLint u_light_position;
	GLint u_light_intensity;
	GLint u_normal_matrix;
	GLint u_terrain_roughness;
	GLint u_terrain_metal;
	GLint u_terrain_color;
};

// list_nodes, when true, causes verbose logging put of quadtree built nodes and lod_selection nodes
bool terrain_create( const bool list_nodes );

void terrain_delete();

bool terrain_setup();

void terrain_render( const bool draw_boxes, const bool draw_terrain );

void terrain_cleanup();
