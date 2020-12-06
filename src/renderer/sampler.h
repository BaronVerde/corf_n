
#pragma once

#include "glad/glad.h"

typedef enum {
	NEAREST_CLAMP,
	LINEAR_CLAMP,
	NEAREST_REPEAT,
	LINEAR_REPEAT,
	LINEAR_MIPMAP_CLAMP
} sampler_type_t;

extern void set_default_sampler( GLuint texture, sampler_type_t type );
