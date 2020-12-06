
#include "view_frustum.h"
#include "base/logbook.h"
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

view_frustum_t *frustum_set_fov(
		const float angle, const float ratio, const float near_plane, const float far_plane,
		view_frustum_t *view_frustum ) {
	view_frustum->ratio = ratio;
	view_frustum->near_plane = near_plane;
	view_frustum->far_plane = far_plane;
	view_frustum->angle = radians( angle );
	// compute width and height of the near plane for point intersection test
	view_frustum->tangens_angle = tan( view_frustum->angle * 0.5f );
	view_frustum->height = view_frustum->near_plane * view_frustum->tangens_angle;
	view_frustum->width = view_frustum->height * view_frustum->ratio;
	// compute sphere factors for sphere intersection test
	view_frustum->sphere_factor_y = 1.0f / cos( view_frustum->angle );
	float anglex = atan( view_frustum->tangens_angle * view_frustum->ratio );
	view_frustum->sphere_factor_x = 1.0f / cos( anglex );
	return view_frustum;
}

view_frustum_t *frustum_set_camera_vectors(
		const vec3f* const pos, const vec3f* const front, const vec3f* const up,
		view_frustum_t *view_frustum ) {
	view_frustum->camera_position = *pos;
	vec3f temp = {0.0,0.0,0.0};
	vec3f_normalize( vec3f_sub( front, pos, &temp ), &view_frustum->z );
	vec3f_normalize( vec3f_cross( &view_frustum->z, up, &temp ), &view_frustum->x );
	vec3f_cross( &view_frustum->x, &view_frustum->z, &view_frustum->y );
	return view_frustum;
}

intersect_t frustum_contains_point(
		const vec3f* const point, const view_frustum_t *const view_frustum ) {
	// compute vector from camera position to p
	vec3f temp = {0.0,0.0,0.0};
	vec3f_sub( point, &view_frustum->camera_position, &temp );
	// compute and test the z coordinate
	float pcz = vec3f_dot( &temp, &view_frustum->z );
	if( pcz > view_frustum->far_plane || pcz < view_frustum->near_plane )
		return OUTSIDE;
	// compute and test the view_frustum.y coordinate
	float pcy = vec3f_dot( &temp, &view_frustum->y );
	float aux = pcz * view_frustum->tangens_angle;
	if( pcy > aux || pcy < -aux )
		return OUTSIDE;
	// compute and test the view_frustum.x coordinate
	float pcx = vec3f_dot( &temp, &view_frustum->x );
	aux *= view_frustum->ratio;
	if( pcx > aux || pcx < -aux )
		return OUTSIDE;
	return INSIDE;
}

intersect_t frustum_contains_sphere(
		const vec3f* const center, const float radius, const view_frustum_t *const view_frustum ) {
	intersect_t result = INSIDE;
	vec3f temp = {0.0,0.0,0.0};
	vec3f_sub( center, &view_frustum->camera_position, &temp );
	float az = vec3f_dot( &temp, &view_frustum->z );
	// Early out against near and far plane
	if( az > view_frustum->far_plane + radius || az < view_frustum->near_plane - radius )
		return OUTSIDE;
	if( az > view_frustum->far_plane - radius || az < view_frustum->near_plane + radius )
		result = INTERSECTS;
	float ay = vec3f_dot( &temp, &view_frustum->y );
	float d = view_frustum->sphere_factor_y * radius;
	az *= view_frustum->tangens_angle;
	if( ay > az + d || ay < -az - d )
		return OUTSIDE;
	if( ay > az - d || ay < -az + d )
		result = INTERSECTS;
	float ax = vec3f_dot( &temp, &view_frustum->x );
	az *= view_frustum->ratio;
	d = view_frustum->sphere_factor_x * radius;
	if( ax > az + d || ax < -az - d )
		return OUTSIDE;
	if( ax > az-d || ax < -az+d )
		result = INTERSECTS;
	return result;
}

intersect_t frustum_contains_box( const aabbf* const box, const view_frustum_t *const view_frustum ) {
	// @todo I am so lazy ...
	vec3f center = {0.0f,0.0f,0.0f};
	return frustum_contains_sphere(
			aabbf_get_center( box, &center ), aabbf_get_diagonal_size( box ) * 0.5f, view_frustum
	);
}

void view_frustum_print( const view_frustum_t *const view_frustum ) {
	char msg[MAX_LEN_MESSAGES];
	sprintf( msg, "View frustum:\ncamera position: (%.2lf/%.2lf/%.2lf)",
			view_frustum->camera_position.x, view_frustum->camera_position.y, view_frustum->camera_position.z );
	logbook_log( LOG_INFO, msg );
	sprintf( msg, "view frustum x: (%.2lf/%.2lf/%.2lf)",
			view_frustum->x.x, view_frustum->x.y, view_frustum->x.z );
	logbook_log( LOG_INFO, msg );
	sprintf( msg, "view frustum y: (%.2lf/%.2lf/%.2lf)",
			view_frustum->y.x, view_frustum->y.y, view_frustum->y.z );
	logbook_log( LOG_INFO, msg );
	sprintf( msg, "view frustum x: (%.2lf/%.2lf/%.2lf)",
			view_frustum->z.x, view_frustum->z.y, view_frustum->z.z );
	logbook_log( LOG_INFO, msg );
	sprintf( msg, "Near %.2lf, far %.2lf, width %.2lf, height %.2lf, ratio %.2lf",
			view_frustum->near_plane, view_frustum->far_plane, view_frustum->width, view_frustum->height,
			view_frustum->ratio );
	logbook_log( LOG_INFO, msg );
	sprintf( msg, "Angle %.2lf, tan angle %.2lf, sphere factors %.2lf/%.2lf",
			view_frustum->angle, view_frustum->tangens_angle,
			view_frustum->sphere_factor_y, view_frustum->sphere_factor_x );
	logbook_log( LOG_INFO, msg );
}

/*
 * box/frustum intersection (fb frustum; ob box)
 * if (fb_xmin > ob_xmax || fb_xmax < ob_xmin || fb_ymin > ob_ymax || fb_ymax < ob_ymin ||
 * 		fb_zmin > ob_zmax || fb_zmax < ob_zmin)
	return (OUTSIDE);

else if (fb_xmin < ob_xmin && fb_xmax > ob_xmax &&
		 fb_ymin < ob_ymin && fb_ymax > ob_ymax &&
		 fb_zmin < ob_zmin && fb_zmax > ob_zmax)
	return (INSIDE);
else
	return(INTERSECT);
 */
