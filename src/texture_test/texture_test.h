
#pragma once

#include "renderer/draw_texture2d.h"
#include "renderer/sampler.h"
#include "stb/stb_image.h"
#include "base/logbook.h"
#include <stdbool.h>

static GLuint image;

void texture_test_delete() {
	if( glIsTexture(image) )
		glDeleteTextures( 1, &image );
}

bool texture_test_create() {
	if( !draw_texture2d_create() )
		return false;
	glCreateTextures( GL_TEXTURE_2D, 1, &image );
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load( "resources/images/green_baron.png", &width, &height, &nrChannels, 0 );
	if( data && nrChannels == 3 ) {
		glTextureStorage2D( image, 1, GL_RGB8, width, height );
		glTextureSubImage2D(
				image, 0,				// texture and mip level
				0, 0, width, height,	// offset and size
				GL_RGB, GL_UNSIGNED_BYTE, data
		);
		set_default_sampler( image, LINEAR_CLAMP );
		//glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		logbook_log( LOG_ERROR, "Texture could not be loaded" );
		texture_test_delete();
		return false;
	}
	if(data)
		stbi_image_free(data);
	return true;
}

void texture_test_render() {
	draw_texture2d_render( image );
}
