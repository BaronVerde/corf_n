
#pragma once

#include "base/base.h"

#define NUMBER_OF_LOD_LEVELS 5
// @todo what's that for ?
#define NUMBER_OF_GRID_MESHES (NUMBER_OF_LOD_LEVELS+1)
// @todo should depend on node size and lod levels
#define MAX_NUMBER_SELECTED_NODES 1024
// @todo: calc from number of lod levels and heightmap size. Memory usage rises for small nodes.
// Must be power of 2.
#define LEAF_NODE_SIZE 64
// Size of a single terrain tile, quadratic (artificial contsraint), pow 2 and <=16384
// @todo: in a future version this could be handled dynamically
#define TILE_SIZE 2048
/* Determines rendering LOD level distribution based on distance from the viewer.
 * Value of 2.0 should result in equal number of triangles displayed on screen (in
 * average) for all distances. Values above 2.0 will result in less triangles
 * on closer areas, and vice versa. Must be between 1.5 and 16.0 ! */
#define LOD_LEVEL_DISTANCE_RATIO 2.5f
// [0, 1] when to start morphing to the next (lower-detailed) LOD level;
// default is 0.67 - first 0.67 part will not be morphed, and the morph will go from 0.67 to 1.0
#define MORPH_START_RATIO 0.7f
// texel to grid ratio
#define RENDER_GRID_RESULUTION_MULT 1
#define GRIDMESH_DIMENSION (LEAF_NODE_SIZE * RENDER_GRID_RESULUTION_MULT)

#define TERRAIN_PATH "resources/terrain/area_52_06/"
#define SHADER_PATH "src/applications/terrain/"

// heightmap texture is bound to this texture unit, shader expects it
#define HEIGHTMAP_TEXTURE_UNIT 0

typedef struct gridmesh_t gridmesh_t;
typedef struct heightmap_t heightmap_t;
typedef struct node_t node_t;
typedef struct quadtree_t quadtree_t;
typedef struct terrain_tile_t terrain_tile_t;
