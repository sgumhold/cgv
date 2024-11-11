#include "Tile3DData.h"

#include "WGS84toCartesian.hpp"
#include <iostream>

Tile3DData::Tile3DData() : lat_min(0), lon_min(0), lat_max(0), lon_max(0), ref_lat(0), ref_lon(0)
{

}

Tile3DData::Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>& _mesh)
	: lat_min(latMin), lon_min(lonMin), lat_max(latMax), lon_max(lonMax), ref_lat(0), ref_lon(0)
{
	mesh_wgs = std::move(_mesh);
}

void Tile3DData::ConvertTo3DCoordinates(double refLat, double refLon)
{
	ref_lat = refLat;
	ref_lon = refLon;

	mesh_cartesian.clear();

	for (int i = 0; i < mesh_wgs.size(); i += 6)
	{
		auto p0 = wgs84::toCartesian({ refLat, refLon }, { mesh_wgs[i].x, mesh_wgs[i].y });
		auto p1 = wgs84::toCartesian({ refLat, refLon }, { mesh_wgs[i + 2].x, mesh_wgs[i + 2].y });
		auto p2 = wgs84::toCartesian({ refLat, refLon }, { mesh_wgs[i + 4].x, mesh_wgs[i + 4].y });

		// the inversion of the z-axis is required in order to convert from the left-hand coordinate system (used during the wgs to cartesian conversion)
		// to the right-hand coordinate system (used for rendering)
		glm::dvec3 v0 = { p0[0], mesh_wgs[i][2],	 -p0[1]};
		glm::dvec3 v1 = { p1[0], mesh_wgs[i + 2][2], -p1[1]};
		glm::dvec3 v2 = { p2[0], mesh_wgs[i + 4][2], -p2[1]};

		glm::dvec3 normal = glm::normalize(glm::cross((v0 - v1), (v2 - v1)));
		mesh_cartesian.push_back(v0);
		mesh_cartesian.push_back(mesh_wgs[i + 1]);
		mesh_cartesian.push_back(normal);
		mesh_cartesian.push_back(v2);
		mesh_cartesian.push_back(mesh_wgs[i + 5]);
		mesh_cartesian.push_back(normal);
		mesh_cartesian.push_back(v1);
		mesh_cartesian.push_back(mesh_wgs[i + 3]);
		mesh_cartesian.push_back(normal);
	}
}
