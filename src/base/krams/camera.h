
// One camera for now, and inline functions to access it

#pragma once

#include "omath/mat4.h"
#include "omath/vec3.h"
#include "omath/view_frustum.h"

// @todo Terrain will be a first person camera with double precision view matrix
typedef enum {
	ORBITING = 0, FIRST_PERSON, TERRAIN
} camera_mode_t;

typedef enum {
	// Movement of first person camera
	FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN, ROTATE_LEFT, ROTATE_RIGHT,
	// Faster ...
	FAST_FORWARD, FAST_BACKWARD, FAST_RIGHT, FAST_LEFT, FAST_UP, FAST_DOWN,
	// Movement of orbiting camera
	CLOSE, RETREAT, FAST_CLOSE, FAST_RETREAT
} direction_t;

extern void camera_create( const vec3f *const camera_position, const vec3f *const camera_target );

extern bool camera_mouse_move( double x_pos, double y_pos );

extern void camera_set_position_and_target( const vec3f *const pos, const vec3f *const target );

extern void camera_update_moving( const float deltatime );

extern bool camera_key_event( int key, int action, int mods );

extern void camera_print_position();

extern float camera_get_near_plane();

extern float camera_get_far_plane();

extern void camera_set_near_far_plane( const float near, const float far );

extern void camera_set_movement_speed( const float speed );

extern mat4f *camera_get_view_projection_matrix();

extern vec3f *camera_get_position();

extern view_frustum_t *camera_get_view_frustum();
