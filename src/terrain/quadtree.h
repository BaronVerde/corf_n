
#pragma once

#include "settings.h"
#include <stdbool.h>

struct quadtree_t {
	unsigned short top_node_size;
	unsigned int top_node_count;
	unsigned int node_count;
	node_t *all_nodes;
	node_t ***top_level_nodes;
	terrain_tile_t *terrain_tile;
};

// list_nodes, when true, caues a list of nodes and their bounding boxes to be printed to logbook
quadtree_t *quadtree_create( terrain_tile_t *tile, const bool list_nodes, quadtree_t *quadtree );

extern quadtree_t *quadtree_delete( quadtree_t *quadtree );

// tile index is saved in selection list for sorting by tile and distance
void quadtree_lod_select( const quadtree_t *const quadtree );
