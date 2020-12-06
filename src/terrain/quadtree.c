
#include "node.h"
#include "quadtree.h"
#include "heightmap.h"
#include "terrain_tile.h"
#include "base/logbook.h"
#include <stdlib.h>
#include <stdio.h>

quadtree_t *quadtree_create( terrain_tile_t *tile, const bool list_nodes, quadtree_t *quadtree ) {
	if( quadtree ) {
		logbook_log( LOG_WARNING, "Non null pointer passed to quadtree_create" );
		return quadtree;
	}
	quadtree = malloc(sizeof(quadtree_t));
	if( !quadtree ) {
		logbook_log( LOG_ERROR, "Error allocating quadtree memory" );
		return NULL;
	}
	quadtree->terrain_tile = tile;
	// shortcut
	const unsigned int raster_size = quadtree->terrain_tile->heightmap->extent;
	// Determine how many nodes will we use, and the size of the top (root) tree node.
	unsigned int total_node_count = 0;
	quadtree->top_node_size = LEAF_NODE_SIZE;
	for( int i = 0; i < NUMBER_OF_LOD_LEVELS; i++ ) {
		if( i != 0 )
			quadtree->top_node_size *= 2;
		const unsigned int node_count = (raster_size-1) / quadtree->top_node_size + 1;
		total_node_count += node_count * node_count;
	}
	// Initialize the tree memory, create tree nodes, and extract min/max Ys (heights)
	quadtree->all_nodes = malloc(total_node_count*sizeof(node_t));
	if( !quadtree->all_nodes ) {
		logbook_log( LOG_ERROR, "Error allocating node memory in quadtree_create" );
		return quadtree_delete(quadtree);
	}
	unsigned int node_counter = 0;
	quadtree->top_node_count = (raster_size-1) / quadtree->top_node_size + 1;
	quadtree->top_level_nodes = malloc(quadtree->top_node_count*sizeof(node_t*));
	if( !quadtree->top_level_nodes ) {
		logbook_log( LOG_ERROR, "Error allocating quadtree memory" );
		return quadtree_delete(quadtree);
	}
	for( unsigned int z = 0; z < quadtree->top_node_count; ++z ) {
		quadtree->top_level_nodes[z] = malloc(quadtree->top_node_count*sizeof(node_t*));
		if( !quadtree->top_level_nodes ) {
			logbook_log( LOG_ERROR, "Error allocating quadtree memory" );
			// @todo this is going to crash if memory runs out in mid looping. Try malloc beforehand
			return quadtree_delete(quadtree);
		}
		for( unsigned int x = 0; x < quadtree->top_node_count; ++x ) {
			quadtree->top_level_nodes[z][x] = &quadtree->all_nodes[node_counter];
			++node_counter;
			node_create(
					x*quadtree->top_node_size, z*quadtree->top_node_size, quadtree->top_node_size, 0,
					quadtree->terrain_tile, quadtree->all_nodes, &node_counter, quadtree->top_level_nodes[z][x]
			);
		}
	}
	quadtree->node_count = node_counter;
	char msg[MAX_LEN_MESSAGES];
	if( quadtree->node_count != total_node_count ) {
		snprintf( msg, MAX_LEN_MESSAGES-1,
				"Quadtree not built. Node counter (%d) does not equal pre-calculated node count (%d)",
				quadtree->node_count, total_node_count );
		logbook_log( LOG_ERROR, msg );
		return quadtree_delete(quadtree);
	}

	// Debug output - summary and list of nodes
	const float size_in_memory = (float)(quadtree->node_count*(sizeof(node_t)+sizeof(aabbf))+sizeof(quadtree_t));
	snprintf( msg, MAX_LEN_MESSAGES-1,
			"Quadtree created. %d nodes, size in memory %.2fkb, %d*%d top level nodes",
			quadtree->node_count, size_in_memory/1024.0f, quadtree->top_node_count, quadtree->top_node_count );
	logbook_log( LOG_INFO, msg );
	// Debug: List of all Nodes
	if( list_nodes ) {
		for( unsigned int i = 0; i < quadtree->node_count; ++i ) {
			const node_t *n = &quadtree->all_nodes[i];
			if( !n->is_leaf )
				snprintf( msg, MAX_LEN_MESSAGES-1,
						"Node %d, level %d, aabb (%.2f/%.2f/%.2f)/(%.2f/%.2f/%.2f), leaves (%d/%d/%d/%d)", i,
						n->level, n->aabb.min.x, n->aabb.min.y, n->aabb.min.z, n->aabb.max.x, n->aabb.max.y,
						n->aabb.max.z, n->subTL->level, n->subTR->level, n->subBL->level, n->subBR->level );
			else
				snprintf( msg, MAX_LEN_MESSAGES-1,
						"Node %d, level %d, aabb (%.2f/%.2f/%.2f)/(%.2f/%.2f/%.2f), is leaf", i, n->level,
						n->aabb.min.x, n->aabb.min.y, n->aabb.min.z, n->aabb.max.x, n->aabb.max.y, n->aabb.max.z );
			logbook_log( LOG_INFO, msg );
		}
	}
	return quadtree;
}

inline quadtree_t *quadtree_delete( quadtree_t *quadtree ) {
	if( quadtree ) {
		if( quadtree->all_nodes )
			free(quadtree->all_nodes);
		if( quadtree->top_level_nodes ) {
			for( unsigned int y = 0; y < quadtree->top_node_count; ++y )
				free(quadtree->top_level_nodes[y]);
			free(quadtree->top_level_nodes);
		}
		free(quadtree); quadtree = NULL;
	}
	return quadtree;
}

void quadtree_lod_select( const quadtree_t *const quadtree ) {
	for( unsigned int z = 0; z < quadtree->top_node_count; ++z )
		for( unsigned int x = 0; x < quadtree->top_node_count; ++x )
			node_lod_select( quadtree->top_level_nodes[z][x], false );
}
