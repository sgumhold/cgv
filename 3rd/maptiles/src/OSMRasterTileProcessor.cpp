#include "OSMRasterTileProcessor.h"

#include <stb_image.h>

OSMRasterTileProcessor::OSMRasterTileProcessor(OSMRasterTileLoader& loader)
	: m_image(nullptr), m_height(0), m_width(0), m_nchannels(0), m_size(0)
{
	m_size = (unsigned)loader.GetResponse().size();
	unsigned char* response = (unsigned char*)loader.GetResponse().c_str();

	stbi_set_flip_vertically_on_load(1);
	m_image = stbi_load_from_memory(response, m_size, &m_width, &m_height, &m_nchannels, 4);
}
