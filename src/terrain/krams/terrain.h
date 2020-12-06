
#pragma once

#include <stdbool.h>
#include "settings.h"

#define TERRAIN_MAX_TILES 1

typedef enum { requested, loading, ready } tile_status_t;

typedef struct tiles_t {
	terrain_tile_t *tile;
	unsigned int tile_index;
	tile_status_t status;
} tiles_t;

// list_nodes, when true, causes verbose logging put of quadtree built nodes and lod_selection nodes
bool terrain_create( const bool list_nodes );

void terrain_delete();

bool terrain_setup();

void terrain_render( const bool draw_boxes, const bool draw_terrain );

void terrain_cleanup();
