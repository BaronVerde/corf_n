
/* The camera's view frustum, radar approach in world space
 * Source: http://www.lighthouse3d.com/tutorials/view-frustum-culling/ */

#pragma once

#include <stdlib.h>
#include "vec3.h"
#include "aabb.h"

// @todo: UNDEFINED, OUT_OF_RANGE and SELECTED belong elsewhere
typedef enum {
	OUTSIDE, INTERSECTS, INSIDE, UNDEFINED, OUT_OF_RANGE, SELECTED
} intersect_t;

typedef struct view_frustum_t view_frustum_t;
// Store camera position and reference vectors for rapid use in intersection tests
struct view_frustum_t {
	vec3f camera_position;
	// frustum orientation vectors
	vec3f x;
	vec3f y;
	vec3f z;
	float near_plane;
	float far_plane;
	// frustum width and height depending on view angle. NOT screen width/height !
	float width;
	float height;
	float ratio;
	float angle;
	float tangens_angle;
	// Precomputed angle for sphere intersection test.
	float sphere_factor_y;
	float sphere_factor_x;
};

// Must be called every time the lookAt matrix changes,
// e.g. on zoom-facter or near/far clip plane change. Angle in degrees.
view_frustum_t *frustum_set_fov(
		const float angle, const float ratio,
		const float near_plane, const float far_plane, view_frustum_t *view_frustum );

// Must be called every time camera position or orientation changes, i. e. every frame.
// Takes unnormalized vectors just like the lookAt() matrix.
view_frustum_t *frustum_set_camera_vectors(
		const vec3f* const pos, const vec3f* const front, const vec3f* const up,
		view_frustum_t *view_frustum );

intersect_t frustum_contains_point( const vec3f* const point, const view_frustum_t *const view_frustum );

intersect_t frustum_contains_sphere(
		const vec3f* const center, const float radius, const view_frustum_t *const view_frustum );

intersect_t frustum_contains_box( const aabbf* const box, const view_frustum_t *const view_frustum );

void view_frustum_print( const view_frustum_t *const view_frustum );
