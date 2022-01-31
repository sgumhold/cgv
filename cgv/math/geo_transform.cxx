#include "geo_transform.h"
#include <cmath>

namespace cgv {
	namespace math {

		static const double PI = 3.1415926535898;
		static const double TWO_PI = 6.28318530718;
		static const double DEG_TO_RAD = 0.01745329252;
		static const double RAD_TO_DEG = 57.2957795131;

		// One millimeter tolerance.
		static const double POSITION_TOLERANCE = 0.001;

		typedef fvec<float, 3> vec3;
		typedef fvec<double, 2> dvec2;
		typedef fvec<double, 3> dvec3;

		dvec3 ECFE_from_geodetic(const dvec3& geodetic, double a, double e)
		{
			const double& phi = geodetic.x();
			const double& lambda = geodetic.y();
			const double& h = geodetic.z();

			// convert deg to rad
			double phi_rad = phi * DEG_TO_RAD;
			double lambda_rad = lambda * DEG_TO_RAD;

			// some precalculations
			double e_2 = e * e;
			double sin_phi = sin(phi_rad);
			double cos_phi = cos(phi_rad);
			double sin_lambda = sin(lambda_rad);
			double cos_lambda = cos(lambda_rad);
			double N = a / sqrt(1 - e_2 * sin_phi * sin_phi);

			//... finally
			return dvec3(
				(h + N) * cos_phi * cos_lambda,
				(h + N) * cos_phi * sin_lambda,
				(h + (1 - e_2) * N) * sin_phi);
		}
		
		
		template <class reference_ellipsoid>
		void cartesian_to_geodetic_iterative(const double X, const double Y, const double Z,
			double& phi, double& lambda, double& h)
		{
		}

		dvec3 geodetic_from_ECFE(const dvec3& ECFE, double a, double e)
		{
			// From: GPStk
			// Iterative routine to convert cartesian (ECEF) to geodetic coordinates,
			// (Geoid specified by semi-major axis and eccentricity squared).
			// @param xyz (input): X,Y,Z in meters
			// @param llh (output): geodetic lat(deg N), lon(deg E),
			//                      height above ellipsoid (meters)
			const double& X = ECFE[0]; 
			const double& Y = ECFE[1]; 
			const double& Z = ECFE[2];

			dvec3 geodetic;
			double& phi = geodetic[0];
			double& lambda = geodetic[1];
			double& h = geodetic[2];

			double eccSq = e*e;
			double p, slat, N, htold, latold;
			p = sqrt(X * X + Y * Y);
			if (p < POSITION_TOLERANCE / 5)
			{  // pole or origin
				phi = (Z > 0 ? 90.0 : -90.0);
				lambda = 0;                            // lon undefined, really
				h = ::fabs(Z) - a * sqrt(1.0 - eccSq);
				return geodetic;
			}
			phi = ::atan2(Z, p * (1.0 - eccSq));
			h = 0;
			for (int i = 0; i < 5; i++) {
				slat = ::sin(phi);
				N = a / sqrt(1.0 - eccSq * slat * slat);
				htold = h;
				h = p / ::cos(phi) - N;
				latold = phi;
				phi = ::atan2(Z, p * (1.0 - eccSq * (N / (N + h))));
				if (::fabs(phi - latold) < 1.0e-9 && fabs(h - htold) < 1.0e-9 * a) break;
			}
			lambda = ::atan2(Y, X);
			if (lambda < 0.0) lambda += TWO_PI;
			phi *= RAD_TO_DEG;
			lambda *= RAD_TO_DEG;
			return geodetic;
		}
	
		dvec3 geodetic_from_ECFE_approx(const dvec3& ECFE, double a, double e)
		{
			// converts from ECEF-r to ECEF-g in a closed form
			// input:  [X,Y,Z] in meters
			// output: [phi,lambda,h], lat. (deg N), long. (deg E), height above ellipsoid (in meters)			void cartesian_to_geodetic(const double X, const double Y, const double Z, double& phi, double& lambda, double& h)
			const double& X = ECFE[0];
			const double& Y = ECFE[1];
			const double& Z = ECFE[2];

			dvec3 geodetic;
			double& phi = geodetic[0];
			double& lambda = geodetic[1];
			double& h = geodetic[2];

			phi = 0;
			lambda = 0;
			h = 0;

			double a_2 = a * a;
			double e_2 = e * e;
			double e_4 = e_2 * e_2;
			double X_2 = X * X;
			double Y_2 = Y * Y;
			double Z_2 = Z * Z;

			if (sqrt(X_2 + Y_2 + Z_2) > (a * e_2 / (sqrt(1.0 - e_2)))) {
				if (!((std::abs(X) < POSITION_TOLERANCE) && (std::abs(Y) < POSITION_TOLERANCE))) // <- TODO: adjust tolerance? 
				{
					// some intermediate results...
					double p = (X_2 + Y_2) / a_2;
					double q = ((1.0 - e_2) / a_2) * Z_2;
					double r = (p + q - e_4) / 6.0;
					double s = e_4 * ((p * q) / (4.0 * r * r * r));
					double t = std::pow(1.0 + s + sqrt(s * (2.0 + s)), 1.0 / 3.0); // <- TODO : could negative values occur under the cube root? 
					double u = r * (1.0 + t + 1.0 / t);
					double v = sqrt(u * u + e_4 * q);
					double w = e_2 * (u + v - q) / (2 * v);
					double k = sqrt(u + v + w * w) - w;
					double D = (k * sqrt(X_2 + Y_2)) / (k + e_2);

					double D_2 = D * D;

					// ... finally

					// HEIGHT
					h = ((k + e_2 - 1) / k) * sqrt(D_2 + Z_2);

					// LATITUDE
					phi = 2 * atan(Z / (D + sqrt(D_2 + Z_2)));
					phi *= RAD_TO_DEG;

					// LONGITUDE
					if (Y >= 0.0)
					{
						lambda = 0.5 * PI - 2.0 * atan(X / (sqrt(X_2 + Y_2) + Y));
					}
					else
					{
						lambda = -0.5 * PI + 2.0 * atan(X / (sqrt(X_2 + Y_2) + Y));
					}
					lambda *= RAD_TO_DEG;

				}
				else
					std::cout << "\nError! Input coordinates not valid";
			}
			return geodetic;
		}

		dvec3 ENU_from_geodetic(const dvec3& position_geodetic, const dvec3& origin_geodetic, double a, double e)
		{
			const double& phi_geo = position_geodetic[0];
			const double& lambda_geo = position_geodetic[1];
			const double& h_geo = position_geodetic[2];
			const double& phi_origin = origin_geodetic[0];
			const double& lambda_origin = origin_geodetic[1];
			const double& h_origin = origin_geodetic[2];
			// some precalculations
			double phi_rad = phi_origin * DEG_TO_RAD;
			double lambda_rad = lambda_origin * DEG_TO_RAD;
			double sin_phi = sin(phi_rad);
			double cos_phi = cos(phi_rad);
			double sin_lambda = sin(lambda_rad);
			double cos_lambda = cos(lambda_rad);

			// calculate ECEF-r coordinates
			dvec3 position_ECFE = ECFE_from_geodetic(position_geodetic, a, e);
			dvec3 origin_ECFE = ECFE_from_geodetic(origin_geodetic, a, e);
			const double& X_geo = position_ECFE[0];
			const double& Y_geo = position_ECFE[1];
			const double& Z_geo = position_ECFE[2];
			const double& X_origin = origin_ECFE[0];
			const double& Y_origin = origin_ECFE[1];
			const double& Z_origin = origin_ECFE[2];

			// we translate our coordinate system into the given origin and rotate around z-axis and x-axis
			// we define a transformation from LTS-r to ECEF-r as follows: x_geo = origin + R * x_lts with R = R_z(phi) * swap(x,y) * R_x(lambda) * swap(y, z) 
			// [R_x/z = cartesian rotation around x-/z-axis]
			// hence our method has to return the inverse transformation: x_lts = R^T * (x_geo - origin) = swap(y, z) * R_x(lambda) * swap(x,y) * R_z(phi) * (x_geo - origin)
			double x = X_geo - X_origin;
			double y = Y_geo - Y_origin;
			double z = Z_geo - Z_origin;

			return dvec3(
				-sin_lambda * x + cos_lambda * y,
				-sin_phi * cos_lambda * x - sin_phi * sin_lambda * y + cos_phi * z,
				cos_phi * cos_lambda * x + cos_phi * sin_lambda * y + sin_phi * z);
		}
	}
}