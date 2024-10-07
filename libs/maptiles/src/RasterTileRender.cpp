#include "RasterTileRender.h"

#include "utils.h"
#include "WGS84toCartesian.hpp"


RasterTileRender::RasterTileRender(cgv::render::context& ctx, RasterTileData& tile, double ref_lat, double ref_lon) 
{
	// Calculate the bounding quad
	double top = tiley2lat(tile.m_y, tile.m_zoom);
	double bottom = tiley2lat(tile.m_y + 1, tile.m_zoom);
	double left = tilex2long(tile.m_x, tile.m_zoom);
	double right = tilex2long(tile.m_x + 1, tile.m_zoom);

	std::array<double, 2> bottomLeftCartesian = wgs84::toCartesian({ref_lat, ref_lon}, {bottom, left});
	std::array<double, 2> topRightCartesian = wgs84::toCartesian({ref_lat, ref_lon}, {top, right});

	std::vector<Vertex> vertices;
	Vertex V;

	V.position = vec3(bottomLeftCartesian[0], (float)bottomLeftCartesian[1], 0);
	V.texcoord = vec2(0.0f, 0.0f);
	vertices.push_back(V);

	V.position = vec3(topRightCartesian[0], (float)bottomLeftCartesian[1], 0);
	V.texcoord = vec2(1.0f, 0.0f);
	vertices.push_back(V);

	V.position = vec3(topRightCartesian[0], (float)topRightCartesian[1], 0);
	V.texcoord = vec2(1.0f, 1.0f);
	vertices.push_back(V);

	V.position = vec3(bottomLeftCartesian[0], (float)bottomLeftCartesian[1], 0);
	V.texcoord = vec2(0.0f, 0.0f);
	vertices.push_back(V);

	V.position = vec3(topRightCartesian[0], (float)topRightCartesian[1], 0);
	V.texcoord = vec2(1.0f, 1.0f);
	vertices.push_back(V);

	V.position = vec3(bottomLeftCartesian[0], (float)topRightCartesian[1], 0);
	V.texcoord = vec2(0.0f, 1.0f);
	vertices.push_back(V);

	vertex_buffer.create(ctx, &(vertices[0]), vertices.size());

	cgv::render::shader_program my_shader;
	my_shader.build_program(ctx, "maptiles_textured.glpr");
	my_shader.specify_standard_uniforms(true, false, false, true);
	my_shader.specify_standard_vertex_attribute_names(ctx, false, false, true);
	my_shader.allow_context_to_set_color(true);

	cgv::render::type_descriptor vec3type =
		  cgv::render::element_descriptor_traits<vec3>::get_type_descriptor(vertices[0].position);
	cgv::render::type_descriptor vec2type =
		  cgv::render::element_descriptor_traits<vec2>::get_type_descriptor(vertices[0].texcoord);

	vertex_array.create(ctx);

	vertex_array.set_attribute_array(ctx, my_shader.get_position_index(), vec3type, vertex_buffer, 0,
									 vertices.size(), sizeof(Vertex));

	vertex_array.set_attribute_array(ctx, my_shader.get_texcoord_index(), vec2type, vertex_buffer, sizeof(vec3),
									 vertices.size(), sizeof(Vertex));

	//texture.create_from_image(ctx, "C:\\Users\\shali\\Desktop\\2770.png");
	tmp_texture = std::make_shared<MapTiles::Texture>(tile.m_image, tile.m_height, tile.m_width);
}
