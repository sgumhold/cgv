#pragma once

#include <cgv/math/fvec.h>
#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>

#include "Tile3DData.h"

class MAPTILES_API Tile3DRender
{
	typedef cgv::math::fvec<float, 3> vec3;
	typedef cgv::math::fvec<float, 4> vec4;
	
	struct Vertex
	{
		vec3 position;
		vec4 color;
		vec3 normal;
	};

public:
	std::vector<Vertex> mesh;
	float lat_min, lon_min, lat_max, lon_max;
	
	cgv::render::vertex_buffer vertex_buffer;
	cgv::render::attribute_array_binding vertex_array;

	uint32_t count = 0;

	bool success = true;
 public:
	Tile3DRender();
	Tile3DRender(cgv::render::context& ctx, const Tile3DData& tile, double lat, double lon);
};