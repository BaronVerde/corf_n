
#include "aabb.h"
#include <float.h>
#include <tgmath.h>

inline vec3f* aabbf_get_center( const aabbf* const aabb, vec3f* out ) {
	vec3f temp;
	out = vec3f_mul_s( vec3f_add( &(aabb->min), &aabb->max, &temp ), 0.5f, out );
	return out;
}

inline vec3f* aabbf_get_size( const aabbf* const aabb, vec3f* out ) {
	return vec3f_sub( &(aabb->max), &(aabb->min), out );
}

inline vec3d* aabbd_get_center( const aabbd* const aabb, vec3d* out ) {
	vec3d temp;
	out = vec3d_mul_s( vec3d_add( &(aabb->min), &aabb->max, &temp ), 0.5, out );
	return out;
}

inline vec3d* aabbd_get_size( const aabbd* const aabb, vec3d* out ) {
	return vec3d_sub( &(aabb->max), &(aabb->min), out );
}

inline float aabbf_get_diagonal_size( const aabbf* const aabb ) {
	vec3f temp;
	return vec3f_magnitude( vec3f_sub( &(aabb->max), &(aabb->min), &temp ) );
}

inline double aabbd_get_diagonal_size( const aabbd* const aabb ) {
	vec3d temp;
	return vec3_magnitude( vec3d_sub( &(aabb->max), &(aabb->min), &temp ) );
}

inline bool aabb_intersect_other( const aabbf* const one, const aabbf* const other ) {
	return !( (other->max.x < one->min.x) || (other->min.x > one->max.x) ||
			  (other->max.y < one->min.y) || (other->min.y > one->max.y) ||
			  (other->max.z < one->min.z) || (other->min.z > one->max.z) );
}

inline float aabbf_min_distance_from_point_sq( const aabbf* const aabb, const vec3f* const point ) {
	float dist = 0.0f;
	if( point->x < aabb->min.x ) {
		float d = point->x - aabb->min.x;
		dist += d * d;
	} else if( point->x > aabb->max.x ) {
		float d = point->x - aabb->max.x;
		dist += d * d;
	}
	if( point->y < aabb->min.y ) {
		float d = point->y - aabb->min.y;
		dist += d * d;
	} else if( point->y > aabb->max.y ) {
		float d = point->y - aabb->max.y;
		dist += d * d;
	}
	if( point->z < aabb->min.z ) {
		float d = point->z - aabb->min.z;
		dist += d * d;
	} else if( point->z > aabb->max.z ) {
		float d = point->z - aabb->max.z;
		dist += d * d;
	}
	return dist;
}

inline double aabbd_min_distance_from_point_sq( const aabbd* const aabb, const vec3d* const point ) {
	double dist = 0.0;
	if( point->x < aabb->min.x ) {
		double d = point->x - aabb->min.x;
		dist += d * d;
	} else if( point->x > aabb->max.x ) {
		double d = point->x - aabb->max.x;
		dist += d * d;
	}
	if( point->y < aabb->min.y ) {
		double d = point->y - aabb->min.y;
		dist += d * d;
	} else if( point->y > aabb->max.y ) {
		double d = point->y - aabb->max.y;
		dist += d * d;
	}
	if( point->z < aabb->min.z ) {
		double d = point->z - aabb->min.z;
		dist += d * d;
	} else if( point->z > aabb->max.z ) {
		double d = point->z - aabb->max.z;
		dist += d * d;
	}
	return dist;
}

inline float aabb_max_distance_from_point_sq( const aabbf* const aabb, const vec3f* const point ) {
	float k = fmax( fabs( point->x - aabb->min.x ), fabs( point->x - aabb->max.x ) );
	float dist = k * k;
	k = fmax( fabs( point->y - aabb->min.y ), fabs( point->y - aabb->max.y ) );
	dist += k * k;
	k = fmax( fabs( point->z - aabb->min.z ), fabs( point->z - aabb->max.z ) );
	dist += k * k;
	return dist;
}

inline bool aabbf_intersect_sphere_sq( const aabbf* const aabb, const vec3f* const center, float radius_sq ) {
	return aabbf_min_distance_from_point_sq( aabb, center ) <= radius_sq;
}

inline bool aabbd_intersect_sphere_sq( const aabbd* const aabb, const vec3d* const center, double radius_sq ) {
	return aabbd_min_distance_from_point_sq( aabb, center ) <= radius_sq;
}

inline bool aabb_is_inside_sphere_sq( const aabbf* const aabb, const vec3f* const center, float radius_sq ) {
	return aabb_max_distance_from_point_sq( aabb, center ) <= radius_sq;
}

// @todo needs overwork to work with vectors
inline bool aabb_intersect_ray(
		const aabbf* const aabb, const vec3f* const ray_origin, const vec3f* const ray_direction, float* out_distance ) {
	float tmin = FLT_MIN;
	float tmax = FLT_MAX;
	const float _ray_origin[] = { ray_origin->x, ray_origin->y, ray_origin->z };
	const float _ray_direction[] = { ray_direction->x, ray_direction->y, ray_direction->z };
	const float _min[] = { aabb->min.x, aabb->min.y, aabb->min.z };
	const float _max[] = { aabb->max.x, aabb->max.y, aabb->max.z };
	const float epsilon = { 1e-5f };	// FLT_EPSILON is a bit tight for our case
	for( int i = 0; i < 3; ++i ) {
		if( fabs( _ray_direction[i] ) < epsilon ) {
			// Parallel to the plane
			if( _ray_origin[i] < _min[i] || _ray_origin[i] > _max[i] )
				return false;
		} else {
			const float ood = 1.0f / _ray_direction[i];
			float t1 = ( _min[i] - _ray_origin[i] ) * ood;
			float t2 = ( _max[i] - _ray_origin[i] ) * ood;
			if( t1 > t2 ) { // swap ?
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}
			if( t1 > tmin )
				tmin = t1;
			if (t2 < tmax)
				tmax = t2;
			if (tmin > tmax)
				return false;
		}
	}
	*out_distance = tmin;
	return true;
}

inline aabbf* aabb_enclose_other( const aabbf* const one, const aabbf* const other, aabbf* out_aabb ) {
	out_aabb->min.x = fmin( one->min.x, other->min.x );
	out_aabb->min.y = fmin( one->min.y, other->min.y );
	out_aabb->min.z = fmin( one->min.z, other->min.z );
	out_aabb->max.x = fmax( one->max.x, other->max.x );
	out_aabb->max.y = fmax( one->max.y, other->max.y );
	out_aabb->max.z = fmax( one->max.z, other->max.z );
	return out_aabb;
}

inline bool aabb_equals_other( const aabbf* const one, const aabbf* const other ) {
	return vec3f_fcmp( &(one->min), &(other->min) ) && vec3f_fcmp( &(one->max), &(other->max) );
}

inline float aabbf_get_bounding_sphere_radius( const aabbf* const aabb ) {
	vec3f temp;
	return vec3f_magnitude( aabbf_get_size( aabb, &temp ) ) * 0.5f;
}

inline double aabbd_get_bounding_sphere_radius( const aabbd* const aabb ) {
	vec3d temp;
	return vec3d_magnitude( aabbd_get_size( aabb, &temp ) ) * 0.5;
}

inline vec3f* aabbf_get_vertex_positive( const aabbf* const aabb, const vec3f* const normal, vec3f* out_positive ) {
	*out_positive = aabb->min;
	if( normal->x >= 0.0f )
		out_positive->x = aabb->max.x;
	if( normal->y >= 0.0f )
		out_positive->y = aabb->max.y;
	if( normal->z >= 0.0f )
		out_positive->z = aabb->max.z;
	return out_positive;
}

inline vec3f* aabbf_get_vertex_negative( const aabbf* const aabb, const vec3f* const normal, vec3f* out_negative ) {
	*out_negative = aabb->max;
	if( normal->x >= 0.0f )
		out_negative->x = aabb->min.x;
	if( normal->y >= 0.0f )
		out_negative->y = aabb->min.y;
	if( normal->z >= 0.0f )
		out_negative->z = aabb->min.z;
	return out_negative;
}

inline vec3d* aabbd_get_vertex_positive( const aabbd* const aabb, const vec3d* const normal, vec3d* out_positive ) {
	*out_positive = aabb->min;
	if( normal->x >= 0.0f )
		out_positive->x = aabb->max.x;
	if( normal->y >= 0.0f )
		out_positive->y = aabb->max.y;
	if( normal->z >= 0.0f )
		out_positive->z = aabb->max.z;
	return out_positive;
}

inline vec3d* aabbd_get_vertex_negative( const aabbd* const aabb, const vec3d* const normal, vec3d* out_negative ) {
	*out_negative = aabb->max;
	if( normal->x >= 0.0f )
		out_negative->x = aabb->min.x;
	if( normal->y >= 0.0f )
		out_negative->y = aabb->min.y;
	if( normal->z >= 0.0f )
		out_negative->z = aabb->min.z;
	return out_negative;
}

inline aabbf* aabb_expand( const aabbf* const aabb, const float percentage, aabbf* out_expanded ) {
	vec3f temp, offset;
	vec3f_mul_s( aabbf_get_size( aabb, &temp ), percentage, &offset );
	vec3f_sub( &(aabb->min), &offset, &(out_expanded->min) );
	vec3f_add( &(aabb->max), &offset, &(out_expanded->max) );
	return out_expanded;
}
