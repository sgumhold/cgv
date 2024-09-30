#pragma once

#include "OSMDataLoader.h"

#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

// OSM element type: Node
// A node is essentially just a geographical location on the map along with an ID
class MAPTILES_API OSMNode
{
public:
	OSMNode() : m_id(0), m_lat(0), m_lon(0) {}
	OSMNode(unsigned long long id, double lat, double lon) : m_id(id), m_lat(lat), m_lon(lon) {}
public:
	unsigned long long m_id;
	double m_lat;
	double m_lon;
};

// OSM element type: Way
// A way is used to represent all different kind of structures on the map (for example: buildings and roads)
// A way consists of a list of node references that define the shape of the structure 
// It also consists tags that provide additional information about the structure
class OSMWay
{
public:
	OSMWay(unsigned long long id) : m_id(id), m_type(WayType::closed) {}
public:
	// There are two way types:
	// Closed ways: which have vertices that form a closed loop. Used to represent structures like buildings
	// Open ways: which don't form a closed loops. Used to represent structures like roads
	enum WayType
	{
		closed, open
	};

	WayType m_type;
	unsigned long long m_id;
	std::unordered_map<std::string, std::string> m_tags;
	std::vector<unsigned long long> m_nodeReferences;
};

// This class takes as input the raw response from the overpass API and extracts the OSMNodes and OSMWays contained in that response
// It used the OSMNodes and OSMWays to construct geometry and stores it in a mesh
class OSMDataProcessor
{
public:
	MAPTILES_API OSMDataProcessor(OSMDataLoader& loader);
	MAPTILES_API std::vector<glm::dvec3>& GetTileGeometry() { return mesh; }

private:
	void Parse(std::string& response);
	void ExtractTileGeometry();
	void ExtractBuildingGeometry(OSMWay& way);

private:
	std::unordered_map<unsigned long long, OSMNode> m_nodes;	// a map of all the OSM nodes referenced by all the OSM ways in the tile
	std::vector<OSMWay> m_ways;									// a list of all the OSM ways in the tile

	// a list of vertices of all the building meshes
	// 3 consecutive vertices form a face
	std::vector<glm::dvec3> mesh;
};