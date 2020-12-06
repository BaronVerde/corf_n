
#include "heightmap.h"
#include "terrain_tile.h"
#include "base/camera.h"
#include "base/logbook.h"
#include "lod_selection.h"
#include "node.h"

void node_create(
		const unsigned int x, const unsigned int z, const unsigned int size, const unsigned int level,
		const terrain_tile_t *const tile, node_t *all_nodes, unsigned  int *last_index, node_t *node ) {
	node->x = x;
	node->z = z;
	node->level = level;
	node->size = size;
	node->subBL = NULL;
	node->subTL = NULL;
	node->subBR = NULL;
	node->subTR = NULL;
	const heightmap_t *heightmap = tile->heightmap;
	// Find min/max heights at this patch of terrain
	const unsigned int limit_x = heightmap->extent <= x+size+1 ? heightmap->extent : x+size+1;
	// z = y-axis of heightmap
	const unsigned int limit_z = heightmap->extent <= z+size+1 ? heightmap->extent : z+size+1;
	heightmap_get_min_max_height_area(
			x, z, limit_x-x, limit_z-z, &node->min_height, &node->max_height, tile->heightmap
	);
	// Get bounding box in world coords @todo: the box is relative to heightmap for now
	// also @todo: real height values
	float temp_height_factor = 655.35f * 2.0f;
	node->aabb.min.x = tile->aabb.min.x+(float)x;
	node->aabb.min.y = node->min_height * temp_height_factor;
	node->aabb.min.z = tile->aabb.min.z+(float)z;
	node->aabb.max.x = tile->aabb.min.x+(float)(x+size);
	node->aabb.max.y = node->max_height * temp_height_factor;
	node->aabb.max.z = tile->aabb.min.z+(float)(z+size);
	// Highest level reached already ?
	if( size == LEAF_NODE_SIZE ) {
		if( level != NUMBER_OF_LOD_LEVELS-1 ) {
			logbook_log( LOG_ERROR, "Lowest lod level != number lod levels while creating nodes. Good luck rendering" );
			return;
		}
		// Mark leaf node
		node->is_leaf = true;
	} else {
		unsigned int sub_size = size/2;
		node->subTL = &all_nodes[(*last_index)++];
		node_create( x, z, sub_size, level+1, tile, all_nodes, last_index, node->subTL );
		if( ( x+sub_size ) < heightmap->extent) {
			node->subTR = &all_nodes[(*last_index)++];
			node_create( x+sub_size, z, sub_size, level+1, tile, all_nodes, last_index, node->subTR );
		}
		if( (z+sub_size) < heightmap->extent ) {
			node->subBL = &all_nodes[(*last_index)++];
			node_create( x, z+sub_size, sub_size, level+1, tile, all_nodes, last_index, node->subBL );
		}
		if( ((x+sub_size) < heightmap->extent) && ((z+sub_size) < heightmap->extent) ) {
			node->subBR = &all_nodes[(*last_index)++];
			node_create( x+sub_size, z+sub_size, sub_size, level+1, tile, all_nodes, last_index, node->subBR );
		}
	}
}

intersect_t node_lod_select( node_t *node, bool parent_completely_in_frustum ) {
	// Test early outs
	intersect_t frustum_intersection = parent_completely_in_frustum ?
			INSIDE : frustum_contains_box( &node->aabb, camera_get_view_frustum() );
	if( OUTSIDE == frustum_intersection )
		return OUTSIDE;
	float dist_limit = lod_selection_get_visibility_range(node->level);
	if( !aabb_intersect_sphere_sq( &node->aabb, camera_get_position(), dist_limit * dist_limit ) )
		return OUT_OF_RANGE;
	intersect_t sub_tl_res = UNDEFINED;
	intersect_t sub_tr_res = UNDEFINED;
	intersect_t sub_bl_res = UNDEFINED;
	intersect_t sub_br_res = UNDEFINED;
	// Stop at one below number of lod levels
	if( node->level != lod_selection_get_stop_at_level() ) {
		float next_dist_limit = lod_selection_get_visibility_range(node->level+1);
		if( aabb_intersect_sphere_sq( &node->aabb, camera_get_position(), next_dist_limit * next_dist_limit ) ) {
			bool we_are_completely_in_frustum = frustum_intersection == INSIDE;
			if( node->subTL != NULL )
				sub_tl_res = node_lod_select( node->subTL, we_are_completely_in_frustum );
			if( node->subTR != NULL )
				sub_tr_res = node_lod_select( node->subTR, we_are_completely_in_frustum );
			if( node->subBL != NULL )
				sub_bl_res = node_lod_select( node->subBL, we_are_completely_in_frustum );
			if( node->subBR != NULL )
				sub_br_res = node_lod_select( node->subBR, we_are_completely_in_frustum );
		}
	}

	// We don't want to select sub nodes that are invisible (out of frustum) or are selected;
	// (we DO want to select if they are out of range, since we are not)
	bool remove_tl = (sub_tl_res == OUTSIDE) || (sub_tl_res == SELECTED);
	bool remove_tr = (sub_tr_res == OUTSIDE) || (sub_tr_res == SELECTED);
	bool remove_bl = (sub_bl_res == OUTSIDE) || (sub_bl_res == SELECTED);
	bool remove_br = (sub_br_res == OUTSIDE) || (sub_br_res == SELECTED);

	if( lod_selection_get_selection_count() >= MAX_NUMBER_SELECTED_NODES ) {
		logbook_log( LOG_WARNING, "Maximum selection count exceeded by lod. Some nodes will not be drawn" );
		return OUTSIDE;
	}
	// Add node to selection
	if( !( remove_tl && remove_tr && remove_bl && remove_br ) &&
		 ( lod_selection_get_selection_count() < MAX_NUMBER_SELECTED_NODES ) ) {
		unsigned int lod_level = lod_selection_get_stop_at_level() - node->level;
		// mind current tile index and node pointer
		lod_selection_add_node( node, lod_level, !remove_tl, !remove_tr, !remove_bl, !remove_br );
		return SELECTED;
	}
	// if any of child nodes are selected, then return selected -
	// otherwise all of them are out of frustum, so we're out of frustum too
	if( (sub_tl_res == SELECTED) || (sub_tr_res == SELECTED) ||
		(sub_bl_res == SELECTED) || (sub_br_res == SELECTED) )
		return SELECTED;
	else
		return OUTSIDE;
}

/* 	    // Find heights for 4 corner points (used for approx ray casting)
	    // (reuse otherwise empty pointers used for sub nodes)
	    float * pTLZ = (float *)&subTL;
	    float * pTRZ = (float *)&subTR;
	    float * pBLZ = (float *)&subBL;
	    float * pBRZ = (float *)&subBR;
	    int limitX = ::min( rasterSizeX - 1, x + size );
	    int limitY = ::min( rasterSizeY - 1, y + size );
	    *pTLZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( x, y ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pTRZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( limitX, y ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pBLZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( x, limitY ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pBRZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( limitX, limitY ) * createDesc.MapDims.SizeZ / 65535.0f;
 */

