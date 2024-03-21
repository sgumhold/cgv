#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Config.h"

// Class to store all data related to a Tile3D
class Tile3DData
{
public:
	MAPTILES_API Tile3DData();
	MAPTILES_API Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>& mesh);
	MAPTILES_API void ConvertTo3DCoordinates(double refLat, double refLon);

  public:
	double m_latMin, m_lonMin, m_latMax, m_lonMax;
	std::vector<glm::dvec3> m_mesh;
};
