
#pragma once

#include "settings.h"
#include "glad/glad.h"
#include <inttypes.h>

struct heightmap_t {
	char filename[MAX_LEN_FILENAMES];
	// Height/width of texture file in pixels. Texture of a tile is square.
	unsigned int extent;
	GLuint texture;
	uint16_t min_height_value;
	uint16_t max_height_value;
	uint16_t *height_values;
};

heightmap_t *heightmap_create( const char *filename, heightmap_t *heightmap );

extern heightmap_t *heightmap_delete( heightmap_t *heightmap );

extern void heightmap_bind( const heightmap_t *const heightmap );

// returns min/max values in the world range of 0.0f..65535.0f
void heightmap_get_min_max_height_area(
		const unsigned int x, const unsigned int z, const unsigned int w, const unsigned int h,
		uint16_t *min, uint16_t *max, const heightmap_t *const heightmap );

// returns the real world height value at coords
extern uint16_t heightmap_get_height_at(
		const unsigned int x, const unsigned int y, const heightmap_t *const heightmap
);
