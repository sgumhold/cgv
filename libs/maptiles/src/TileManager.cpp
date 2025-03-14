
#include <future>

#include "TileManager.h"
#include "utils.h"
#include "WGS84toCartesian.hpp"

TileManager::TileManager() :	cam_lat(0), cam_lon(0), altitude(0), config(nullptr), 
								frustum_min_lat(0), frustum_max_lat(0), frustum_min_lon(0), 
								frustum_max_lon(0), min_extent({0, 0}), max_extent({0, 0})
{
}

TileManager::~TileManager() 
{ 
	config = nullptr;
}

void TileManager::Init(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _altitude;
	config = _conf;
	tile_manager_data.Init(config);
}

void TileManager::ReInit(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _altitude;
	config = _conf;

	active_raster_tile.clear();
	active_tile3D.clear();
}

void TileManager::Finalize()
{
	// active_raster_tiles.clear();
	// neighbour_set_raster_tile.clear();
}

std::map<Tile3DIndex, Tile3DRender>& TileManager::GetActiveTile3Ds()
{ 
	return active_tile3D;
}

std::map<RasterTileIndex, RasterTileRender>& TileManager::GetActiveRasterTiles()
{
	return active_raster_tile;
}

void TileManager::Update(cgv::render::context& ctx)
{
	AddRasterTiles(ctx);
	AddTile3D(ctx);

	min_extent = {90, 180};
	max_extent = {-90, -180};

	if (config->FrustumBasedTileGeneration)
	{
		GenerateRasterTileFrustumNeighbours();
		GenerateTile3DFrustumNeighbours();
	}
	else
	{
		GenerateRasterTileNeighbours();
		GenerateTile3DNeighbours();
	}

	RemoveRasterTiles();
	RemoveTile3Ds();
	PruneNeighbourSetRasterTile();
	PruneNeighbourSetTile3D();
	GetRasterTileNeighbours();
	GetTile3DNeighbours();
}

void TileManager::SetPosition(double _lat, double _lon, double _alt)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _alt;
}

std::pair<std::array<double, 2>, std::array<double, 2>> TileManager::GetExtent()
{
	return std::pair<std::array<double, 2>, std::array<double, 2>>(min_extent, max_extent);
}

void TileManager::GenerateRasterTileFrustumNeighbours() 
{
	// we need to clear the set of neighbours so that we don't create requests for tiles
	// that were previously in the set but are no longer in the frustum
	neighbour_set_raster_tile.clear();

	int zoom = (int)GetZoom();

	int camX = long2tilex(cam_lon, zoom);
	int camY = lat2tiley(cam_lat, zoom);

	int minX = long2tilex(frustum_min_lon, zoom);
	int maxY = lat2tiley(frustum_min_lat, zoom);
	int maxX = long2tilex(frustum_max_lon, zoom);
	int minY = lat2tiley(frustum_max_lat, zoom);

	// Number of tiles at the current zoom level
	int nTiles = std::exp2(zoom);

	int count = 0;

	for (int i = minX; i <= maxX; i++) {
		// check if we are out of bounds
		//if (i < 0 || i >= nTiles || std::abs(tilex2long(i, zoom) - cam_lon) > config->FrustumRasterTilesDistance)
		if (i < 0 || i >= nTiles || (std::abs(tilex2long(i, zoom) - cam_lon) > config->FrustumRasterTilesDistance && std::abs(i - camX) > config->FrustumRasterTilesCount))
			continue;
		for (int j = minY; j <= maxY; j++) {
			// check if we are out of bounds
			//if (j < 0 || j >= nTiles || std::abs(tiley2lat(j, zoom) - cam_lat) > config->FrustumRasterTilesDistance)
			if (j < 0 || j >= nTiles || (std::abs(tiley2lat(j, zoom) - cam_lat) > config->FrustumRasterTilesDistance && std::abs(j - camY) > config->FrustumRasterTilesCount))
				continue;

			bool isVisible = true;
			cgv::math::fvec<float, 3> tileMin, tileMax;

			double lat_min = tiley2lat(j, zoom);
			double lat_max = tiley2lat(j + 1, zoom);
			double lon_min = tilex2long(i, zoom);
			double lon_max = tilex2long(i + 1, zoom);

			std::array<double, 2> min =
				  wgs84::toCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon}, {lat_min, lon_min});
			std::array<double, 2> max = wgs84::toCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon},
														   {lat_max, lon_max});

			tileMin[0] = min[0];
			tileMin[1] = 0.0f;
			tileMin[2] = -min[1];

			tileMax[0] = max[0];
			tileMax[1] = 1.0f;
			tileMax[2] = -max[1];

			for (int i = 0; i < 6; i++) {
				if (IsBoxCompletelyBehindPlane(tileMin, tileMax, frustum_planes[i])) {
					isVisible = false;
					break;
				}
			}

			if (isVisible) {
				if (lat_min < min_extent[0])
					min_extent[0] = lat_min;
				if (lat_max > max_extent[0])
					max_extent[0] = lat_max;
				if (lon_min < min_extent[1])
					min_extent[1] = lon_min;
				if (lon_max > max_extent[1])
					max_extent[1] = lon_max;

				RasterTileIndex index = {static_cast<unsigned>(zoom), i, j};
				neighbour_set_raster_tile.insert(index);
				count++;
			}

		}
	}
	//std::cout << "Raster Tile Count: " << count << std::endl;
}

void TileManager::GenerateTile3DFrustumNeighbours() 
{
	// we need to clear the set of neighbours so that we don't create requests for tiles
	// that were previously in the set but are no longer in the frustum
	neighbour_set_tile3D.clear();
	
	if (config->MaxTile3DRenderAltitude < altitude)
		return;

	double size = config->Tile3DSize;

	int count = 0;

	for (double lat = frustum_min_lat; lat <= frustum_max_lat; lat+=size) {
		// check if we are out of bounds
		if (lat < -89 || lat >= 89 || std::abs(lat - cam_lat) > config->FrustumTile3DMaxDistance)
			continue;
		for (double lon = frustum_min_lon; lon <= frustum_max_lon; lon+=size) {
			if (lon < -180 || lon >= 180 || std::abs(lon - cam_lon) > config->FrustumTile3DMaxDistance)
				continue;

			bool isVisible = true;
			cgv::math::fvec<float, 3> tileMin, tileMax;

			std::array<double, 2> min = wgs84::toCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon}, {lat, lon});
			std::array<double, 2> max = wgs84::toCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon}, {lat + size, lon + size});
			
			tileMin[0] = min[0];
			tileMin[1] = 0.0f;
			tileMin[2] = -min[1];

			tileMax[0] = max[0];
			tileMax[1] = 100.0f;
			tileMax[2] = -max[1];

			for (int i = 0; i < 6; i++)
			{
				if (IsBoxCompletelyBehindPlane(tileMin, tileMax, frustum_planes[i]))
				{
					isVisible = false;
					break;
				}
			}

			if (isVisible)
			{
				// hack to get around floating point precision issues for indexing tiles
				lat = (double)llround(lat * (1/size)) * size;
				lon = (double)llround(lon * (1/size)) * size;
				
				if (lat < min_extent[0])
					min_extent[0] = lat;
				if (lat + size > max_extent[0])
					max_extent[0] = lat + size;
				if (lon < min_extent[1])
					min_extent[1] = lon;
				if (lon + size > max_extent[1])
					max_extent[1] = lon + size;

				Tile3DIndex index = {lat, lon};
				neighbour_set_tile3D.insert(index);
				count++;
			}
		}
	}
	//std::cout << "3D Tile Count: " << count << std::endl;
}

void TileManager::GenerateRasterTileNeighbours()
{
	// we need to clear the set of neighbours so that we don't create requests for tiles
	// that were previously in the set but are no longer in the grid
	neighbour_set_raster_tile.clear();

	int k = config->NeighbourhoodFetchSizeRasterTile;

	int centerX;
	int centerY;

	int zoom = (int)GetZoom();

	centerX = long2tilex(cam_lon, zoom);
	centerY = lat2tiley(cam_lat, zoom);
	// Number of tiles at the current zoom level
	int nTiles = std::exp2(zoom);

	for (int i = -k; i <= k; i++) {
		// check if we are out of bounds
		if ((i + centerX) < 0 || (i + centerX) >= nTiles)
			continue;
		for (int j = -k; j <= k; j++) {
			// check if we are out of bounds
			if ((j + centerY) < 0 || (j + centerY) >= nTiles)
				continue;
			
			
			auto pos_min = wgs84::fromCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon},
												{tiley2lat(j + centerY, zoom), tilex2long(i + centerX, zoom)});
			auto pos_max = wgs84::fromCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon},
												{tiley2lat(j + centerY + 1, zoom), tilex2long(i + centerX + 1, zoom)});

			if (pos_min[0] < min_extent[0])
				min_extent[0] = pos_min[0];
			if (pos_max[0] > max_extent[0])
				max_extent[0] = pos_max[0];
			if (pos_min[1] < min_extent[1])
				min_extent[1] = pos_min[1];
			if (pos_max[1] > max_extent[1])
				max_extent[1] = pos_max[1];

			RasterTileIndex index = {static_cast<unsigned>(zoom), i + centerX, j + centerY};
			neighbour_set_raster_tile.insert(index);
		}
	}
}

void TileManager::GenerateTile3DNeighbours()
{
	// we need to clear the set of neighbours so that we don't create requests for tiles
	// that were previously in the set but are no longer in the grid
	neighbour_set_tile3D.clear();

	if (config->MaxTile3DRenderAltitude < altitude)
		return;

	int k = config->NeighbourhoodFetchSizeTile3D;
	double size = config->Tile3DSize;

	double centerLat = std::floor(cam_lat / size) * size;
	double centerLon = std::floor(cam_lon / size) * size;

	for (int i = -k; i <= k; i++) {
		// check if we are out of bounds
		if ((i * size + centerLat) < -89 || (i * size + centerLat) >= 89)
			continue;
		for (int j = -k; j <= k; j++) {
			if ((j * size + centerLon) < -180 || (j * size + centerLon) >= 180)
				continue;

			double _lat = i * size + centerLat;
			double _lon = j * size + centerLon;

			_lat = (double)llround(_lat * (1 / size)) * size;
			_lon = (double)llround(_lon * (1 / size)) * size;

			if (_lat < min_extent[0])
				min_extent[0] = _lat;
			if (_lat + size > max_extent[0])
				max_extent[0] = _lat + size;
			if (_lon < min_extent[1])
				min_extent[1] = _lon;
			if (_lon + size > max_extent[1])
				max_extent[1] = _lon + size;

			Tile3DIndex index = {_lat, _lon};
			neighbour_set_tile3D.insert(index);
		}
	}
}

template <typename index_type, typename render_type>
void TileManager::RemoveTiles(std::map<index_type, render_type>& active_tiles, std::set<index_type>& neighbour_set)
{
	int size = active_tiles.size();

	// Create a vector that will store the indices that need to be removed
	std::vector<index_type> indices;
	indices.resize(size);

	// Get the indices to be removed from the active list
	for (auto const& element : active_tiles) {
		if (neighbour_set.find(element.first) == neighbour_set.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the active list
	for (auto& index : indices) {
		active_tiles.erase(index);
	}
}

void TileManager::RemoveRasterTiles()
{ 
	RemoveTiles(active_raster_tile, neighbour_set_raster_tile);
}

void TileManager::RemoveTile3Ds() 
{
	RemoveTiles(active_tile3D, neighbour_set_tile3D);
}

template <typename index_type, typename data_type, typename render_type>
void TileManager::PruneNeighbourSet(std::map<index_type, render_type>& active_tiles,
									std::set<index_type>& neighbour_set, std::map<index_type, std::future<data_type&>>& requested_tiles)
{
	std::vector<index_type> indices;

	for (const auto& element : neighbour_set) {
		if (active_tiles.find(element) != active_tiles.end()) {
			indices.push_back(element);
		}
		else if (requested_tiles.find(element) != requested_tiles.end()) {
			indices.push_back(element);
		}
	}

	// Remove already present tiles from the neighbour set
	for (auto& index : indices) {
		neighbour_set.erase(index);
	}
}

void TileManager::PruneNeighbourSetRasterTile()
{
	PruneNeighbourSet(active_raster_tile, neighbour_set_raster_tile, requested_raster_tiles);
}

void TileManager::PruneNeighbourSetTile3D()
{
	PruneNeighbourSet(active_tile3D, neighbour_set_tile3D, requested_tile3Ds);
}

void TileManager::GetRasterTileNeighbours()
{
	std::set<RasterTileIndex> to_be_removed;

	for (const RasterTileIndex& itr : neighbour_set_raster_tile) {
		const RasterTileIndex& index = itr;

		if (raster_tile_cache.find(index) != raster_tile_cache.end()) {
			active_raster_tile[index] = raster_tile_cache[index];
			to_be_removed.insert(index);
		}
		else if (requested_raster_tiles.size() < config->MaxRasterTileRequestThreads) {
			requested_raster_tiles.emplace(index, std::async(std::launch::async, &TileManager::GetRasterTile, this, index));
			to_be_removed.insert(index);
		}
	}

	// Once the request is made, remove the index from the set
	for (const RasterTileIndex& index : to_be_removed) {
		neighbour_set_raster_tile.erase(index);
	}
}

void TileManager::GetTile3DNeighbours()
{
	std::set<Tile3DIndex> to_be_removed;

	for (const Tile3DIndex& itr : neighbour_set_tile3D) {
		const Tile3DIndex& index = itr;

		if (tile3D_cache.find(index) != tile3D_cache.end()) {
			active_tile3D[index] = tile3D_cache[index];
			to_be_removed.insert(index);
		}
		else if (requested_tile3Ds.size() < config->MaxTile3DRequestThreads) {
			requested_tile3Ds.emplace(index, std::async(std::launch::async, &TileManager::GetTile3D, this, index));
			to_be_removed.insert(index);
		}
	}

	// Once the request is made, remove the index from the set
	for (const Tile3DIndex& index : to_be_removed) {
		neighbour_set_tile3D.erase(index);
	}
}

RasterTileData& TileManager::GetRasterTile(RasterTileIndex index)
{
	RasterTileData& tileData = tile_manager_data.GetRasterTile(index.zoom, index.x, index.y);
	tile_downloaded();

	return tileData;
}

Tile3DData& TileManager::GetTile3D(Tile3DIndex index)
{
	Tile3DData& tileData = tile_manager_data.GetTile3D(index.lat, index.lon);
	tile_downloaded();

	return tileData;
}

template <typename index_type, typename data_type, typename render_type>
void TileManager::AddTiles(cgv::render::context& ctx, std::map<index_type, std::future<data_type&>>& requested_tiles,
						   std::map<index_type, render_type>& cache_tiles,
						   std::map<index_type, render_type>& active_tiles)
{
	std::vector<index_type> to_be_removed;

	for (auto& element : requested_tiles)
	{
		if (element.second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			auto& tile = element.second.get();
			if (tile.valid)
			{
				render_type render_tile(ctx, tile, config->ReferencePoint.lat, config->ReferencePoint.lon);
				//std::lock_guard<std::mutex> lock_cache(cache_lock);
				cache_tiles.emplace(element.first, render_tile);
		
				active_tiles[element.first] = render_tile;
			}
			to_be_removed.push_back(element.first);
		}
	}

	for (const auto& index : to_be_removed) {
		requested_tiles.erase(index);
	}
}

void TileManager::AddRasterTiles(cgv::render::context& ctx)
{
	AddTiles(ctx, requested_raster_tiles, raster_tile_cache, active_raster_tile);
}

void TileManager::AddTile3D(cgv::render::context& ctx)
{
	AddTiles(ctx, requested_tile3Ds, tile3D_cache, active_tile3D);
}

void TileManager::TrimRenderCache() 
{ 
	std::set<RasterTileIndex> to_be_removed_raster_tile;
	std::set<Tile3DIndex> to_be_removed_tile3D;

	for (const auto& element : raster_tile_cache)
	{
		// Get the position of the tile in degrees
		double lat = tiley2lat(element.first.y, element.first.zoom);
		double lon = tilex2long(element.first.x, element.first.zoom);

		if (std::abs(lat - cam_lat) > config->RasterTileRenderCacheTrimDistance || 
			std::abs(lon - cam_lon) > config->RasterTileRenderCacheTrimDistance)
			to_be_removed_raster_tile.insert(element.first);
	}

	for (const auto& element : tile3D_cache) {
		// Get the position of the tile in degrees
		double lat = element.first.lat;
		double lon = element.first.lon;
		if (std::abs(lat - cam_lat) > config->Tile3DRenderCacheTrimDistance ||
			std::abs(lon - cam_lon) > config->Tile3DRenderCacheTrimDistance)
			to_be_removed_tile3D.insert(element.first);
	}

	//std::lock_guard<std::mutex> lock_raster_cache(mutex_cache_raster_tile);
	//std::lock_guard<std::mutex> lock_tile3D_cache(mutex_cache_tile3D);
	
	for (const auto& index : to_be_removed_raster_tile)
		raster_tile_cache.erase(index);

	for (const auto& index : to_be_removed_tile3D)
		tile3D_cache.erase(index);
}

void TileManager::ClearDataCache() 
{ 
	tile_manager_data.ClearCache(); 
}

void TileManager::TrimDataCache() 
{ 
	tile_manager_data.TrimCache();
}

bool TileManager::IsBoxCompletelyBehindPlane(const cgv::math::fvec<float, 3>& boxMin, const cgv::math::fvec<float, 3>& boxMax,
								const cgv::math::fvec<float, 4>& plane)
{
	return cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMin.x(), boxMin.y(), boxMin.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMin.x(), boxMin.y(), boxMax.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMin.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMin.x(), boxMax.y(), boxMax.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMax.x(), boxMin.y(), boxMin.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMax.x(), boxMin.y(), boxMax.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMax.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		   cgv::math::dot(plane, cgv::math::fvec<float, 4>(boxMax.x(), boxMax.y(), boxMin.z(), 1)) < 0;
}

void TileManager::ClearRenderCache() 
{ 
	std::cout << "Clearing Render Cache\n";
	std::cout << "Raster Tile Cache Size: " << raster_tile_cache.size() << std::endl;
	std::cout << "Tile3D Cache Size: " << tile3D_cache.size() << std::endl;

	//std::lock_guard<std::mutex> lock_raster_cache(mutex_cache_raster_tile);
	//std::lock_guard<std::mutex> lock_tile3D_cache(mutex_cache_tile3D);
	raster_tile_cache.clear();
	tile3D_cache.clear();
}

void TileManager::CalculateViewFrustum(const cgv::mat4& mvp) 
{
	frustum_planes[0] = (mvp.row(3) + mvp.row(0));
	frustum_planes[1] = (mvp.row(3) - mvp.row(0));
	frustum_planes[2] = (mvp.row(3) + mvp.row(1));
	frustum_planes[3] = (mvp.row(3) - mvp.row(1));
	frustum_planes[4] = (mvp.row(3) + mvp.row(2));
	frustum_planes[5] = (mvp.row(3) - mvp.row(2));

	constexpr double double_min = std::numeric_limits<double>::min();
	constexpr double double_max = std::numeric_limits<double>::max();
	frustum_bbox_min.set(double_max, double_max, double_max);
	frustum_bbox_max.set(double_min, double_min, double_min);

	cgv::mat4 invMvp = inv(mvp);

	for (int x = -1; x <= 1; x += 2)
		for (int y = -1; y <= 1; y += 2)
			for (int z = -1; z <= 1; z += 2) 
			{
				cgv::math::fvec<double, 4> corner = invMvp * cgv::math::fvec<float, 4>((double)x, (double)y, (double)z, 1);
				corner /= corner.w();
				if (corner[0] < frustum_bbox_min[0])
					frustum_bbox_min[0] = corner[0];
				if (corner[0] > frustum_bbox_max[0])
					frustum_bbox_max[0] = corner[0];
				if (corner[1] < frustum_bbox_min[1])
					frustum_bbox_min[1] = corner[1];
				if (corner[1] > frustum_bbox_max[1])
					frustum_bbox_max[1] = corner[1];
				if (corner[2] < frustum_bbox_min[2])
					frustum_bbox_min[2] = corner[2];
				if (corner[2] > frustum_bbox_max[2])
					frustum_bbox_max[2] = corner[2];
			}

	std::array<double, 2> frustum_min = wgs84::fromCartesian(
		  {config->ReferencePoint.lat, config->ReferencePoint.lon}, {frustum_bbox_min[0], -frustum_bbox_max[2]});

	std::array<double, 2> frustum_max = wgs84::fromCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon},
															 {frustum_bbox_max[0], -frustum_bbox_min[2]});	  
	
	double& size = config->Tile3DSize;
	frustum_min_lat = std::floor(frustum_min[0] / size) * size;
	frustum_min_lon = std::floor(frustum_min[1] / size) * size;
	frustum_max_lat = std::ceil(frustum_max[0] / size) * size;
	frustum_max_lon = std::ceil(frustum_max[1] / size) * size;

}

