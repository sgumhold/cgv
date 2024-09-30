#pragma once

#include <map>
#include <mutex>

#include "RasterTileData.h"
#include "Tile3DData.h"
#include "Config.h"


struct RasterTileIndex
{
	unsigned int zoom;
	int x;
	int y;

	bool operator < (const RasterTileIndex& rhs) const
	{
		if (zoom == rhs.zoom)
		{
			if (x == rhs.x)
			{
				return y < rhs.y;
			}
			else
			{
				return x < rhs.x;
			}
		}
		else
		{
			return zoom < rhs.zoom;
		}
	}
};


struct Tile3DIndex
{
	double lat;
	double lon;

	bool operator != (const Tile3DIndex& rhs) const
	{
		if (std::abs(lat - rhs.lat) > 1e-6 || std::abs(lon - rhs.lon) > 1e-6)
		{
			return true;
		}
		return false;
	}

	bool operator < (const Tile3DIndex& rhs) const
	{
		if (std::abs(lat - rhs.lat) < 1e-6)
			return lon < rhs.lon;
		return lat < rhs.lat;
	}
};


// A class to manage the creation of all the differnt tile types
// Internally, it used the 'Loader' and 'Processor' classes of the tile types to retrieve API responses and extracting relevant information
// It also caches the tiles so that that subsequent requests for the same tile can be fulfilled quicker.
class TileManagerData
{
public:
	MAPTILES_API TileManagerData();
	MAPTILES_API ~TileManagerData();

	MAPTILES_API void Init(GlobalConfig config);
	
	MAPTILES_API RasterTileData& GetRasterTile(int zoom, int x, int y);
	MAPTILES_API RasterTileData& GetRasterTile(int zoom, double lat, double lon);
	MAPTILES_API Tile3DData& GetTile3D(double lat, double lon);

private:
	GlobalConfig m_config;

	std::map<RasterTileIndex, RasterTileData> m_RasterTileCache;
	std::map<Tile3DIndex, Tile3DData> m_Tile3DCache;

	mutable std::mutex m_MutexRasterTiles;
	mutable std::mutex m_MutexTile3Ds;
};
