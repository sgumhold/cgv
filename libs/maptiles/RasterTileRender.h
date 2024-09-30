#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/texture.h>
#include <cgv/render/shader_program.h>

#include "RasterTileData.h"

#include "Texture.h"

class RasterTileRender
{
	typedef cgv::math::fvec<float, 3> vec3;
	typedef cgv::math::fvec<float, 2> vec2;

	struct Vertex
	{
		vec3 position;
		vec2 texcoord;
	};

public:
	cgv::render::vertex_buffer vertex_buffer;
	cgv::render::attribute_array_binding vertex_array;
	cgv::render::texture texture;
	// TODO: Remove MapTiles::Texture. Use cgv::render::texture instead
	std::shared_ptr<MapTiles::Texture> tmp_texture;

public:
	RasterTileRender() {}
	RasterTileRender(cgv::render::context& ctx, RasterTileData& tile, double ref_lat, double ref_lon);
};