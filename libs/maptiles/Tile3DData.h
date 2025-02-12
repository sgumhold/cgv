#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Config.h"

// Class to store all data related to a Tile3D
class Tile3DData
{
public:
	MAPTILES_API Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>&& mesh);
	MAPTILES_API Tile3DData(Tile3DData&& other) noexcept;
	MAPTILES_API ~Tile3DData();
	MAPTILES_API void ConvertTo3DCoordinates(double refLat, double refLon);

 public:
	bool valid;
	double lat_min, lon_min, lat_max, lon_max;
	double ref_lat, ref_lon;
	std::vector<glm::dvec3> mesh_cartesian;
	std::vector<glm::dvec3> mesh_wgs;
};
