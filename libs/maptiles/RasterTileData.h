#pragma once

#include "Config.h"

// Class to store all data related to a Raster Tile
class MAPTILES_API RasterTileData
{
public:
	RasterTileData(unsigned char* image, int height, int width, int zoom, int x, int y);
	RasterTileData(RasterTileData&& other) noexcept;
	~RasterTileData();

public:
	bool valid;
	int m_zoom;
	int m_x, m_y;
	int m_height, m_width;
	unsigned char* m_image;
};
