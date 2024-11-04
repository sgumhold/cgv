#include "TileManagerData.h"

#include "OSMRasterTileLoader.h"
#include "OSMRasterTileProcessor.h"
#include "OSMDataLoader.h"
#include "OSMDataProcessor.h"

#include "utils.h"

#include "Timer.h"

TileManagerData::TileManagerData() : m_config(nullptr) 
{
}

TileManagerData::~TileManagerData()
{
}

void TileManagerData::Init(GlobalConfig* config)
{
	m_config = config;
}

RasterTileData& TileManagerData::GetRasterTile(int zoom, int x, int y)
{
	RasterTileIndex index = { (unsigned)zoom, x, y };
	

	if (m_RasterTileCache.find(index) != m_RasterTileCache.end())
	{
		return m_RasterTileCache[index];
	}

	OSMRasterTileLoader loader(zoom, x, y);
	loader.FetchTile();

	OSMRasterTileProcessor processor(loader);
	RasterTileData rasterTile(processor.GetImage(), processor.GetHeight(), processor.GetWidth(), zoom, loader.GetX(), loader.GetY());

	std::lock_guard<std::mutex> lockActive(m_MutexRasterTiles);
	m_RasterTileCache[index] = std::move(rasterTile);

	return m_RasterTileCache[index];
}

RasterTileData& TileManagerData::GetRasterTile(int zoom, double lat, double lon)
{
	int x = long2tilex(lon, zoom);
	int y = lat2tiley(lat, zoom);

	return GetRasterTile(zoom, x, y);
}

Tile3DData& TileManagerData::GetTile3D(double lat, double lon)
{
	Tile3DIndex index = { lat, lon };
	

	if (m_Tile3DCache.find(index) != m_Tile3DCache.end())
	{
		auto& tile = m_Tile3DCache[index];
		if (tile.ref_lat == m_config->ReferencePoint.lat && tile.ref_lon == m_config->ReferencePoint.lon)
		{
			return tile;
		}
		tile.ConvertTo3DCoordinates(m_config->ReferencePoint.lat, m_config->ReferencePoint.lon);
		std::cout << "Changing ref point for tile3D to (" << m_config->ReferencePoint.lat << ", "
				  << m_config->ReferencePoint.lon << ")\n";
		return tile;
	}

	double size = m_config->Tile3DSize;
	OSMDataLoader loader(lat, lon, lat + size, lon + size);
	loader.FetchOSMWays();
	OSMDataProcessor processor(loader);
	Tile3DData tile3DData(lat, lon, lat + size, lon + size, processor.GetTileGeometry());

	tile3DData.ConvertTo3DCoordinates(m_config->ReferencePoint.lat, m_config->ReferencePoint.lon);

	std::lock_guard<std::mutex> lockActive(m_MutexTile3Ds);
	m_Tile3DCache[index] = tile3DData;
	std::cout << "Cache Size: " << m_Tile3DCache.size() << " (" << index.lat << ", " << index.lon << ")\n";

	return m_Tile3DCache[index];
}

