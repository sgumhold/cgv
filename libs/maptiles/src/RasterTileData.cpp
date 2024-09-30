#include "RasterTileData.h"
#include "utils.h"

RasterTileData::RasterTileData()
	: m_zoom(0), m_x(0), m_y(0), m_height(0), m_width(0), m_image(nullptr)
{
}

RasterTileData::RasterTileData(unsigned char* image, int height, int width, int zoom, int x, int y)
	: m_zoom(zoom), m_x(x), m_y(y), m_height(height), m_width(width), m_image(image)
{
}
