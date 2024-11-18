#pragma once

#include "Config.h"

#include "RasterTileRender.h"
#include "Tile3DRender.h"

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>


class MAPTILES_API TileRenderer
{
  private:
	cgv::render::shader_program shader_tile3D, shader_raster_tile;
  public:
	TileRenderer();
	~TileRenderer();

	void Init(cgv::render::context& ctx, std::string shader_3D = "maptiles.glpr", std::string shader_raster = "maptiles_textured.glpr");
	void Draw(cgv::render::context& ctx, RasterTileRender& tile);
	void Draw(cgv::render::context& ctx, Tile3DRender& tile);
	void Finalize(cgv::render::context& ctx);
};
