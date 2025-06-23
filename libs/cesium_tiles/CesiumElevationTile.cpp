#include "CesiumElevationTile.h"

CesiumElevationTileRender::CesiumElevationTileRender(cgv::render::context& ctx, const CesiumElevationTileData& data, cgv::render::shader_program& shader) 
{
	std::vector<Vertex> vertices;

	for (int i = 0; i < data._positions.size(); ++i) {
		Vertex vertex;
		vertex.position = vec3(data._positions[i].x, data._positions[i].y, data._positions[i].z);
		vertex.normal = vec3(data._normals[i].x, data._normals[i].y, data._normals[i].z);
		vertex.texCoord = vec2(data._texCoords[i].x, data._texCoords[i].y);
		vertices.push_back(vertex);
	}

	_count = vertices.size();

	_vertex_buffer = std::make_shared<cgv::render::vertex_buffer>();
	_vertex_buffer->create(ctx, &(vertices[0]), vertices.size());

	_vertex_array = std::make_shared<cgv::render::attribute_array_binding>();
	_vertex_array->create(ctx);

	cgv::render::type_descriptor vec3type =
		  cgv::render::element_descriptor_traits<vec3>::get_type_descriptor(vertices[0].position);
	cgv::render::type_descriptor vec2type =
		  cgv::render::element_descriptor_traits<vec2>::get_type_descriptor(vertices[0].texCoord);

	_vertex_array->set_attribute_array(ctx, shader.get_position_index(), vec3type, *_vertex_buffer.get(), 
		0, vertices.size(), sizeof(Vertex));
	_vertex_array->set_attribute_array(ctx, shader.get_normal_index(), vec3type, *_vertex_buffer.get(), 
		sizeof(vec3type), vertices.size(), sizeof(Vertex));
	_vertex_array->set_attribute_array(ctx, shader.get_texcoord_index(), vec2type, *_vertex_buffer.get(),
									   sizeof(vec3type) * 2, vertices.size(), sizeof(Vertex));

}

void CesiumElevationTileRender::SetTexture(const unsigned char* image, int width, int height) 
{
	_texture = std::make_shared<CesiumTiles::Texture>(image, height, width);
}
