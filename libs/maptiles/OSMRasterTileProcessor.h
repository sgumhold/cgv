#pragma once

#include "OSMRasterTileLoader.h"

// Class to process the raw response from the OSMRasterTileLoader class
class MAPTILES_API OSMRasterTileProcessor
{
public:
	OSMRasterTileProcessor(OSMRasterTileLoader& loader);

	inline unsigned char* GetImage() { return m_image; }
	inline int GetHeight() { return m_height; }
	inline int GetWidth() { return m_width; }
	inline int GetSize() { return m_size; }

private:
	unsigned char* m_image;
	int m_height, m_width, m_nchannels, m_size;
};
