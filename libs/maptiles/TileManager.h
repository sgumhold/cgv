#pragma once

#include "Config.h"
#include "TileManagerData.h"
#include "RasterTileData.h"
#include "RasterTileRender.h"
#include "Tile3DData.h"
#include "Tile3DRender.h"

#include <cgv/render/context.h>

#include <vector>

class MAPTILES_API TileManager
{
public:
	TileManager();

	void Init(double _lat, double _lon, double altitude, GlobalConfig& conf);
	void Finalize();

	void Update(cgv::render::context& ctx);
	void SetPosition(double _lat, double _lon, double _alt);
	std::map<Tile3DIndex, Tile3DRender>& GetActiveTile3Ds() { return active_tile3D; }
	std::map<RasterTileIndex, RasterTileRender>& GetActiveRasterTiles() { return active_raster_tile; }

private:
	void GenerateRasterTileNeighbours();
	void GenerateTile3DNeighbours();
	void RemoveRasterTiles();
	void RemoveTile3Ds();
	void PruneNeighbourSetRasterTile();
	void PruneNeighbourSetTile3D();
	void GetRasterTileNeighbours();
	void GetTile3DNeighbours();
	void AddRasterTileToQueue(RasterTileIndex index);
	void AddTile3DToQueue(Tile3DIndex index);
	void AddRasterTiles(cgv::render::context& ctx);
	void AddTile3D(cgv::render::context& ctx);

	bool AllQueuesEmpty();
	bool AllBackgroundTilesProcessed();
	
	inline float GetZoom()
	{
		return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(lat)) /
										  (std::max(1.0, altitude)))); /*Circumference of earth = 40075016*/
	}
	inline float GetZoomAtAltitude(float alt)
	{
		return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(lat)) /
										  (std::max(1.0f, alt)))); /*Circumference of earth = 40075016*/
	}

private:
	double lat, lon, altitude;
	GlobalConfig config;
	TileManagerData tile_manager_data;

	std::set<RasterTileIndex> neighbour_set_raster_tile;
	std::set<Tile3DIndex> neighbour_set_tile3D;
	std::map<RasterTileIndex, RasterTileRender> active_raster_tile;
	std::map<Tile3DIndex, Tile3DRender> active_tile3D;
	std::map<RasterTileIndex, RasterTileRender> background_raster_tile;
	std::map<Tile3DIndex, Tile3DRender> background_tile3D;
	std::map<RasterTileIndex, RasterTileData> queue_raster_tiles;
	std::map<Tile3DIndex, Tile3DData> queue_tile3Ds;

	mutable std::mutex m_MutexBackgroundRasterTiles;
	mutable std::mutex m_MutexBackgroundTile3Ds;

	mutable std::mutex m_MutexNeighbourSetRasterTiles;
	mutable std::mutex m_MutexNeighbourSetTile3Ds;

	mutable std::mutex m_MutexQueueRasterTiles;
	mutable std::mutex m_MutexQueueTile3Ds;

	bool moved = true;
	bool last_update_finsihed = true;
};