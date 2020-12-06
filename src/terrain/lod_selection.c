
#include "lod_selection.h"
#include "settings.h"
#include "base/camera.h"
#include "base/logbook.h"
#include <tgmath.h>
#include "omath/aabb.h"
#include <stdio.h>

static struct {
	bool sort_by_distance;
	unsigned int stop_at_level;
	unsigned int current_tile_index;
	unsigned int selection_count;
	selected_node_t selected_nodes[MAX_NUMBER_SELECTED_NODES];
	unsigned int max_selected_lod_level;
	unsigned int min_selected_lod_level;
	float visibility_ranges[NUMBER_OF_LOD_LEVELS];
	float morph_start[NUMBER_OF_LOD_LEVELS];
	float morph_end[NUMBER_OF_LOD_LEVELS];
} lod_selection;

inline void lod_selection_create( bool sort_by_distance ) {
	lod_selection.sort_by_distance = sort_by_distance;
	lod_selection.stop_at_level = NUMBER_OF_LOD_LEVELS;
	// @todo a million tiles should be out of the question ...
	lod_selection.current_tile_index = 1000000;
	lod_selection.selection_count = 0;
	lod_selection.max_selected_lod_level = 0;
	lod_selection.min_selected_lod_level = NUMBER_OF_LOD_LEVELS;
	lod_selection_calculate_ranges();
}

inline void lod_selection_reset() {
	lod_selection.selection_count = 0;
	lod_selection.max_selected_lod_level = 0;
	lod_selection.min_selected_lod_level = NUMBER_OF_LOD_LEVELS;
}

void lod_selection_calculate_ranges() {
	float total = 0.0f;
	float current_detail_balance = 1.0f;
	for( unsigned int i = 0; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		total += current_detail_balance;
		current_detail_balance *= LOD_LEVEL_DISTANCE_RATIO;
	}
	float sect = (camera_get_far_plane()-camera_get_near_plane()) / total;
	float prev_pos = camera_get_near_plane();
	current_detail_balance = 1.0f;
	for( unsigned int i = 0; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		lod_selection.visibility_ranges[NUMBER_OF_LOD_LEVELS-i-1] = prev_pos + sect * current_detail_balance;
		prev_pos = lod_selection.visibility_ranges[NUMBER_OF_LOD_LEVELS-i-1];
		current_detail_balance *= LOD_LEVEL_DISTANCE_RATIO;
	}
	prev_pos = camera_get_near_plane();
	char msg[MAX_LEN_MESSAGES];
	logbook_log( LOG_INFO, "Lod levels and ranges: lvl/range/start/end" );
	for( unsigned int i = 0; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		unsigned int index = NUMBER_OF_LOD_LEVELS-i-1;
		lod_selection.morph_end[i] = lod_selection.visibility_ranges[index];
		lod_selection.morph_start[i] = prev_pos + (lod_selection.morph_end[i]-prev_pos) * MORPH_START_RATIO;
		prev_pos = lod_selection.morph_start[i];
		snprintf( msg, MAX_LEN_MESSAGES-1, "\tlevel %d, range %f, start %f, end %f",
				i, lod_selection.visibility_ranges[NUMBER_OF_LOD_LEVELS-i-1],
				lod_selection.morph_start[i], lod_selection.morph_end[i] );
		logbook_log( LOG_INFO, msg );
	}
}

inline unsigned int lod_selection_get_selection_count() {
	return lod_selection.selection_count;
}

inline unsigned int lod_selection_get_max_level() {
	return lod_selection.max_selected_lod_level;
}

inline unsigned int lod_selection_get_min_level() {
	return lod_selection.min_selected_lod_level;
}

inline selected_node_t *lod_selection_get_selected_node( const unsigned int i ) {
	return &lod_selection.selected_nodes[i];
}

inline float lod_selection_get_visibility_range( const unsigned int level ) {
	return lod_selection.visibility_ranges[level];
}

inline unsigned int lod_selection_get_stop_at_level() {
	return lod_selection.stop_at_level;
}

inline void lod_selection_set_tile_index( const unsigned int index ) {
	lod_selection.current_tile_index = index;
}

void lod_selection_print() {
	// Debug output:
	char msg[MAX_LEN_MESSAGES];
	sprintf( msg, "Lod selection selected %d nodes:", lod_selection.selection_count );
	logbook_log( LOG_INFO, msg );
	for( unsigned int i = 0; i < lod_selection.selection_count; ++i ) {
		selected_node_t *n = &lod_selection.selected_nodes[i];
		snprintf( msg, MAX_LEN_MESSAGES-1,
				"Node aabb ((%.2f/%.2f/%.2f)/(%.2f/%.2f/%.2f)), tile %d; lvl %d; distance %.2f",
				n->node->aabb.min.x, n->node->aabb.min.y, n->node->aabb.min.z,
				n->node->aabb.max.x, n->node->aabb.max.y, n->node->aabb.max.z,
				n->tile_index, n->lod_level, n->min_distance_to_camera
		);
		logbook_log( LOG_INFO, msg );
	}
}

static inline int lod_selection_compare_closer_first( const void *arg1, const void *arg2 ) {
	const selected_node_t *a = (const selected_node_t *)arg1;
	const selected_node_t *b = (const selected_node_t *)arg2;
	return a->min_distance_to_camera > b->min_distance_to_camera;
}

// @todo sort by tile index and distance
inline void lod_selection_sort() {
	if( !lod_selection.sort_by_distance )
		return;
	qsort( lod_selection.selected_nodes, lod_selection.selection_count,
			sizeof( *lod_selection.selected_nodes ), lod_selection_compare_closer_first );
}

inline void lod_selection_add_node( node_t *node, unsigned int level, bool tl, bool tr, bool bl, bool br ) {
	lod_selection.selected_nodes[lod_selection.selection_count].node = node;
	lod_selection.selected_nodes[lod_selection.selection_count].tile_index = lod_selection.current_tile_index;
	lod_selection.selected_nodes[lod_selection.selection_count].lod_level = level;
	lod_selection.selected_nodes[lod_selection.selection_count].hasTL = tl;
	lod_selection.selected_nodes[lod_selection.selection_count].hasTR = tr;
	lod_selection.selected_nodes[lod_selection.selection_count].hasBL = bl;
	lod_selection.selected_nodes[lod_selection.selection_count].hasBR = br;
	//lod_selection.selected_nodes[lod_selection.selection_count].min_distance_to_camera =
	lod_selection.min_selected_lod_level = lod_selection.min_selected_lod_level < level ?
			lod_selection.min_selected_lod_level : level;
	lod_selection.max_selected_lod_level = lod_selection.max_selected_lod_level > level ?
			lod_selection.max_selected_lod_level : level;
	// Set tile index, min distance and min/max levels for sorting
	if( lod_selection.sort_by_distance )
		lod_selection.selected_nodes[lod_selection.selection_count].min_distance_to_camera =
				sqrt( aabbf_min_distance_from_point_sq( &node->aabb, camera_get_position() ) );
	lod_selection.selection_count++;
}

inline vec4f lod_selection_get_morph_consts( const unsigned int lod_level ) {
	const float start = lod_selection.morph_start[lod_level];
	float end = lod_selection.morph_end[lod_level];
	const float error_fudge = 0.01f;
	end = lerpf( end, start, error_fudge );
	const float d = end - start;
	vec4f v = { start, 1.0f / d, end / d, 1.0f / d };
	return v;
}
