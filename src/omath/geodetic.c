
#include "common.h"
#include "geodetic.h"

inline bool geodetic_equals( const geodetic_t *const a, const geodetic_t *const b ) {
	return fcmpd( a->lon, b->lon ) &&
		   fcmpd( a->lat, b->lat ) &&
		   fcmpd( a->height, b->height );
}

inline geodetic_extent_t *geodetic_extent_create(
		const geodetic_t *const south_west, const geodetic_t *const north_east, geodetic_extent_t *geodetic ) {
	geodetic->sw_lat = south_west->lat;
	geodetic->sw_lon = south_west->lon;
	geodetic->ne_lat = north_east->lat;
	geodetic->ne_lon = north_east->lon;
	return geodetic;
}

inline bool geodetic_extent_equals( const geodetic_extent_t *const a, const geodetic_extent_t *const b ) {
	return fcmpd( a->ne_lat, b->ne_lat ) &&
		   fcmpd( a->ne_lon, b->ne_lon ) &&
		   fcmpd( a->sw_lat, b->sw_lat ) &&
		   fcmpd( a->sw_lon, b->sw_lon );
}
