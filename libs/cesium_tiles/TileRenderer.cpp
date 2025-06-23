#include "TileRenderer.h"

void TileRenderer::Draw(cgv::render::context& ctx, CesiumElevationTileRender& tile, cgv::render::shader_program& shader)
{
	shader.enable(ctx);
	ctx.set_color(cgv::rgba(0.5, 0.5, 0.5, 1.0));

	tile._vertex_array->enable(ctx);
	tile._texture->Bind();

	glDrawArrays(GL_TRIANGLES, 0, tile._count);

	tile._texture->Unbind();
	tile._vertex_array->disable(ctx);
	shader.disable(ctx);
}
