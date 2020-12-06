
/* A class for a triaxial allipsoid. Comes with conversion routines for cartesian
 * (e.g. WGS84) to geodetic (lat,lon,height) coordinates. */

// @todo normals don't need double precision (not even single)
// cen = centric, geo = geodetic, car = cartesian, _n = normal

/* Example ellipsoids:
 * WGS84_ELLIPSOID{ 6378137.0, 6378137.0, 6356752.314245 };
 * WGS84_ELLIPSOID_SMALL{ 6378.137, 6378.137, 6356.752314245 };
 * WGS84_ELLIPSOID_SCALED{ 1.0, 1.0, 6378137.0 / 6356752.314245 };
 * UNIT_SPHERE{ 1.0, 1.0, 1.0 };
 * ONE_TO_FIVE{ 1.0, 0.2, 0.2 };	// 'Oumuamua
 * ONE_TO_TWO{ 1.0, 0.5f, 0.5 };
 * ONE_TO_THREE{ 1.0, 0.33, 0.33 }; */

#pragma once

#include "geodetic.h"
#include "vec3.h"
#include "vec2.h"

typedef struct ellipsoid_t {
	vec3d position;
	vec3d radii;
	vec3d radii_squared;
	vec3d radii_to_the_fourth;
	vec3d one_over_radii_squared;
} ellipsoid_t;

// all radii > 0.0 !
extern ellipsoid_t *ellipsoid_create( const double x, const double y, const double z, ellipsoid_t *e );

extern ellipsoid_t *ellipsoid_create_from_vec( const vec3d *const radii, ellipsoid_t *el );

// geodetic surface normal from geodetic position
extern vec3d *ellipsoid_geodetic_normal_from_geodetic( const geodetic_t *const geo, vec3d *n );

// conversion from geodetic to cartesian position.
extern vec3d *ellipsoid_to_cartesian(
		const geodetic_t *const geodetic, const ellipsoid_t *const e, vec3d *cartesian
);

// geocentric surface normal from cartesian position on ellipsoid; origin is center
extern vec3d *CentricSurfaceNormal( const vec3d *const pos, vec3d *n );

// geodetic surface normal from cartesian position on ellipsoid; origin is center
extern vec3d *GeodeticSurfaceNormal( const vec3d *const pos, const ellipsoid_t *const e, vec3d *n );

extern double MinimumRadius( const ellipsoid_t *const e );

extern double MaximumRadius( const ellipsoid_t *const e );

// returns number of intersections with e (0, 1, or 2) from origin in direction
// sets intersection time from origin in param intersections, closest first
unsigned int Intersections(
		const vec3d *const origin, const vec3d *const direction,
		const ellipsoid_t *const e, double *intersections[2] );

// Cartesian to geodetic conversion
geodetic_t *ToGeodetic3D( const vec3d *const position, const ellipsoid_t *const e, geodetic_t *geo );

/* Determine surface point of a cartesian coordinate along its geodetic normal. Iterative, should
 * converge quickly (1-4 iterations). Could take several iterations for mor oblate ellipsoids.
 * Assumes the ellipsoid is centered at the origin. */
extern vec3d *ScaleToGeodeticSurface( const vec3d *const position, const ellipsoid_t *const e, vec3d *scaled_pos );

/* Determine surface point of a cartesian coordinate along its geocentric normal.
 * Assumes the ellipsoid is centered at the origin. */
extern vec3d *ScaleToGeocentricSurface( const vec3d *const position, const ellipsoid_t *const e, vec3d *scaled );

// Granularity ? @todo: Wip ! test the implementation.
//std::vector<dvec3> compute_curve( const vec3d *const start, const vec3d *const stop, const double granularity );

/* Converts texture coordinates
 * Takes a geodetic surface normal normalized to [-1, 1] and computes
 * s (horizontal) and t (vertical) coordinate as [0, 1]  */
vec2d *ellipsoid_compute_tex_coord( const vec3d *const normal, const ellipsoid_t *const e, vec2d *tc );
