
/* Selects visible nodes and lod levels from a quad tree based on lod settings
 * and camera frustum. */

#pragma once

#include "omath/vec4.h"
#include "node.h"
#include <stdbool.h>

typedef struct selected_node_t {
	node_t *node;
	unsigned int lod_level;
	bool hasTL;
	bool hasTR;
	bool hasBL;
	bool hasBR;
	// determines drawing sequence to avoid shader updates
	unsigned int tile_index;
	// @todo sorting temporarily disabled, sort by tile index and distance
	float min_distance_to_camera;
} selected_node_t;

extern void lod_selection_create( bool sort_by_distance );

extern void lod_selection_add_node( node_t *node, unsigned int level, bool tl, bool tr, bool bl, bool br );

// Called when camera near or far plane changed to recalc visibility and morph ranges.
void lod_selection_calculate_ranges();

extern unsigned int lod_selection_get_selection_count();

extern selected_node_t *lod_selection_get_selected_node( const unsigned int i );

extern unsigned int lod_selection_get_max_level();

extern unsigned int lod_selection_get_min_level();

extern float lod_selection_get_visibility_range( const unsigned int level );

extern vec4f lod_selection_get_morph_consts( const unsigned int lod_level );

extern unsigned int lod_selection_get_stop_at_level();

extern void lod_selection_sort();

extern void lod_selection_reset();

void lod_selection_print();

extern void lod_selection_set_tile_index( const unsigned int index );
