#include "TileRenderer.h"

#include <iostream>

TileRenderer::TileRenderer() {}

TileRenderer::~TileRenderer() {}

void TileRenderer::Init(cgv::render::context& ctx, std::string shader_3D, std::string shader_raster)
{
	shader_raster_tile.build_program(ctx, shader_raster);
	shader_raster_tile.specify_standard_uniforms(true, false, false, true);
	shader_raster_tile.specify_standard_vertex_attribute_names(ctx, false, false, true);
	shader_raster_tile.allow_context_to_set_color(true);

	shader_tile3D.build_program(ctx, shader_3D);
	shader_tile3D.specify_standard_uniforms(true, false, false, true);
	shader_tile3D.specify_standard_vertex_attribute_names(ctx, true, true, false);
	shader_tile3D.allow_context_to_set_color(true);
}

void TileRenderer::Draw(cgv::render::context& ctx, RasterTileRender& tile)
{
	shader_raster_tile.enable(ctx);
	ctx.set_color(cgv::rgba(0.5, 0.5, 0.5, 1.0));

	tile.vertex_array.enable(ctx);
	tile.tmp_texture->Bind();
	//tile.texture.enable(ctx, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	//tile.texture.disable(ctx);
	tile.tmp_texture->Unbind();
	tile.vertex_array.disable(ctx);

	shader_raster_tile.disable(ctx);
}

void TileRenderer::Draw(cgv::render::context& ctx, Tile3DRender& tile)
{
	shader_tile3D.enable(ctx);
	ctx.set_color(cgv::rgba(0.5, 0.5, 0.5, 1.0));

	tile.vertex_array.enable(ctx);

	glDrawArrays(GL_TRIANGLES, 0, tile.count);

	tile.vertex_array.disable(ctx);
	shader_tile3D.disable(ctx);
}

void TileRenderer::Finalize(cgv::render::context& ctx) 
{ 
	shader_raster_tile.destruct(ctx);
	shader_tile3D.destruct(ctx);
}
