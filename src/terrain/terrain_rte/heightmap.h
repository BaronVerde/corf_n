
#pragma once

#include "settings.h"
#include "glad/glad.h"

/* A 2D heightmap texture for use as displacement, additionally stores normalized
 * height values for lookup. Creates OpenGL texture from given texture file.
 * Heightmap must be quadratic and power of two and 16 bit monochrome.
 * Heightmap x and y are heightmap horizontal coordinates, the height is the z value.
 * While in world space and OpenGL (and gridmesh as well as quad tree and nodes),
 * horizontal ccords are x and z, the height value is y. */

struct heightmap_t {
	char filename[MAX_LEN_FILENAMES];
	// Height/width of texture file in pixels.
	unsigned int extent;
	GLuint texture;
	// Minimum and maximum height values (int int16_t range) of the texture.
	uint16_t min_height_value;
	uint16_t max_height_value;
	// Real world height values, normalized to 0..1 over the range of uint16_t
	GLfloat *height_values_normalized;
};

/* Create a new height map texture from file.
 * For the shader: heightmap texture is bound to texture unit 0, high positions texture to unit 1,
 * low positions to unit 2. */
heightmap_t *heightmap_create( const char *filename, heightmap_t *heightmap );

extern heightmap_t *heightmap_delete( heightmap_t *heightmap );

extern void heightmap_bind( const heightmap_t *const heightmap );

// returns min/max values in the world range of 0.0f..65535.0f
void heightmap_get_min_max_height_area(
		const unsigned int x, const unsigned int z, const unsigned int w, const unsigned int h,
		GLfloat *min, GLfloat *max, const heightmap_t *const heightmap );

// returns the real world height value at coords
extern GLfloat heightmap_get_height_at(
		const unsigned int x, const unsigned int y, const heightmap_t *const heightmap
);
