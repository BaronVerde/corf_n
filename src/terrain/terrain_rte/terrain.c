
#include "terrain.h"
#include "terrain_tile.h"
#include "quadtree.h"
#include "lod_selection.h"
#include "gridmesh.h"
#include "base/logbook.h"
#include "base/camera.h"
#include "base/window.h"
#include "omath/common.h"
#include "omath/mat4.h"
#include "renderer/color.h"
#include "renderer/shader_program.h"
#include "debug_drawing.h"
#include <stddef.h>
#include <string.h>

static void debug_draw_boxes();

// number of currently loaded tiles
static unsigned int num_tiles = 0;

static struct {
	gridmesh_t *gridmesh;
	// @todo data structure, loading and unloading
	terrain_tile_t *tiles[TERRAIN_MAX_TILES];
	GLuint shader;
	mat4f model_matrix;
} terrain;

bool terrain_create( const bool list_nodes ) {
	if( !debug_drawing_create() )
		return false;
	terrain.gridmesh = NULL;
	// Prepare and check settings
	if( !is_pow2(LEAF_NODE_SIZE) || LEAF_NODE_SIZE < 8 || LEAF_NODE_SIZE > 1024 ) {
		logbook_log( LOG_ERROR, "Settings LEAF_NODE_SIZE must be power of 2 and between 2 and 1024" );
		return false;
	}
	if( !is_pow2(TILE_SIZE) || TILE_SIZE < 2 * LEAF_NODE_SIZE || TILE_SIZE > 16384 ) {
		logbook_log( LOG_ERROR,
				"Settings TILE_SIZE must be power of 2 and between 2*LEAF_NODE_SIZE and 16384" );
		return false;
	}
	if( !is_pow2(RENDER_GRID_RESULUTION_MULT) ||
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
	if( !is_pow2(GRIDMESH_DIMENSION) ||	GRIDMESH_DIMENSION<8 || GRIDMESH_DIMENSION>1024 ) {
		logbook_log( LOG_ERROR, "Gridmesh dimension must be power of 2 and > 8 and < 1024." );
		return false;
	}
	// Prepare gridmesh for drawing and load terrain tiles
	terrain.gridmesh = gridmesh_create( GRIDMESH_DIMENSION, terrain.gridmesh );
	if( !terrain.gridmesh )
		return false;
	terrain.tiles[0] = terrain_tile_create(
			TERRAIN_PATH"tile_2048_1.png", TERRAIN_PATH"tile_2048_1.bb", list_nodes, terrain.tiles[0]
	);
	terrain.tiles[1] = terrain_tile_create(
			TERRAIN_PATH"tile_2048_2.png", TERRAIN_PATH"tile_2048_2.bb", list_nodes, terrain.tiles[1]
	);
	terrain.tiles[2] = terrain_tile_create(
			TERRAIN_PATH"tile_2048_3.png", TERRAIN_PATH"tile_2048_3.bb", list_nodes, terrain.tiles[2]
	);
	terrain.tiles[3] = terrain_tile_create(
			TERRAIN_PATH"tile_2048_4.png", TERRAIN_PATH"tile_2048_4.bb", list_nodes, terrain.tiles[3]
	);
	if( !terrain.tiles[0] || !terrain.tiles[1] || !terrain.tiles[2] || !terrain.tiles[3] ) {
		terrain_delete();
		return false;
	}
	num_tiles = 4;
	// Create terrain shaders
	if( !sp_create( SHADER_PATH"terrain.vert.glsl", SHADER_PATH"terrain.frag.glsl", &terrain.shader ) ) {
		terrain_delete();
		return false;
	}
	return true;
}

void terrain_delete() {
	if( terrain.gridmesh )
		terrain.gridmesh = gridmesh_delete(terrain.gridmesh);
	if( num_tiles > 0 )
		for( unsigned int i = 0; i < num_tiles; ++i )
			terrain.tiles[i] = terrain_tile_delete(terrain.tiles[i]);
	if( glIsProgram(terrain.shader) )
		glDeleteProgram(terrain.shader);
	debug_drawing_delete();
}

bool terrain_setup() {
	// Camera and selection object. Are connected because selection is based on view frustum and range
	// @todo parametrize or calculate initial position, direction and view range
	const vec3f position = { 0.0f, 100.0f, 0.0f };
	const vec3f target = { 2047.0f, 50.0f, 2047.0f };
	camera_set_position_and_target( &position, &target );
	camera_set_near_far_plane( 10.0f, 4000.0f );
	// @todo: no sorting for now. Should be sorted by tileIndex, distanceToCamera and lodLevel
	// to avoid too many heightmap switches and shader uniform settings.
	// Lod selection ranges depend on camera near/far plane distances
	lod_selection_create( false /*don't sort*/ );
	// @todo m_drawPrimitives.setupDebugDrawing();
	// Set global shader uniforms valid for all tiles
	glUseProgram(terrain.shader);
	// Set default global shader uniforms belonging to this quadtree/heightmap
	// tile extent corresponds to heightmap extent
	const float tile_extent = (float)TILE_SIZE;
	// Used to clamp edges to correct terrain size (only max-es needs clamping, min-s are clamped implicitly)
	const vec2f tile_to_texture = { (tile_extent-1.0f) / tile_extent, (tile_extent-1.0f) / tile_extent };
	const vec4f height_map_info = { tile_extent, tile_extent, 1.0f/tile_extent, 1.0f/tile_extent };
	sp_set_uniform_vec2f( terrain.shader, "g_tileToTexture", &tile_to_texture );
	sp_set_uniform_vec4f( terrain.shader, "g_heightmapTextureInfo", &height_map_info );
	sp_set_uniform_float( terrain.shader, "u_height_factor", 2.0f );
	// Set dimensions of the gridmesh used for rendering an individual node
	vec3f grid_dim = {
			(float)GRIDMESH_DIMENSION, (float)GRIDMESH_DIMENSION*0.5f, 2.0f/(float)GRIDMESH_DIMENSION
	};
	sp_set_uniform_vec3f( terrain.shader, "g_gridDim", &grid_dim );
	// @todo: Global lighting. In the long run, this will be done elsewhere ...
	const vec4f light_color_ambient = { 0.35f, 0.35f, 0.35f, 1.0f };
	const vec4f light_color_diffuse = { 0.65f, 0.65f, 0.65f, 1.0f };
	//const vec4f fog_color = { 0.0f, 0.5f, 0.5f, 1.0f };
	const vec4f color_mult = { 1.0f, 1.0f, 1.0f, 1.0f };
	const vec3f diffuse_light_pos = { -5.0f, -1.0f, 0.0f };
	const float id[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	memcpy( &terrain.model_matrix, id, sizeof(terrain.model_matrix) );
	sp_set_uniform_vec4f( terrain.shader, "g_lightColorDiffuse", &light_color_diffuse );
	sp_set_uniform_vec4f( terrain.shader, "g_lightColorAmbient", &light_color_ambient );
	sp_set_uniform_vec4f( terrain.shader, "g_colorMult", &color_mult );
	sp_set_uniform_vec3f( terrain.shader, "g_diffuseLightDir", &diffuse_light_pos );
	return true;
}

void terrain_render( const bool draw_boxes, const bool draw_terrain ) {
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	// reset selection, add nodes, sort selection, sort by tile index, nearest to farest
	lod_selection_reset();
	for( unsigned int i = 0; i < num_tiles; ++i ) {
		lod_selection_set_tile_index(i);
		quadtree_lod_select(terrain.tiles[i]->quadtree);
	}
	lod_selection_sort();

	//lod_selection_print();
	if( draw_boxes )
		debug_draw_boxes();
	if( !draw_terrain )
		return;

	// Bind meshes, shader, reset stats, prepare and set matrices and cam pos
	gridmesh_bind(terrain.gridmesh);
	int num_rendered_triangles = 0;
	int num_rendered_nodes = 0;
	glUseProgram(terrain.shader);
	// Build view projection matrix relative to eye
	vec3f looking_dir;
	vec3f_add( camera_get_position(), camera_get_front(), &looking_dir );
	mat4f view, model_view, mv_rte, mvp_rte;
	mat4f_lookat( camera_get_position(), &looking_dir, camera_get_up(), &view );
	// Identity matrix as model matrix so far
	mat4f_mul( &view, &terrain.model_matrix, &model_view );
	// the mv_rte matrix can be used for all objects in same coord system
	memcpy( mv_rte.data, model_view.data, sizeof(model_view) );
	mv_rte.data[12] = 0.0f;
	mv_rte.data[13] = 0.0f;
	mv_rte.data[14] = 0.0f;
	mat4f_mul( camera_get_projection_matrix(), &mv_rte, &mvp_rte );
	sp_set_model_view_projection_matrix( &mvp_rte );
	sp_set_view_projection_matrix( camera_get_view_projection_matrix() );
	sp_set_uniform_vec4f( terrain.shader, "debugColor", &color_gray );
	sp_set_camera_position( camera_get_position() );
	// Draw tile by tile
	const GLenum draw_mode = window_get_draw_mode();
	for( unsigned int i = 0; i < num_tiles; ++i ) {
		// set tile world coords
		const vec2f tile_max = { terrain.tiles[i]->aabb.max.x, terrain.tiles[i]->aabb.max.z };
		sp_set_uniform_vec2f( terrain.shader, "g_tileMax", &tile_max );
		vec3f tile_scale;
		vec3f_sub( &terrain.tiles[i]->aabb.max, &terrain.tiles[i]->aabb.min, &tile_scale );
		sp_set_uniform_vec3f(
				terrain.shader, "g_tileScale",
				vec3f_sub( &terrain.tiles[i]->aabb.max, &terrain.tiles[i]->aabb.min, &tile_scale )
		);
		sp_set_uniform_vec3f( terrain.shader, "g_tileOffset", &terrain.tiles[i]->aabb.min );
		int num_tris, num_nodes;
		terrain_tile_render( terrain.shader, terrain.gridmesh, i, draw_mode, terrain.tiles[i], &num_tris, &num_nodes );
		num_rendered_triangles += num_tris;
		num_rendered_nodes += num_nodes;
	}
}

void terrain_cleanup() {}

void debug_draw_boxes() {
	// To keep the below less verbose
	glUseProgram(debug_drawing.shader);
	sp_set_uniform_mat4f( debug_drawing.shader, "projViewMatrix", camera_get_view_projection_matrix() );
	for( unsigned int i = 0; i < num_tiles; ++i ) {
			for( unsigned int i = 0; i < lod_selection_get_selection_count(); ++i ) {
				const selected_node_t *n = lod_selection_get_selected_node(i);
				const bool draw_full = n->hasTL && n->hasTR && n->hasBL && n->hasBR;
				if( draw_full )
					// expand by -0.003f
					debug_drawing_draw_aabb( &n->node->aabb, &color_rainbow[n->node->level] );
				else {
					if( n->hasTL )
						// expand by -0.002f
						debug_drawing_draw_aabb( &n->node->subTL->aabb, &color_rainbow[n->node->subTL->level] );
					if( n->hasTR )
						// expand by -0.002f
						debug_drawing_draw_aabb( &n->node->subTR->aabb, &color_rainbow[n->node->subTR->level] );
					if( n->hasBL )
						// expand by -0.002f
						debug_drawing_draw_aabb( &n->node->subBL->aabb, &color_rainbow[n->node->subBL->level] );
					if( n->hasBR )
						// expand by -0.002f
						debug_drawing_draw_aabb( &n->node->subBR->aabb, &color_rainbow[n->node->subBR->level] );
				}
			}
	}
}
