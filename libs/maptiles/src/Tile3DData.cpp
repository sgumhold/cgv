#include "Tile3DData.h"

#include "WGS84toCartesian.hpp"

Tile3DData::Tile3DData()
	: m_latMin(0), m_lonMin(0), m_latMax(0), m_lonMax(0)
{

}

Tile3DData::Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>& mesh)
	: m_latMin(latMin), m_lonMin(lonMin), m_latMax(latMax), m_lonMax(lonMax)
{
	m_mesh = std::move(mesh);
}

void Tile3DData::ConvertTo3DCoordinates(double refLat, double refLon)
{
	std::vector<glm::dvec3> newMesh;

	for (int i = 0; i < m_mesh.size(); i += 6)
	{
		auto p0 = wgs84::toCartesian({ refLat, refLon }, { m_mesh[i].x, m_mesh[i].y });
		auto p1 = wgs84::toCartesian({ refLat, refLon }, { m_mesh[i + 2].x, m_mesh[i + 2].y });
		auto p2 = wgs84::toCartesian({ refLat, refLon }, { m_mesh[i + 4].x, m_mesh[i + 4].y });

		glm::vec3 v0 = { p0[0], p0[1], m_mesh[i][2] };
		glm::vec3 v1 = { p1[0], p1[1], m_mesh[i + 2][2] };
		glm::vec3 v2 = { p2[0], p2[1], m_mesh[i + 4][2] };


		glm::vec3 normal = glm::normalize(glm::cross((v0 - v1), (v2 - v1)));

		newMesh.push_back(v0);
		newMesh.push_back(m_mesh[i + 1]);
		newMesh.push_back(normal);
		newMesh.push_back(v1);
		newMesh.push_back(m_mesh[i + 3]);
		newMesh.push_back(normal);
		newMesh.push_back(v2);
		newMesh.push_back(m_mesh[i + 5]);
		newMesh.push_back(normal);
	}

	std::swap(m_mesh, newMesh);
	newMesh.clear();
}
