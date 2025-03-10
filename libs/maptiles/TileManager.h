#pragma once

#include "Config.h"
#include "TileManagerData.h"
#include "RasterTileData.h"
#include "RasterTileRender.h"
#include "Tile3DData.h"
#include "Tile3DRender.h"

#include <cgv/render/context.h>

#include <vector>
#include <array>

class MAPTILES_API TileManager
{
  public:
	TileManager();
	~TileManager();

	void Init(double _lat, double _lon, double altitude, GlobalConfig* conf);
	void ReInit(double _lat, double _lon, double altitude, GlobalConfig* conf);
	void Finalize();

	void Update(cgv::render::context& ctx);
	void SetPosition(double _lat, double _lon, double _alt);
	
	std::map<Tile3DIndex, Tile3DRender>& GetActiveTile3Ds();
	std::map<RasterTileIndex, RasterTileRender>& GetActiveRasterTiles();
	
	std::pair<std::array<double, 2>, std::array<double, 2>> GetExtent();

	void CalculateViewFrustum(const cgv::mat4& mvp);
	bool IsBoxCompletelyBehindPlane(const cgv::math::fvec<float, 3>& boxMin, const cgv::math::fvec<float, 3>& boxMax,
									const cgv::math::fvec<float, 4>& plane);

	void ClearRenderCache();
	void TrimRenderCache();
	void ClearDataCache();
	void TrimDataCache();

	// Signal for when a tile is downloaded. In this implmentation, this signal informs the application to post a redraw
	cgv::signal::signal<> tile_downloaded;

  private:
	void GenerateRasterTileFrustumNeighbours();
	void GenerateTile3DFrustumNeighbours();
	void GenerateRasterTileNeighbours();
	void GenerateTile3DNeighbours();

	template <typename index_type, typename data_type, typename render_type>
	void RemoveTiles(std::map<index_type, render_type>& active_tiles, std::set<index_type>& neighbour_set,
					 std::set<index_type>& requested_tiles, std::map<index_type, data_type&>& queue_tiles,
					 std::mutex& queue_lock);
	void RemoveRasterTiles();
	void RemoveTile3Ds();

	template <typename index_type, typename data_type, typename render_type>
	void PruneNeighbourSet(std::map<index_type, render_type>& active_tiles, std::set<index_type>& neighbour_set,
						   std::set<index_type>& requested_tiles, std::map<index_type, data_type&>& queue_tiles,
						   std::mutex& queue_lock, std::mutex& request_lock);
	void PruneNeighbourSetRasterTile();
	void PruneNeighbourSetTile3D();
	
	void GetRasterTileNeighbours();
	void GetTile3DNeighbours();

	template <typename index_type, typename data_type>
	void AddTilesToQueue(index_type index, std::set<index_type>& requested_tiles,
						 std::map<index_type, data_type&>& queue_tiles, std::mutex& queue_lock,
						 std::mutex& request_lock);
	void AddRasterTileToQueue(RasterTileIndex index);
	void AddTile3DToQueue(Tile3DIndex index);

	template <typename index_type, typename data_type, typename render_type>
	void AddTiles(cgv::render::context& ctx, std::map<index_type, data_type>& queue_tiles,
				  std::map<index_type, render_type>& cache_tiles, std::map<index_type, render_type>& active_tiles,
				  std::mutex& queue_lock, std::mutex& cache_lock);
	void AddRasterTiles(cgv::render::context& ctx);
	void AddTile3D(cgv::render::context& ctx);

	inline float GetZoom() const
	{
		return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(cam_lat)) /
										  (std::max(1.0, altitude)))); /*Circumference of earth = 40075016*/
	}
	inline float GetZoomAtAltitude(float alt) const
	{
		return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(cam_lat)) /
										  (std::max(1.0f, alt)))); /*Circumference of earth = 40075016*/
	}

  private:
	double cam_lat, cam_lon, altitude;
	GlobalConfig* config;
	TileManagerData tile_manager_data;

	std::set<RasterTileIndex> neighbour_set_raster_tile;
	std::set<Tile3DIndex> neighbour_set_tile3D;
	std::set<RasterTileIndex> requested_raster_tile;
	std::set<Tile3DIndex> requested_tile3D;
	std::map<RasterTileIndex, RasterTileRender> active_raster_tile;
	std::map<Tile3DIndex, Tile3DRender> active_tile3D;
	std::map<RasterTileIndex, RasterTileData&> queue_raster_tiles;
	std::map<Tile3DIndex, Tile3DData&> queue_tile3Ds;

	mutable std::mutex m_MutexRequestRasterTiles;
	mutable std::mutex m_MutexRequestTile3Ds;

	mutable std::mutex m_MutexQueueRasterTiles;
	mutable std::mutex m_MutexQueueTile3Ds;

	mutable std::mutex mutex_cache_raster_tile;
	mutable std::mutex mutex_cache_tile3D;

	std::array<cgv::math::fvec<double, 4>, 6> frustum_planes;
	cgv::math::fvec<double, 3> frustum_bbox_min, frustum_bbox_max;
	double frustum_min_lat, frustum_max_lat, frustum_min_lon, frustum_max_lon;
	std::array<double, 2> min_extent, max_extent;

	std::map<RasterTileIndex, RasterTileRender> raster_tile_cache;
	std::map<Tile3DIndex, Tile3DRender> tile3D_cache;
};
