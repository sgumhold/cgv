#include "TileRenderer.h"

#include <iostream>

void TileRenderer::Init(cgv::render::context& ctx, std::string shader_3D, std::string shader_raster)
{
	shader_raster_tile.build_program(ctx, "maptiles_textured.glpr");
	shader_raster_tile.specify_standard_uniforms(true, false, false, true);
	shader_raster_tile.specify_standard_vertex_attribute_names(ctx, false, false, true);
	shader_raster_tile.allow_context_to_set_color(true);

	shader_tile3D.build_program(ctx, "maptiles.glpr");
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
/*
	bool success = true;
	cgv::render::shader_program my_shader;
	my_shader.build_program(ctx, "maptiles_textured.glpr");
	my_shader.specify_standard_uniforms(true, false, false, true);
	my_shader.specify_standard_vertex_attribute_names(ctx, false, false, true);
	my_shader.allow_context_to_set_color(true);

	success = my_shader.enable(ctx) && success;
	ctx.set_color(cgv::rgba(0.5, 0.5, 0.5, 1.0));

	success = tile.vertex_array.enable(ctx) && success;

	bool result = my_shader.set_uniform(ctx, "texture", 0);
	success = tile.texture.enable(ctx, 0) && success;

	glDrawArrays(GL_TRIANGLES, 0, 6);

	tile.texture.disable(ctx);
	tile.vertex_array.disable(ctx);

	my_shader.disable(ctx);
*/
}

void TileRenderer::Draw(cgv::render::context& ctx, Tile3DRender& tile, cgv::math::fvec<float, 3> camera_pos)
{
	//cgv::render::shader_program& default_shader = ctx.ref_default_shader_program();
	//default_shader.enable(ctx);

	/*
	cgv::render::shader_program my_shader;
	my_shader.build_program(ctx, "maptiles.glpr");
	my_shader.specify_standard_uniforms(true, false, false, true);
	my_shader.specify_standard_vertex_attribute_names(ctx, true, false, false);
	my_shader.allow_context_to_set_color(true);
	my_shader.enable(ctx);
	*/

	shader_tile3D.enable(ctx);

	shader_tile3D.set_uniform(ctx, "camera_pos", camera_pos, true);

	ctx.set_color(cgv::rgba(0.5, 0.5, 0.5, 1.0));

	tile.vertex_array.enable(ctx);

	glDrawArrays(GL_TRIANGLES, 0, tile.count);

	tile.vertex_array.disable(ctx);

	shader_tile3D.disable(ctx);
}
