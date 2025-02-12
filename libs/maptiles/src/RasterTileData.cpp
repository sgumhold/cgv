#include "RasterTileData.h"
#include "utils.h"

RasterTileData::RasterTileData(unsigned char* image, int height, int width, int zoom, int x, int y)
	: m_zoom(zoom), m_x(x), m_y(y), m_height(height), m_width(width), m_image(image), valid(true)
{
}

RasterTileData::RasterTileData(RasterTileData&& other) noexcept 
{
	m_zoom = other.m_zoom;
	m_x = other.m_x;
	m_y = other.m_y;
	m_height = other.m_height;
	m_width = other.m_width;
	m_image = other.m_image;
	valid = other.valid;
	other.valid = false;
	other.m_image = nullptr;
}

RasterTileData::~RasterTileData() 
{
	if (m_image != nullptr) {
		delete[] m_image;
		m_image = nullptr;
	}
}
