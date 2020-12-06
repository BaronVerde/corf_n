
#include "heightmap.h"
#include "base/logbook.h"
#include "stb/stb_image.h"
#include "renderer/sampler.h"
#include "omath/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

heightmap_t *heightmap_create( const char *filename, heightmap_t *heightmap ) {
	char msg[MAX_LEN_MESSAGES];
	if( strlen(filename) >= MAX_LEN_FILENAMES-1 ) {
		snprintf( msg, MAX_LEN_MESSAGES-1, "Filename too long for heightmap texture '%s'", filename );
		logbook_log( LOG_ERROR, msg );
		return heightmap;
	}
	if( heightmap ) {
		logbook_log( LOG_WARNING, "Non-null pointer passed to heightmap create" );
		return heightmap;
	}
	heightmap = malloc(sizeof(heightmap_t));
	if( !heightmap ) {
		logbook_log( LOG_ERROR, "Error allocating heightmap" );
		return heightmap;
	}
	// stbi_set_flip_vertically_on_load( true );
	int w, h, channels;
	// load the data, single channel 16bit
	heightmap->height_values = stbi_load_16( filename, &w, &h, &channels, 1 );
	if( !heightmap->height_values ) {
		snprintf( msg, MAX_LEN_MESSAGES-1, "Could not read heightmap texture '%s'", filename );
		logbook_log( LOG_ERROR, msg );
		return heightmap_delete(heightmap);
	}
	if( channels != 1 ) {
		snprintf( msg, MAX_LEN_MESSAGES-1, "Error reading heightmap texture '%s'. Not monochrome", filename );
		logbook_log( LOG_ERROR, msg );
		return heightmap_delete(heightmap);
	}
	strncpy( heightmap->filename, filename, MAX_LEN_FILENAMES-1 );
	heightmap->extent = (unsigned int)w;
	size_t num_pixels = (size_t)(w * h);
	// Unclamped values are allways stored as 16 bit integers
	GLfloat *valuesf = malloc(num_pixels*sizeof(GLfloat));
	if( !valuesf ) {
		snprintf(
				msg, MAX_LEN_MESSAGES-1,
				"Error allocating mem to store height values of heightmap texture '%s'", filename
		);
		logbook_log( LOG_ERROR, msg );
		return heightmap_delete(heightmap);
	}
	// find min/max values and assign normalized values
	heightmap->min_height_value = UINT16_MAX;
	heightmap->max_height_value = 0;
	for( unsigned int x = 0; x < heightmap->extent; ++x ) {
		for( unsigned int y = 0; y < heightmap->extent; ++y ) {
			uint16_t t = heightmap->height_values[x+heightmap->extent*y];
			heightmap->min_height_value = t < heightmap->min_height_value ? t : heightmap->min_height_value;
			heightmap->max_height_value = t > heightmap->max_height_value ? t : heightmap->max_height_value;
			valuesf[x+heightmap->extent*y] = (GLfloat)t / 65535.0f;
		}
	}
	// There's only float data 0..1 from now on
	glCreateTextures( GL_TEXTURE_2D, 1, &heightmap->texture );
	// no mip levels @todo 16bit floats, compression ?
	glTextureStorage2D(
			heightmap->texture, 1, GL_R32F, (GLsizei)heightmap->extent, (GLsizei)heightmap->extent
	);
	glTextureSubImage2D( heightmap->texture, 0,								// texture and mip level
			0, 0, (GLsizei)heightmap->extent, (GLsizei)heightmap->extent,	// offset and size
			GL_RED, GL_FLOAT, valuesf );
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, heightmap->texture );
	// set the default sampler for the heightmap texture
	set_default_sampler( heightmap->texture, LINEAR_CLAMP );
	// release mem
	free(valuesf);
	// @todo: query texture size ! sizeof(heightmap->height_values_normalized)
	float total_size = (float)( sizeof(heightmap_t) + num_pixels * sizeof(uint16_t) ) / 1024.0f;
	snprintf(
			msg, MAX_LEN_MESSAGES-1, "Heightmap '%s', texture unit %d, %d * %d, loaded. Size in memory %.2fkb",
			filename, HEIGHTMAP_TEXTURE_UNIT, heightmap->extent, heightmap->extent, total_size
	);
	logbook_log( LOG_INFO, msg );
	return heightmap;
}

inline uint16_t heightmap_get_height_at(
		const unsigned int x, const unsigned int y, const heightmap_t *const heightmap ) {
	return heightmap->height_values[x + y*heightmap->extent];
}

void heightmap_get_min_max_height_area(
		const unsigned int x, const unsigned int z, const unsigned int w, const unsigned int h,
		uint16_t *min, uint16_t *max, const heightmap_t *const heightmap ) {
	*min = UINT16_MAX;
	*max = 0;
	for( unsigned int i = x; i < x + w; ++i )
	    for( unsigned int j = z; j < z + h; ++j ) {
	    	const uint16_t new_val = heightmap_get_height_at( i, j, heightmap );
	    	*min = new_val < *min ? new_val : *min;
	    	*max = new_val > *max ? new_val : *max;
	    }
}

inline heightmap_t *heightmap_delete( heightmap_t *heightmap ) {
	if( heightmap ) {
		if( glIsTexture(heightmap->texture) )
			glDeleteTextures( 1, &heightmap->texture );
		if( heightmap->height_values )
			stbi_image_free(heightmap->height_values);
		char msg[MAX_LEN_MESSAGES];
		sprintf( msg, "Heightmap '%s' destroyed", heightmap->filename );
		logbook_log( LOG_INFO, msg );
		free(heightmap);
		heightmap = NULL;
	}
	return heightmap;
}

inline void heightmap_bind( const heightmap_t *const heightmap ) {
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, heightmap->texture );
}
