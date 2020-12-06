
#include "renderer/draw_aabb.h"
#include "terrain.h"
#include "terrain_tile.h"
#include "quadtree.h"
#include "lod_selection.h"
#include "gridmesh.h"
#include "base/logbook.h"
#include "heightmap.h"
#include "base/camera.h"
#include "base/window.h"
#include "omath/common.h"
#include "omath/mat4.h"
#include "renderer/color.h"
#include "renderer/shader_program.h"
#include "renderer/sampler.h"
#include <stddef.h>
#include <string.h>

static void debug_draw_boxes();
static inline bool check_params();
static inline void set_shader_uniform_locations();

static struct terrain_t terrain;

bool terrain_create( const bool list_nodes ) {
	// Prepare and check settings
	if( !check_params() )
		return false;
	if( !draw_aabb_create() )
		return false;
	// Prepare gridmesh for drawing and load terrain tiles
	terrain.gridmesh = gridmesh_create( GRIDMESH_DIMENSION, terrain.gridmesh );
	if( !terrain.gridmesh )
		return false;
	terrain.tiles[0] = terrain_tile_create(
			"resources/terrain/area_52_06/tile_4096_1.png", "resources/terrain/area_52_06/tile_4096_1.bb",
			list_nodes, terrain.tiles[0]
	);
	if( !terrain.tiles[0] ) {
		terrain_delete();
		return false;
	}
	const unsigned int size = terrain.tiles[0]->heightmap->extent;
	if( !is_pow2u(size) || size < 2 * LEAF_NODE_SIZE || size > 16384 ) {
		logbook_log( LOG_ERROR, "Terrain tile extent must be pow2 and between 2*LEAF_NODE_SIZE and 16384" );
		terrain_delete();
		return false;
	}
	terrain.num_tiles = 1;
	// Create terrain shaders
	if( !sp_create( "src/terrain/terrain.vert.glsl", "src/terrain/terrain.frag.glsl", &terrain.shader ) ) {
		terrain_delete();
		return false;
	}
	set_shader_uniform_locations();
	return true;
}

bool terrain_setup() {
	// Camera and selection object. Are connected because selection is based on view frustum and range
	const vec3f position = { 0.0f, 100.0f, 0.0f };
	const vec3f target = { 2047.0f, 50.0f, 2047.0f };
	camera_set_position_and_target( &position, &target );
	// @todo Separate camera view range and lod ranges
	camera_set_near_far_plane( 1.0f, 4000.0f );
	// @todo Should be sorted by tileIndex, distanceToCamera and lodLevel
	const bool sort_selection = false;
	lod_selection_create(sort_selection);
	// Set global shader uniforms valid for all tiles
	glUseProgram(terrain.shader);
	// Set global shader uniforms for this quadtree/heightmap; tile extent = heightmap extent for now
	const float extent = (float)terrain.tiles[0]->heightmap->extent;
	// Used to clamp edges to correct terrain extent (only max-es needs clamping, min-s are clamped implicitly)
	glUniform2f( terrain.u_tile_to_texture, (extent-1.0f)/extent, (extent-1.0f)/extent );
	glUniform4f( terrain.u_heightmap_texture_info, extent, extent, 1.0f/extent, 1.0f/extent );
	glUniform1f( terrain.u_height_factor, (GLfloat)HEIGHT_FACTOR );
	const float dim = (float)GRIDMESH_DIMENSION;
	glUniform3f( terrain.u_griddim, dim, dim*0.5f, 2.0f/dim );
	// Global lighting
	//const vec4f fog_color = { 0.0f, 0.5f, 0.5f, 1.0f };
	// .w = 0 means directional light, else (point light) position = view_matrix * light_position
	glUniform4f( terrain.u_light_position, 0.3f, 0.5f, 0.0f, 0.0f );
	glUniform3f( terrain.u_light_intensity, 2.5f, 2.5f, 2.5f );
	glUniform1f( terrain.u_terrain_roughness, 0.9f );
	glUniform1f( terrain.u_terrain_metal, GL_FALSE );
	glUniform3f( terrain.u_terrain_color, 0.3f, 0.2f, 0.2f );
	return true;
}

void terrain_render( const bool draw_boxes, const bool draw_terrain ) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	lod_selection_reset();
	for( unsigned int i = 0; i < terrain.num_tiles; ++i ) {
		lod_selection_set_tile_index(i);
		quadtree_lod_select(terrain.tiles[i]->quadtree);
	}
	lod_selection_sort();
	const bool print_selection = false;
	if( print_selection )
		lod_selection_print();

	if( draw_boxes )
		debug_draw_boxes();
	if( !draw_terrain )
		return;

	gridmesh_bind(terrain.gridmesh);
	int num_rendered_triangles = 0;
	int num_rendered_nodes = 0;
	glUseProgram(terrain.shader);
	// Matrices for lighting, mv, normal and mvp matrices, but model matrix is identity
	glUniformMatrix4fv( terrain.u_model_view_matrix, 1, GL_FALSE, (float*)camera_get_view_matrix() );
	glUniformMatrix4fv( terrain.u_view_projection_matrix, 1, GL_FALSE, (float*)camera_get_view_projection_matrix() );
	// model_matrix * view_matrix
	mat3f normal_matrix, basis, trans;
	mat4f_get_basis( camera_get_view_matrix(), &basis );
	mat3f_transpose( &basis, &trans );
	mat3f_inverse( &trans, &normal_matrix );
	glUniformMatrix3fv( terrain.u_normal_matrix, 1, GL_FALSE, (float*)&normal_matrix );
	// Matrices and uniforms for terrain CDLOD
	glUniform3fv( terrain.u_camera_position, 1, (float*)camera_get_position() );
	// Draw tile by tile
	const GLenum draw_mode = window_get_draw_mode();
	for( unsigned int i = 0; i < terrain.num_tiles; ++i ) {
		// set tile world coords
		glUniform2f( terrain.u_tile_max, terrain.tiles[i]->aabb.max.x, terrain.tiles[i]->aabb.max.z );
		vec3f tile_scale;
		vec3f_sub( &terrain.tiles[i]->aabb.max, &terrain.tiles[i]->aabb.min, &tile_scale );
		glUniform3fv( terrain.u_tile_scale, 1, (float*)&tile_scale );
		glUniform3fv( terrain.u_tile_offset, 1, (float*)&terrain.tiles[i]->aabb.min );
		int num_tris, num_nodes;
		terrain_tile_render( &terrain, i, draw_mode, &num_tris, &num_nodes );
		num_rendered_triangles += num_tris;
		num_rendered_nodes += num_nodes;
	}
}

void terrain_cleanup() {}

void terrain_delete() {
	if( terrain.gridmesh )
		terrain.gridmesh = gridmesh_delete(terrain.gridmesh);
	if( terrain.num_tiles > 0 )
		for( unsigned int i = 0; i < terrain.num_tiles; ++i )
			terrain.tiles[i] = terrain_tile_delete(terrain.tiles[i]);
	if( glIsProgram(terrain.shader) )
		glDeleteProgram(terrain.shader);
	draw_aabb_delete();
}

// *** static stuff
void debug_draw_boxes() {
	// To keep the below less verbose
	glUseProgram(draw_abb_get_program());
	sp_set_uniform_mat4f( draw_abb_get_program(), "projViewMatrix", camera_get_view_projection_matrix() );
	for( unsigned int i = 0; i < terrain.num_tiles; ++i ) {
		for( unsigned int i = 0; i < lod_selection_get_selection_count(); ++i ) {
			const selected_node_t *n = lod_selection_get_selected_node(i);
			const bool draw_full = n->hasTL && n->hasTR && n->hasBL && n->hasBR;
			if( draw_full )
				// expand by -0.003f
				draw_aabb( &n->node->aabb, &color_rainbow[n->node->level] );
			else {
				if( n->hasTL )
					// expand by -0.002f
					draw_aabb( &n->node->subTL->aabb, &color_rainbow[n->node->subTL->level] );
				if( n->hasTR )
					// expand by -0.002f
					draw_aabb( &n->node->subTR->aabb, &color_rainbow[n->node->subTR->level] );
				if( n->hasBL )
					// expand by -0.002f
					draw_aabb( &n->node->subBL->aabb, &color_rainbow[n->node->subBL->level] );
				if( n->hasBR )
					// expand by -0.002f
					draw_aabb( &n->node->subBR->aabb, &color_rainbow[n->node->subBR->level] );
			}
		}
	}
}

bool check_params() {
	if( !is_pow2u(LEAF_NODE_SIZE) || LEAF_NODE_SIZE < 8 || LEAF_NODE_SIZE > 1024 ) {
		logbook_log( LOG_ERROR, "Settings LEAF_NODE_SIZE must be power of 2 and between 2 and 1024" );
		return false;
	}
	if( !is_pow2u(RENDER_GRID_RESULUTION_MULT) ||
			RENDER_GRID_RESULUTION_MULT<1 || RENDER_GRID_RESULUTION_MULT>LEAF_NODE_SIZE ) {
		logbook_log( LOG_ERROR,
				"Settings RENDER_GRID_RESULUTION_MULT must be power of 2 and between 1 and LEAF_NODE_SIZE" );
		return false;
	}
	if( NUMBER_OF_LOD_LEVELS < 2 || NUMBER_OF_LOD_LEVELS > 15 ) {
		logbook_log( LOG_ERROR, "Settings NUMBER_OF_LOD_LEVELS must be between 1 and 15" );
		return false;
	}
	if( LOD_LEVEL_DISTANCE_RATIO < 1.5f || LOD_LEVEL_DISTANCE_RATIO > 16.0f ) {
		logbook_log( LOG_ERROR, "Settings LOD_LEVEL_DISTANCE_RATIO must be between 1.5f and 16.0f" );
		return false;
	}
	if( !is_pow2u(GRIDMESH_DIMENSION) ||	GRIDMESH_DIMENSION<8 || GRIDMESH_DIMENSION>1024 ) {
		logbook_log( LOG_ERROR, "Gridmesh dimension must be power of 2 and > 8 and < 1024." );
		return false;
	}
	return true;
}

static inline void set_shader_uniform_locations() {
	glUseProgram(terrain.shader);
	terrain.u_height_factor = glGetUniformLocation( terrain.shader, "u_height_factor" );
	terrain.u_tile_offset = glGetUniformLocation( terrain.shader, "u_tile_offset" );
	terrain.u_tile_scale = glGetUniformLocation( terrain.shader, "u_tile_scale" );
	terrain.u_tile_max = glGetUniformLocation( terrain.shader, "u_tile_max" );
	terrain.u_tile_to_texture = glGetUniformLocation( terrain.shader, "u_tile_to_texture" );
	terrain.u_heightmap_texture_info = glGetUniformLocation( terrain.shader, "u_heightmap_texture_info" );
	terrain.u_griddim = glGetUniformLocation( terrain.shader, "u_griddim" );
	terrain.u_node_offset = glGetUniformLocation( terrain.shader, "u_node_offset" );
	terrain.u_node_scale = glGetUniformLocation( terrain.shader, "u_node_scale" );
	terrain.u_morph_consts = glGetUniformLocation( terrain.shader, "u_morph_consts" );
	terrain.u_camera_position = glGetUniformLocation( terrain.shader, "u_camera_position" );
	terrain.u_view_projection_matrix = glGetUniformLocation( terrain.shader, "u_view_projection_matrix" );
	terrain.u_model_view_matrix = glGetUniformLocation( terrain.shader, "u_model_view_matrix" );
	// Is identical to vp-matrix
	//terrain.u_model_view_projection_matrix;
	terrain.u_normal_matrix = glGetUniformLocation( terrain.shader, "u_normal_matrix" );
	terrain.u_light_position = glGetUniformLocation( terrain.shader, "u_light_position" );
	terrain.u_light_intensity = glGetUniformLocation( terrain.shader, "u_light_intensity" );
	terrain.u_terrain_roughness = glGetUniformLocation( terrain.shader, "u_terrain_roughness" );
	terrain.u_terrain_metal = glGetUniformLocation( terrain.shader, "u_terrain_metal" );
	terrain.u_terrain_color = glGetUniformLocation( terrain.shader, "u_terrain_color" );
}
