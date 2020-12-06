
#pragma once

#include "glad/glad.h"
#include "omath/vec4.h"
#include <stdbool.h>

bool draw_texture2d_create();

void draw_texture2d_render( const GLuint texture_unit );

void draw_texture2d_delete();
