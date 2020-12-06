
// Debug immediate drawing of boxes

#pragma once

#include "omath/aabb.h"
#include "omath/vec4.h"
#include "glad/glad.h"
#include <stdbool.h>
#include "color.h"

bool draw_aabb_create();

void draw_aabb( const aabbf *const bb, const color_t *const color );

extern void draw_aabb_delete();

extern GLuint draw_abb_get_program();
