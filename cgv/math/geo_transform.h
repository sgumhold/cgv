#pragma once

#include <cgv/math/fvec.h>
#include "lib_begin.h"

namespace cgv {
	namespace math {

		/// major axis length of WGS84 ellipsoid
		static const double WGS84_a = 6378137.0;
		/// excentricity of WGS84 ellipsoid
		static const double WGS84_e = 0.081819191;

		/// scale of the EPSG 3857 coordinate system in Mercator projection
		const double epsg3857_scale = 20037508.3428;

		/// convert longitude and latitute given in degree to x and y in the Mercator projection in meters
		/// careful: compared to geodetic data where latitude comes first, in this function longitude comes first
		/// as it is linearly related to x of the Mercator projection 
		extern CGV_API void epsg4326_to_epsg3857(double lon, double lat, double& x, double& y);
		/// convert x and y in the Mercator projection in meters to longitude and latitute given in degree to 
		/// careful: compared to geodetic data where latitude comes first, in this function longitude comes first
		/// as it is linearly related to x of the Mercator projection 
		extern CGV_API void epsg3857_to_epsg4326(double x, double y, double& lon, double& lat);

		// converts from geodetic [latitude phi:deg N, longitude lambda:deg E, height h:meters] 
		// based on reference ellipsoid [semi major axis a, eccentricity e] to 
		// global orthonormal earth centered fixed earth (ECFE) coordinates in meters
		extern CGV_API fvec<double, 3> ECFE_from_geodetic(const fvec<double, 3>& geodetic, double a = WGS84_a, double e = WGS84_e);
		// converts from global orthonormal earth centered fixed earth (ECFE) coordinates in meters
		// based on reference ellipsoid [semi major axis a, eccentricity e] to 
		// geodetic [latitude phi:deg N, longitude lambda:deg E, height h:meters]
		extern CGV_API fvec<double, 3> geodetic_from_ECFE(const fvec<double, 3>& ECFE, double a = WGS84_a, double e = WGS84_e);
		// converts from global orthonormal earth centered fixed earth (ECFE) coordinates in meters
		// based on reference ellipsoid [semi major axis a, eccentricity e] to 
		// geodetic [latitude phi:deg N, longitude lambda:deg E, height h:meters] with non iterative and faster but less accurate approach
		extern CGV_API fvec<double, 3> geodetic_from_ECFE_approx(const fvec<double, 3>& ECFE, double a = WGS84_a, double e = WGS84_e);
		// converts from geodetic [latitude phi:deg N, longitude lambda:deg E, height h:meters] 
		// based on reference ellipsoid [semi major axis a, eccentricity e] to 
		// local East North (ENU) coordinates in meters
		extern CGV_API fvec<double, 3> ENU_from_geodetic(const fvec<double, 3>& position_geodetic, const fvec<double, 3>& origin_geodetic, double a = WGS84_a, double e = WGS84_e);
	}
}

#include <cgv/config/lib_end.h>