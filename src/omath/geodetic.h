
#pragma once

#include <stdbool.h>

// lat lon in degrees.decimals, height in m
typedef struct geodetic_t {
	double lon;
	double lat;
	double height;
} geodetic_t;

// southwest and northeast corners in degrees.decimals
typedef struct geodetic_extent_t {
	double sw_lat;
	double sw_lon;
	double ne_lat;
	double ne_lon;
} geodetic_extent_t;

extern bool geodetic_equals( const geodetic_t *const a, const geodetic_t *const b );

extern geodetic_extent_t *geodetic_extent_create(
		const geodetic_t *const south_west, const geodetic_t *const north_east, geodetic_extent_t *geodetic );

extern bool geodetic_extent_equals(
		const geodetic_extent_t *const south_west, const geodetic_extent_t *const north_east );
