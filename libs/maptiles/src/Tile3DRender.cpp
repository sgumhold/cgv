#include "Tile3DRender.h"

Tile3DRender::Tile3DRender() :lat_min(0), lat_max(0), lon_min(0), lon_max(0) {}

Tile3DRender::Tile3DRender(cgv::render::context& ctx, const Tile3DData& tile, double lat, double lon)
{
	lat_min = tile.lat_min;
	lon_min = tile.lon_min;
	lat_max = tile.lat_max;
	lon_max = tile.lon_max;

	for (int i = 0; i < tile.mesh_cartesian.size(); i += 3) {
		Vertex v;
		v.position	= vec3(tile.mesh_cartesian[i][0],			tile.mesh_cartesian[i][1],		tile.mesh_cartesian[i][2]);
		v.color		= vec4(tile.mesh_cartesian[i + 1][0],		tile.mesh_cartesian[i + 1][1],	tile.mesh_cartesian[i + 1][2],	1.0f);
		v.normal	= vec3(tile.mesh_cartesian[i + 2][0],		tile.mesh_cartesian[i + 2][1],	tile.mesh_cartesian[i + 2][2]);
		mesh.push_back(v);
	}

	count = mesh.size();

	success = vertex_buffer.create(ctx, &(mesh[0]), mesh.size()) && success;

	success = vertex_array.create(ctx) && success;

	cgv::render::type_descriptor vec3type =
		  cgv::render::element_descriptor_traits<vec3>::get_type_descriptor(mesh[0].position);
	cgv::render::type_descriptor vec4type =
		  cgv::render::element_descriptor_traits<vec4>::get_type_descriptor(mesh[0].color);
	
	
	cgv::render::shader_program my_shader;
	my_shader.build_program(ctx, "maptiles.glpr");
	my_shader.specify_standard_uniforms(true, false, false, true);
	my_shader.specify_standard_vertex_attribute_names(ctx, true, true, false);
	my_shader.allow_context_to_set_color(true);


	success = vertex_array.set_attribute_array(ctx, my_shader.get_position_index(), vec3type, vertex_buffer, 0,
											   mesh.size(), sizeof(Vertex)) &&
			  success;

	success = vertex_array.set_attribute_array(ctx, my_shader.get_color_index(), vec4type, vertex_buffer,
											   sizeof(vec3), mesh.size(), sizeof(Vertex)) &&
			  success;

	success = vertex_array.set_attribute_array(ctx, my_shader.get_normal_index(), vec3type, vertex_buffer,
											   sizeof(vec3) + sizeof(vec4),
											   mesh.size(), sizeof(Vertex)) &&
			  success;
	
	/*
	cgv::render::shader_program& default_shader = ctx.ref_default_shader_program();
	

	success = vertex_array.create(ctx) && success;

	success = vertex_array.set_attribute_array(ctx, default_shader.get_position_index(), vec3type, vertex_buffer, 0,
											   mesh.size(), sizeof(Vertex)) &&
			  success;

	success = vertex_array.set_attribute_array(ctx, default_shader.get_color_index(), vec4type, vertex_buffer,
											   sizeof(vec3), mesh.size(), sizeof(Vertex)) &&
			  success;
	
	
	success = vertex_array.set_attribute_array(ctx, default_shader.get_normal_index(), vec3type, vertex_buffer,
											   sizeof(vec3) + sizeof(vec4), mesh.size(), sizeof(Vertex)) &&
			  success;
	*/
}

