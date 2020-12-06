
/* An axis aligned bounding box. Rather memory intensive.
 * @todo Eventually store 2 corners only and return the array only on demand.
 * make generic ! */

#pragma once

#include "vec3.h"

// member vars
typedef struct {
	vec3f min;
	vec3f max;
} aabbf;

typedef struct {
	vec3d min;
	vec3d max;
} aabbd;

extern vec3f* aabbf_get_center( const aabbf* const aabb, vec3f *out );
extern vec3d* aabbd_get_center( const aabbd* const aabb, vec3d *out );

extern vec3f* aabbf_get_size( const aabbf* const aabb, vec3f *out );
extern vec3d* aabbd_get_size( const aabbd* const aabb, vec3d *out );

extern float aabbf_get_diagonal_size( const aabbf* const aabb );
extern double aabbd_get_diagonal_size( const aabbd* const aabb );

extern bool aabb_intersect_other( const aabbf* const one, const aabbf* const other );

extern float aabbf_min_distance_from_point_sq( const aabbf* const aabb, const vec3f* const point );
extern double aabbd_min_distance_from_point_sq( const aabbd* const aabb, const vec3d* const point );

extern float aabb_max_distance_from_point_sq( const aabbf* const aabb, const vec3f* const point );

extern bool aabbf_intersect_sphere_sq( const aabbf* const aabb, const vec3f* const center, float radius_sq );
extern bool aabbd_intersect_sphere_sq( const aabbd* const aabb, const vec3d* const center, double radius_sq );

extern bool aabb_is_inside_sphere_sq( const aabbf* const aabb, const vec3f* const center, float radius_sq );

// @todo needs overwork to work with vectors
extern bool aabb_intersect_ray(
		const aabbf* const aabb, const vec3f* const ray_origin, const vec3f* ray_direction, float *out_direction );

// Returns AABB that encloses the one and the other one
extern aabbf* aabb_enclose_other( const aabbf* const one, const aabbf* const other, aabbf* out_aabb );

extern bool aabb_equals_other( const aabbf* const one, const aabbf* const other );

extern float aabbf_get_bounding_sphere_radius( const aabbf* const aabb );
extern double aabbd_get_bounding_sphere_radius( const aabbd* const aabb );

extern vec3f *aabbf_get_vertex_positive( const aabbf* const aabb, const vec3f* const normal, vec3f* out_positive );
extern vec3d *aabbd_get_vertex_positive( const aabbd* const aabb, const vec3d* const normal, vec3d* out_positive );

extern vec3f *aabbf_get_vertex_negative( const aabbf* const aabb, const vec3f* const normal, vec3f* out_negative );
extern vec3d *aabbd_get_vertex_negative( const aabbd* const aabb, const vec3d* const normal, vec3d* out_negative );

extern aabbf* aabb_expand( const aabbf* const aabb, const float percentage, aabbf* out_expanded );
