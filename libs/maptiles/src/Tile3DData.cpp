#include "Tile3DData.h"

#include "WGS84toCartesian.hpp"
#include <iostream>

Tile3DData::Tile3DData()
	: lat_min(0), lon_min(0), lat_max(0), lon_max(0)
{

}

Tile3DData::Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>& _mesh)
	: lat_min(latMin), lon_min(lonMin), lat_max(latMax), lon_max(lonMax)
{
	mesh = std::move(_mesh);
}

Tile3DData::Tile3DData(double latMin, double lonMin, double latMax, double lonMax) 
{ 
	lat_min = latMin;
	lat_max = latMax;
	lon_min = lonMin;
	lon_max = lonMax;

	mesh.push_back({latMin, lonMin, 0});
	mesh.push_back({1.0f, 1.0f, 0.0f});

	mesh.push_back({latMax, lonMin, 0});
	mesh.push_back({1.0f, 1.0f, 0.0f});

	mesh.push_back({latMax, lonMax, 0});
	mesh.push_back({1.0f, 1.0f, 0.0f});

	mesh.push_back({latMin, lonMin, 0});
	mesh.push_back({0.0f, 1.0f, 0.0f});

	mesh.push_back({latMax, lonMax, 0});
	mesh.push_back({0.0f, 1.0f, 0.0f});

	mesh.push_back({latMin, lonMax, 0});
	mesh.push_back({0.0f, 1.0f, 0.0f});

}

void Tile3DData::ConvertTo3DCoordinates(double refLat, double refLon)
{
	std::vector<glm::dvec3> newMesh;

	for (int i = 0; i < mesh.size(); i += 6)
	{
		auto p0 = wgs84::toCartesian({ refLat, refLon }, { mesh[i].x, mesh[i].y });
		auto p1 = wgs84::toCartesian({ refLat, refLon }, { mesh[i + 2].x, mesh[i + 2].y });
		auto p2 = wgs84::toCartesian({ refLat, refLon }, { mesh[i + 4].x, mesh[i + 4].y });

		glm::dvec3 v2 = { p0[0], p0[1], mesh[i][2] };
		glm::dvec3 v1 = { p1[0], p1[1], mesh[i + 2][2] };
		glm::dvec3 v0 = { p2[0], p2[1], mesh[i + 4][2] };

		glm::dvec3 normal = glm::normalize(glm::cross((v2 - v1), (v0 - v1)));
		newMesh.push_back(v0);
		newMesh.push_back(mesh[i + 5]);
		newMesh.push_back(normal);
		newMesh.push_back(v1);
		newMesh.push_back(mesh[i + 3]);
		newMesh.push_back(normal);
		newMesh.push_back(v2);
		newMesh.push_back(mesh[i + 1]);
		newMesh.push_back(normal);
	}

	std::swap(mesh, newMesh);
	newMesh.clear();
}
