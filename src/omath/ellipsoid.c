
#include "ellipsoid.h"
#include <tgmath.h>

inline ellipsoid_t *ellipsoid_create( const double x, const double y, const double z, ellipsoid_t *e ) {
	e->radii.x = x, e->radii.y = y; e->radii.z = z;
	vec3d_mul( &e->radii, &e->radii, &e->radii_squared );
	vec3d_mul( &e->radii_squared, &e->radii_squared, &e->radii_to_the_fourth );
	vec3d_div_s( &e->radii_squared, 1.0, &e->one_over_radii_squared );
	return e;
}

inline ellipsoid_t *ellipsoid_create_from_vec( const vec3d *const radii, ellipsoid_t *el ) {
	return ellipsoid_create( radii->x, radii->y, radii->z, el );
}

inline vec3d *ellipsoid_geodetic_normal_from_geodetic( const geodetic_t *const geo, vec3d *n ) {
	double cos_lat = cos(radiansd(geo->lat));
	n->x = cos_lat * cos(radiansd(geo->lon));
	n->y = cos_lat * sin(radiansd(geo->lon));
	n->z = sin(radiansd(geo->lat));
	return n;
}

inline vec3d *ellipsoid_to_cartesian(
		const geodetic_t *const geodetic, const ellipsoid_t *const e, vec3d *cartesian ) {
	vec3d n;
	ellipsoid_geodetic_normal_from_geodetic( geodetic, &n );
	const vec3d k = { e->radii_squared.x*n.x, e->radii_squared.y*n.y, e->radii_squared.z*n.z };
	const double gamma = sqrt( k.x*n.x + k.y*n.y + k.z*n.z );
	const vec3d r_surface = { k.x/gamma, k.y/gamma, k.z/gamma };
	const vec3d height_offset = { geodetic->height*n.x, geodetic->height*n.y, geodetic->height*n.z };
	return vec3d_add( &r_surface, &height_offset, cartesian );
}

inline vec3d *CentricSurfaceNormal( const vec3d *const pos, vec3d *n ) {
	return vec3d_normalize( pos, n );
}

inline vec3d *GeodeticSurfaceNormal( const vec3d *const pos, const ellipsoid_t *const e, vec3d *n ) {
	vec3d temp;
	return vec3d_normalize( vec3d_mul( pos, &e->one_over_radii_squared, &temp ), n );
}

inline double MinimumRadius( const ellipsoid_t *const e ) {
	const double m = e->radii.x < e->radii.z ? e->radii.x : e->radii.z;
	return e->radii.y < m ? e->radii.y : m;
}

inline double MaximumRadius( const ellipsoid_t *const e ) {
	const double m = e->radii.x > e->radii.z ? e->radii.x : e->radii.z;
	return e->radii.y > m ? e->radii.y : m;
}

unsigned int Intersections(
		const vec3d *const origin, const vec3d *const direction,
		const ellipsoid_t *const e, double *intersections[2] ) {
	vec3d dn;
	vec3d_normalize( direction, &dn );
	// By laborious algebraic manipulation ... (source: Virtual Globe Book by Cozzy/Ring)
	double a = dn.x * dn.x * e->one_over_radii_squared.x +
			dn.y * dn.y * e->one_over_radii_squared.y +
			dn.z * dn.z * e->one_over_radii_squared.z;
	double b = 2.0f * ( origin->x * dn.x * e->one_over_radii_squared.x +
			origin->y * dn.y * e->one_over_radii_squared.y +
			origin->z * dn.z * e->one_over_radii_squared.z );
	double c = origin->x * origin->x * e->one_over_radii_squared.x +
			origin->y * origin->y * e->one_over_radii_squared.y +
			origin->z * origin->z * e->one_over_radii_squared.z - 1.0f;
	// Solve the quadratic equation: ax^2 + bx + c = 0.
	// Algorithm is from Wikipedia's "Quadratic equation" topic, and Wikipedia credits
	// Numerical Recipes in C, section 5.6: "Quadratic and Cubic Equations"
	double discriminant = b * b - 4 * a * c;
	if( discriminant < 0.0 )
		return 0;
	else if( discriminant == 0.0f ) {
		// one intersection at a tangent point
		*intersections[0] = -0.5f * b / a;
		return 1;
	}
	double t = -0.5f * (b + (b > 0.0f ? 1.0f : -1.0f) * sqrt(discriminant));
	double root1 = t / a;
	double root2 = c / t;
	// two intersections - return the smallest first.
	if( root1 < root2 ) {
		*intersections[0] = root1;
		*intersections[0] = root2;
	} else {
		*intersections[0] = root2;
		*intersections[0] = root1;
	}
	return 2;
}

// @todo convert an array of cartesian positions to geodetic
/*public ICollection<Geodetic3D> ToGeodetic3D(IEnumerable<Vector3D> positions) {
	IList<Geodetic3D> geodetics = new List<Geodetic3D>(CollectionAlgorithms.EnumerableCount(positions));
	foreach (Vector3D position in positions)
	geodetics.Add(ToGeodetic3D(position));
	return geodetics;
}*/

vec3d *ScaleToGeodeticSurface( const vec3d *const position, const ellipsoid_t *const e, vec3d *scaled_pos ) {
	double beta = 1.0f / sqrt(
			position->x * position->x * e->one_over_radii_squared.x +
			position->y * position->y * e->one_over_radii_squared.y +
			position->z * position->z * e->one_over_radii_squared.z );
	vec3d temp = {
			beta * position->x * e->one_over_radii_squared.x,
			beta * position->y * e->one_over_radii_squared.y,
			beta * position->z * e->one_over_radii_squared.z
	};
	double n = vec3d_magnitude(&temp);
	double alpha = (1.0f-beta) * ( vec3d_magnitude(position) / n );
	double x2 = position->x * position->x;
	double y2 = position->y * position->y;
	double z2 = position->z * position->z;
	double da = 0.0;
	double db = 0.0;
	double dc = 0.0;
	double s = 0.0;
	double dSdA = 1.0;
	do {
		alpha -= (s / dSdA);
		da = 1.0f + (alpha * e->one_over_radii_squared.x);
		db = 1.0f + (alpha * e->one_over_radii_squared.y);
		dc = 1.0f + (alpha * e->one_over_radii_squared.z);
		double da2 = da * da;
		double db2 = db * db;
		double dc2 = dc * dc;
		double da3 = da * da2;
		double db3 = db * db2;
		double dc3 = dc * dc2;
		s = x2/(e->radii_squared.x*da2) + y2/(e->radii_squared.y*db2) + z2/(e->radii_squared.z*dc2) - 1.0f;
		dSdA = -2.0f * (
				x2/(e->radii_to_the_fourth.x*da3) +
				y2/(e->radii_to_the_fourth.y*db3) +
				z2/(e->radii_to_the_fourth.z*dc3)
		);
	}
	while( fabs(s) > 1e-10 );
	scaled_pos->x = position->x / da;
	scaled_pos->y = position->y / db;
	scaled_pos->z = position->z / dc;
	return scaled_pos;
 }

 /*public Geodetic2D ToGeodetic2D(Vector3D positionOnEllipsoid) {
	 Vector3D n = GeodeticSurfaceNormal(positionOnEllipsoid);
	 return new Geodetic2D(
			 Math.Atan2(n.y, n.x),
			 Math.Asin(n.z / n.Magnitude));
 }*/

geodetic_t *ToGeodetic3D( const vec3d *const position, const ellipsoid_t *const e, geodetic_t *geo ) {
	vec3d p, h, n;
	ScaleToGeodeticSurface( position, e, &p );
	vec3d_sub( position, &p, &h );
	double height = ( vec3d_dot( &h, position ) < 0.0f ? -1.0f : 1.0f ) * vec3d_magnitude(&h);
	GeodeticSurfaceNormal( &p, e, &n );
	geo->lon = atan2( n.y, n.x );
	geo->lat = asin( n.z / vec3d_magnitude(&n) );
	geo->height = height;
	return geo;
}

inline vec3d *ScaleToGeocentricSurface( const vec3d *const position, const ellipsoid_t *const e, vec3d *scaled ) {
	double beta = 1.0f / sqrt(
			position->x * position->x * e->one_over_radii_squared.x +
			position->y * position->y * e->one_over_radii_squared.y +
			position->z * position->z * e->one_over_radii_squared.z );
	vec3d_mul_s( position, beta, scaled );
	return scaled;
}

// granularity must be > 0.0
/*void ComputeCurve( const vec3d *const start, const vec3d *const stop, const double granularity ) {
	vec3d normal, temp;
	vec3d_normalize( vec3d_cross( start, stop, &temp ), &normal );
	double theta = start.AngleBetween(stop);
	int n = Math.Max((int)(theta / granularity) - 1, 0);
	List<Vector3D> positions = new List<Vector3D>(2 + n);
	positions.Add(start);
	for( int i = 1; i <= n; ++i ) {
		double phi = (i * granularity);
		positions.Add(ScaleToGeocentricSurface(start.RotateAroundAxis(normal, phi)));
	}
	positions.Add(stop);
	return positions;
}*/
