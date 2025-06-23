#pragma once

#include "library.h"

#include <glm/glm.hpp>
#include <vector>

#include <cgv/math/fvec.h>
#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>

#include "Texture.h"

class CESIUM_TILES_API CesiumElevationTileData
{
  public:
	std::vector<glm::dvec3> _positions;
	std::vector<glm::dvec3> _normals;
	std::vector<glm::dvec2> _texCoords;
};

class CESIUM_TILES_API CesiumElevationTileRender
{
	typedef cgv::math::fvec<float, 3> vec3;
	typedef cgv::math::fvec<float, 2> vec2;

	struct Vertex
	{
		vec3 position;
		vec3 normal;
		vec2 texCoord;
	};

  public:
	CesiumElevationTileRender(cgv::render::context& ctx, const CesiumElevationTileData& data,
							  cgv::render::shader_program& shader);

	void SetTexture(const unsigned char*, int width, int height);

	size_t _count = 0;
	std::shared_ptr<cgv::render::vertex_buffer> _vertex_buffer;
	std::shared_ptr<cgv::render::attribute_array_binding> _vertex_array;
	std::shared_ptr<CesiumTiles::Texture> _texture;
};
