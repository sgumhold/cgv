#pragma once

#include "CesiumElevationTile.h"

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

class CESIUM_TILES_API TileRenderer
{
  public:
	void Draw(cgv::render::context& ctx, CesiumElevationTileRender& tile, cgv::render::shader_program& shader);
};