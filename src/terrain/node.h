
#pragma once

#include "settings.h"
#include "omath/aabb.h"
#include "omath/view_frustum.h"
#include <inttypes.h>

struct node_t {
	unsigned int x;
	unsigned int z;
	unsigned short size;
	bool is_leaf;
	/* Level 0 is a root node, and level 'lod_level-1' is a leaf node. So the actual
	 * LOD level equals 'lod_level_count - 1 - node.level' */
	unsigned short level;
	uint16_t min_height;
	uint16_t max_height;
	node_t *subTL;
	node_t *subTR;
	node_t *subBL;
	node_t *subBR;
	aabbf aabb;
};

// worldPositionCellsize: .x = lower left latitude, .y = longitude, .z = cellsize
void node_create(
		const unsigned int x, const unsigned int z, const unsigned short size, const unsigned short level,
		const terrain_tile_t *const tile, node_t *all_nodes, unsigned int *last_index, node_t *node
);

intersect_t node_lod_select( node_t *node, bool parent_completely_in_frustum );
