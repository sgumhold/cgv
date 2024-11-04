#include "TileManager2.h"
#include "utils.h"
#include "WGS84toCartesian.hpp"

TileManager2::TileManager2() : cam_lat(0), cam_lon(0), altitude(0), config(nullptr) {}

void TileManager2::Init(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _altitude;
	config = _conf;
	tile_manager_data.Init(config);
}

void TileManager2::ReInit(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _altitude;
	config = _conf;

	active_raster_tile.clear();
	active_tile3D.clear();
}

void TileManager2::Finalize()
{
	// active_raster_tiles.clear();
	// neighbour_set_raster_tile.clear();
}

void TileManager2::Update(cgv::render::context& ctx)
{
	AddRasterTiles(ctx);
	AddTile3D(ctx);

	//GenerateRasterTileNeighbours();
	//GenerateTile3DNeighbours();
	GenerateRasterTileFrustumNeighbours();
	GenerateTile3DFrustumNeighbours();
	RemoveRasterTiles();
	RemoveTile3Ds();
	PruneNeighbourSetRasterTile();
	PruneNeighbourSetTile3D();
	GetRasterTileNeighbours();
	GetTile3DNeighbours();
}

void TileManager2::SetPosition(double _lat, double _lon, double _alt)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _alt;
}

void TileManager2::GenerateRasterTileFrustumNeighbours() 
{
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
		if (i < 0 || i >= nTiles || std::abs(i - camX) > config->FrustumRasterTilesCount)
			continue;
		for (int j = minY; j <= maxY; j++) {
			// check if we are out of bounds
			if (j < 0 || j >= nTiles || std::abs(j - camY) > config->FrustumRasterTilesCount)
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
			tileMin[1] = min[1];
			tileMin[2] = 0.0f;

			tileMax[0] = max[0];
			tileMax[1] = max[1];
			tileMax[2] = 1.0f;

			for (int i = 0; i < 6; i++) {
				if (IsBoxCompletelyBehindPlane(tileMin, tileMax, frustum_planes[i])) {
					isVisible = false;
					break;
				}
			}

			if (isVisible) {
				RasterTileIndex index = {zoom, i, j};
				neighbour_set_raster_tile.insert(index);
				count++;
			}

		}
	}
	std::cout << "Raster Tile Count: " << count << std::endl;
}

void TileManager2::GenerateTile3DFrustumNeighbours() 
{
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
			tileMin[1] = min[1];
			tileMin[2] = 0.0f;

			tileMax[0] = max[0];
			tileMax[1] = max[1];
			tileMax[2] = 100.0f;

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
				// hack to get around floating point precision issues
				lat = (double)llround(lat * (1/size)) * size;
				lon = (double)llround(lon * (1/size)) * size;
				
				Tile3DIndex index = {lat, lon};
				neighbour_set_tile3D.insert(index);
				count++;
			}
		}
	}
	std::cout << "3D Tile Count: " << count << std::endl;
}

void TileManager2::GenerateRasterTileNeighbours()
{
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

			RasterTileIndex index = {zoom, i + centerX, j + centerY};
			neighbour_set_raster_tile.insert(index);
		}
	}
}

void TileManager2::GenerateTile3DNeighbours()
{
	neighbour_set_tile3D.clear();

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

			Tile3DIndex index = {_lat, _lon};
			neighbour_set_tile3D.insert(index);
		}
	}
}

void TileManager2::RemoveRasterTiles()
{
	int gridSize = config->NeighbourhoodFetchSizeRasterTile * 2 + 1;

	// Create a vector that will store the indices that need to be removed
	std::vector<RasterTileIndex> indices;
	indices.resize(gridSize * gridSize);

	// Get the indices to be removed from the queue
	for (auto const& element : queue_raster_tiles) {
		if (neighbour_set_raster_tile.find(element.first) == neighbour_set_raster_tile.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the queue
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueRasterTiles);
	for (auto& index : indices) {
		queue_raster_tiles.erase(index);
	}

	// clear the indices
	indices.clear();

	// Get the indices to be removed from the background list
	for (auto const& element : active_raster_tile) {
		if (neighbour_set_raster_tile.find(element.first) == neighbour_set_raster_tile.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the background list
	std::lock_guard<std::mutex> lockActive(m_MutexActiveRasterTiles);
	for (auto& index : indices) {
		active_raster_tile.erase(index);
	}
}

void TileManager2::RemoveTile3Ds()
{
	int gridSize = config->NeighbourhoodFetchSizeTile3D * 2 + 1;

	// Create a vector that will store the indices that need to be removed
	std::vector<Tile3DIndex> indices;
	indices.resize(gridSize * gridSize);

	// Get the indices to be removed from the queue
	for (auto const& element : queue_tile3Ds) {
		if (neighbour_set_tile3D.find(element.first) == neighbour_set_tile3D.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the queue
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueTile3Ds);
	for (auto& index : indices) {
		queue_tile3Ds.erase(index);
	}

	// clear the indices
	indices.clear();

	// Get the indices to be removed from the background list
	for (auto const& element : active_tile3D) {
		if (neighbour_set_tile3D.find(element.first) == neighbour_set_tile3D.end()) {
			indices.push_back(element.first);
			std::cout << "Removing Tile: (" << element.first.lat << ", " << element.first.lon << ")\n";
		}
	}

	// Remove Raster Tiles from the background list
	std::lock_guard<std::mutex> lockActive(m_MutexActiveTile3Ds);
	for (auto& index : indices) {
		active_tile3D.erase(index);
	}
}

void TileManager2::PruneNeighbourSetRasterTile()
{
	std::vector<RasterTileIndex> indices;

	for (const auto& element : neighbour_set_raster_tile) {
		if (queue_raster_tiles.find(element) != queue_raster_tiles.end()) {
			indices.push_back(element);
		}
		else if (active_raster_tile.find(element) != active_raster_tile.end()) {
			indices.push_back(element);
		}
		else if (requested_raster_tile.find(element) != requested_raster_tile.end())
		{
			indices.push_back(element);
		}
	}

	// Remove already present tiles from the neighbour set
	for (auto& index : indices) {
		neighbour_set_raster_tile.erase(index);
	}
}

void TileManager2::PruneNeighbourSetTile3D()
{
	std::vector<Tile3DIndex> indices;

	for (const auto& element : neighbour_set_tile3D) {
		if (queue_tile3Ds.find(element) != queue_tile3Ds.end()) {
			indices.push_back(element);
		}
		else if (active_tile3D.find(element) != active_tile3D.end()) {
			indices.push_back(element);
		}
		else if (requested_tile3D.find(element) != requested_tile3D.end()) {
			indices.push_back(element);
		}
	}

	// Remove already present tiles from the neighbour set
	for (auto& index : indices) {
		neighbour_set_tile3D.erase(index);
	}
}

void TileManager2::GetRasterTileNeighbours()
{
	for (const RasterTileIndex& itr : neighbour_set_raster_tile) {
		const RasterTileIndex& index = itr;

		std::thread t(&TileManager2::AddRasterTileToQueue, this, index);
		t.detach();
		requested_raster_tile.insert(index);
	}

	// Once the request is made, remove the index from the set
	neighbour_set_raster_tile.clear();
}

void TileManager2::GetTile3DNeighbours()
{
	for (const Tile3DIndex& itr : neighbour_set_tile3D) {
		const Tile3DIndex& index = itr;

		std::thread t(&TileManager2::AddTile3DToQueue, this, index);
		t.detach();
		requested_tile3D.insert(index);
	}

	// Once the request is made, remove the index from the set
	neighbour_set_tile3D.clear();
}

void TileManager2::AddRasterTileToQueue(RasterTileIndex index)
{
	RasterTileData tileData = tile_manager_data.GetRasterTile(index.zoom, index.x, index.y);
	std::lock_guard<std::mutex> lock(m_MutexQueueRasterTiles);
	queue_raster_tiles[index] = tileData;
	requested_raster_tile.erase(index);
	
	// Send the signal that the tile was downloaded
	tile_downloaded();
}

void TileManager2::AddTile3DToQueue(Tile3DIndex index)
{
	Tile3DData tileData = tile_manager_data.GetTile3D(index.lat, index.lon);
	std::lock_guard<std::mutex> lock(m_MutexQueueTile3Ds);
	queue_tile3Ds[index] = tileData;
	requested_tile3D.erase(index);

	// Send the signal that the tile was downloaded
	tile_downloaded();
}

void TileManager2::AddRasterTiles(cgv::render::context& ctx)
{
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueRasterTiles);
	if (queue_raster_tiles.empty()) {
		return;
	}

	std::lock_guard<std::mutex> lockActive(m_MutexActiveRasterTiles);
	while (!queue_raster_tiles.empty()) {
		auto& element = *(queue_raster_tiles.begin());
		RasterTileRender tile(ctx, element.second, config->ReferencePoint.lat, config->ReferencePoint.lon);

		active_raster_tile[element.first] = tile;

		queue_raster_tiles.erase(element.first);
	}
}

void TileManager2::AddTile3D(cgv::render::context& ctx)
{
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueTile3Ds);
	if (queue_tile3Ds.empty()) {
		return;
	}

	std::lock_guard<std::mutex> lockActive(m_MutexActiveTile3Ds);
	while (!queue_tile3Ds.empty()) {
		auto const& element = *(queue_tile3Ds.begin());
		Tile3DRender tile(ctx, element.second, config->ReferencePoint.lat, config->ReferencePoint.lon);

		active_tile3D[element.first] = tile;

		queue_tile3Ds.erase(element.first);
	}
}

bool TileManager2::IsBoxCompletelyBehindPlane(const cgv::math::fvec<float, 3>& boxMin, const cgv::math::fvec<float, 3>& boxMax,
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

void TileManager2::CalculateViewFrustum(const cgv::mat4& mvp) 
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
		  {config->ReferencePoint.lat, config->ReferencePoint.lon}, {frustum_bbox_min[0], frustum_bbox_min[1]});

	std::array<double, 2> frustum_max = wgs84::fromCartesian({config->ReferencePoint.lat, config->ReferencePoint.lon},
															 {frustum_bbox_max[0], frustum_bbox_max[1]});

	double& size = config->Tile3DSize;
	frustum_min_lat = std::floor(frustum_min[0] / size) * size;
	frustum_min_lon = std::floor(frustum_min[1] / size) * size;
	frustum_max_lat = std::ceil(frustum_max[0] / size) * size;
	frustum_max_lon = std::ceil(frustum_max[1] / size) * size;

}

