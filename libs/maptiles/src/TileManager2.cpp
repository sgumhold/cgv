#include "TileManager2.h"
#include "utils.h"

TileManager2::TileManager2() : lat(0), lon(0), altitude(0), config(nullptr) {}

void TileManager2::Init(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	lat = _lat;
	lon = _lon;
	altitude = _altitude;
	config = _conf;
	tile_manager_data.Init(config);
}

void TileManager2::ReInit(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	lat = _lat;
	lon = _lon;
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

	GenerateRasterTileNeighbours();
	GenerateTile3DNeighbours();
	RemoveRasterTiles();
	RemoveTile3Ds();
	PruneNeighbourSetRasterTile();
	PruneNeighbourSetTile3D();
	GetRasterTileNeighbours();
	GetTile3DNeighbours();
}

void TileManager2::SetPosition(double _lat, double _lon, double _alt)
{
	lat = _lat;
	lon = _lon;
	altitude = _alt;
}

void TileManager2::GenerateRasterTileNeighbours()
{
	int k = config->NeighbourhoodFetchSizeRasterTile;

	int centerX;
	int centerY;

	int zoom = (int)GetZoom();

	centerX = long2tilex(lon, zoom);
	centerY = lat2tiley(lat, zoom);
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

	double centerLat = std::floor(lat / size) * size;
	double centerLon = std::floor(lon / size) * size;

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

