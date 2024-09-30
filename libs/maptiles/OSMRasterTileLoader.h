#pragma once

#include <string>
#include "httplib.h"
#include "Config.h"

// Class to construct a request for and fetch a Raster Tile at a given location and zoom level from OSM tile server
class OSMRasterTileLoader
{
public:
	MAPTILES_API OSMRasterTileLoader(int zoom, double lat, double lon);
	MAPTILES_API OSMRasterTileLoader(int zoom, int x, int y);

	MAPTILES_API void FetchTile();
	MAPTILES_API std::string& GetResponse() { return m_result->body; };	// return the API response in the form of a string

	MAPTILES_API int GetErrorStatus() { return (int)m_result.error(); }
	MAPTILES_API int GetHTTPStatus() { return m_result->status; }

	MAPTILES_API int GetX() { return m_x; }
	MAPTILES_API int GetY() { return m_y; }

private:
	std::string m_url;
	std::string m_query;

	// Slippy tile map parameters
	// See: https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
	int m_zoom;
	int m_x;
	int m_y;

	httplib::Result m_result;
};
