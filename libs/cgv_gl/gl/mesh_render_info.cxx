#include <memory>
#include "mesh_render_info.h"

namespace cgv {
	namespace render {
mesh_render_info::mesh_render_info() :
	position_descr(cgv::render::element_descriptor_traits<vec3>::get_type_descriptor(vec3())),
	tex_coords_descr(cgv::render::element_descriptor_traits<vec2>::get_type_descriptor(vec2()))
{
	nr_triangle_elements = 0;
	nr_edge_elements = 0;
}
void mesh_render_info::destruct(cgv::render::context& ctx)
{
	render_info::destruct(ctx);
	material_primitive_start.clear();
	nr_triangle_elements = 0;
	nr_edge_elements = 0;
	nr_vertices = 0;
	vbo_config.clear();
	per_attribute_vbo_config.clear();
	element_size = 0;
}
void mesh_render_info::construct_index_buffers(cgv::render::context& ctx, const cgv::media::mesh::simple_mesh_base& mesh, std::vector<idx4_type>& _unique_quadruples, std::vector<idx_type>& per_corner_vertex_index, std::vector<idx_type>& edges_element_buffer, std::vector<idx_type>& triangles_element_buffer)
{
	include_tex_coords = attribute_is_used(attribute_type::texcoords);
	include_normals = attribute_is_used(attribute_type::normal);
	include_tangents = attribute_is_used(attribute_type::tangent);
	include_colors = attribute_is_used(attribute_type::color);

	// load material textures
	for (unsigned i = 0; i < mesh.get_nr_materials(); ++i) {
		ref_materials().push_back(new cgv::render::textured_material(mesh.get_material(i)));
		ref_materials().back()->ensure_textures(ctx);
	}

	std::unique_ptr<std::vector<idx_type>> permutation;
	bool sort_by_groups = mesh.get_nr_groups() > 0;
	bool sort_by_materials = mesh.get_nr_materials() > 0;
	if (sort_by_groups || sort_by_materials) {
		permutation = std::make_unique<std::vector<idx_type>>();
		mesh.sort_faces(*permutation, sort_by_groups, sort_by_materials);
	}
	mesh.merge_indices(per_corner_vertex_index, _unique_quadruples, &include_tex_coords, &include_normals, &include_tangents);
	nr_vertices = _unique_quadruples.size();
	mesh.extract_triangle_element_buffer(per_corner_vertex_index, triangles_element_buffer, permutation.get(),
										 mesh.get_nr_materials() > 0 ? &material_primitive_start : 0);
	
	nr_triangle_elements = triangles_element_buffer.size();
	mesh.extract_wireframe_element_buffer(per_corner_vertex_index, edges_element_buffer);
	nr_edge_elements = edges_element_buffer.size();
	ct = mesh.get_color_storage_type();
}
void mesh_render_info::construct_element_vbo(cgv::render::context& ctx, const std::vector<idx_type>& edge_element_buffer, const std::vector<idx_type>& triangle_element_buffer)
{
	ref_vbos().push_back(new cgv::render::vertex_buffer(cgv::render::VBT_INDICES));
	cgv::render::vertex_buffer& vbe = *ref_vbos().back();
	vbe.create(ctx, sizeof(idx_type)*(nr_edge_elements + nr_triangle_elements));
	vbe.replace(ctx, 0, &edge_element_buffer.front(), edge_element_buffer.size());
	vbe.replace(ctx, sizeof(idx_type)*edge_element_buffer.size(), &triangle_element_buffer.front(), triangle_element_buffer.size());
}
bool mesh_render_info::attribute_is_used(attribute_type at) const
{
	return per_attribute_vbo_config.find(at) != per_attribute_vbo_config.end();
}
bool mesh_render_info::configure_vbos(cgv::render::context& ctx, const cgv::media::mesh::simple_mesh_base& mesh, const attribute_map& attribute_to_vbo_index_map, const std::vector<int>& dynamic_vbos)
{
	// convert indices of dynamic vbos into per vbo flag
	for (int dvi : dynamic_vbos) {
		if (vbo_config.size() <= dvi)
			vbo_config.resize(dvi + 1);
		vbo_config[dvi].is_dynamic = true;
	}
	// iterate attribute map in order of mesh attributes, ensure their vbo to have a dynamic flag and keep track of entry sizes
	for (attribute_type at = attribute_type::begin; at < attribute_type::end; ++(int&)at) {
		if (attribute_to_vbo_index_map.find(at) != attribute_to_vbo_index_map.end()) {
			if (!mesh.has_attribute(at)) {
				ctx.error(std::string("attempt to add attribute ") + mesh.get_attribute_name(at) + " to mesh_render_info although missing in mesh");
				continue;
			}
			uint32_t vi = attribute_to_vbo_index_map.at(at);
			if (vbo_config.size() <= vi)
				vbo_config.resize(vi + 1);
			per_attribute_vbo_config[at] = { vi, vbo_config[vi].stride };
			vbo_config[vi].attributes = cgv::media::mesh::simple_mesh_base::AttributeFlags(vbo_config[vi].attributes | mesh.get_attribute_flag(at));
			vbo_config[vi].stride += uint32_t(mesh.get_attribute_size(at));
		}
	}
	// validate that all indexed or skipped vbos have mesh attributes assigned to
	for (size_t vi = 0; vi < vbo_config.size(); ++vi) {
		if (vbo_config[vi].stride == 0) {
			ctx.error(std::string("vertex attribute vbo configuration with empty vbo ") + cgv::utils::to_string(vi));
			return false;
		}
	}
	return true;
}
bool mesh_render_info::bind(context& ctx, shader_program& prog, bool force_success, int aa_index)
{
	return render_info::bind(ctx, prog, force_success, 0);
}
bool mesh_render_info::bind_wireframe(context& ctx, shader_program& prog, bool force_success)
{
	wire_draw_call.prog = &prog;
	return render_info::bind(ctx, prog, force_success, 1);
}
void mesh_render_info::construct_draw_calls(cgv::render::context& ctx)
{
	// construct aa with new aab for surface rendering
	int aa_index = (int)ref_aas().size();
	ref_aas().push_back(attribute_array());
	ref_aas().push_back(attribute_array());

	auto& aa = ref_aas()[aa_index];
	auto& wire_aa = ref_aas()[aa_index + 1];

	aa.aab_ptr = new cgv::render::attribute_array_binding();
	aa.aab_ptr->create(ctx);

	wire_aa.aab_ptr = new cgv::render::attribute_array_binding();
	wire_aa.aab_ptr->create(ctx);

	// set element and attribute pointers
	aa.aab_ptr->set_element_array(ctx, *ref_vbos().back());
	wire_aa.aab_ptr->set_element_array(ctx, *ref_vbos().back());
	if (attribute_is_used(attribute_type::position)) {
		const auto& avc = per_attribute_vbo_config[attribute_type::position];
		     aa.add_attribute(position_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_POSITION);
		wire_aa.add_attribute(position_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_POSITION);
	}
	if (attribute_is_used(attribute_type::texcoords)) {
		const auto& avc = per_attribute_vbo_config[attribute_type::texcoords];
		     aa.add_attribute(tex_coords_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_TEXCOORD);
		wire_aa.add_attribute(tex_coords_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_TEXCOORD);
	}
	if (attribute_is_used(attribute_type::normal)) {
		const auto& avc = per_attribute_vbo_config[attribute_type::normal];
		     aa.add_attribute(position_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_NORMAL);
		wire_aa.add_attribute(position_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_NORMAL);
	}
	if (attribute_is_used(attribute_type::tangent)) {
		const auto& avc = per_attribute_vbo_config[attribute_type::tangent];
		     aa.add_attribute(position_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_BY_NAME, "tangent");
		wire_aa.add_attribute(position_descr, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_BY_NAME, "tangent");
	}
	if (attribute_is_used(attribute_type::color)) {
		static int nr_comps[] = { 4,4,3,4 };
		static cgv::type::info::TypeId type_ids[] = { cgv::type::info::TI_UINT8,cgv::type::info::TI_UINT8,cgv::type::info::TI_FLT32,cgv::type::info::TI_FLT32 };
		cgv::render::type_descriptor ctd(type_ids[ct], nr_comps[ct], true);
		const auto& avc = per_attribute_vbo_config[attribute_type::color];
		     aa.add_attribute(ctd, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_COLOR);
		wire_aa.add_attribute(ctd, avc.vbo_index, avc.offset, nr_vertices, vbo_config[avc.vbo_index].stride, cgv::render::VA_COLOR);
	}
	// construct draw call structure which is independent of mesh fragment
	cgv::render::draw_call dc;
	dc.aa_index = aa_index;
	dc.primitive_type = PT_TRIANGLES;
	dc.vertex_offset = 0;
	dc.draw_call_type = cgv::render::RCT_INDEXED;
	dc.index_type = cgv::type::info::TI_UINT32;
	dc.instance_count = 1;
	dc.prog = 0;
	dc.material_index = -1;
	dc.alpha_mode = AM_OPAQUE;
	// in case of no fragments, generate a single draw call
	if (material_primitive_start.empty()) {
		dc.count = uint32_t(nr_triangle_elements);
		dc.indices = (void*)(sizeof(idx_type) * nr_edge_elements);
		ref_draw_calls().push_back(dc);
	}
	// otherwise, for each mesh fragment construct a draw call 
	else {
		size_t fi;
		for (fi = 0; fi < material_primitive_start.size(); ++fi) {
			const auto& mps = material_primitive_start[fi];
			size_t next_start = nr_triangle_elements;
			if (fi + 1 < material_primitive_start.size())
				next_start = material_primitive_start[fi + 1][2];
			dc.count = uint32_t(next_start - mps[2]);
			dc.indices = (void*)(sizeof(idx_type)*(nr_edge_elements+mps[2]));
			dc.material_index = mps[0];
			if (dc.material_index != -1) {
				const auto& mat = *ref_materials()[dc.material_index];
				if ((mat.get_transparency_index() != -1) ||
					(mat.get_transparency() > 0.01f)) {
					dc.alpha_mode = AM_MASK_AND_BLEND;
					dc.alpha_cutoff = 0.02f;
				}
				else
					dc.alpha_mode = AM_OPAQUE;
			}
			ref_draw_calls().push_back(dc);
		}
	}
	// set draw call for wire frame rendering
	wire_draw_call.aa_index = aa_index+1;
	wire_draw_call.primitive_type = PT_LINES;
	wire_draw_call.vertex_offset = 0;
	wire_draw_call.draw_call_type = cgv::render::RCT_INDEXED;
	wire_draw_call.count = uint32_t(nr_edge_elements);
	wire_draw_call.index_type = cgv::type::info::TI_UINT32;
	wire_draw_call.indices = 0;
	wire_draw_call.material_index = -1;
	wire_draw_call.alpha_mode = AM_OPAQUE;
	wire_draw_call.instance_count = 1;
	wire_draw_call.prog = 0;
}
void mesh_render_info::set_nr_instances(unsigned nr)
{
	for (auto& dc : ref_draw_calls()) {
		dc.instance_count = nr;
		switch (dc.draw_call_type) {
		case RCT_ARRAYS:
			if (nr > 0)
				dc.draw_call_type = RCT_ARRAYS_INSTANCED;
			break;
		case RCT_INDEXED:
			if (nr > 0)
				dc.draw_call_type = RCT_INDEXED_INSTANCED;
			break;
		case RCT_ARRAYS_INSTANCED:
			if (nr == 0)
				dc.draw_call_type = RCT_ARRAYS;
			break;
		case RCT_INDEXED_INSTANCED:
			if (nr == 0)
				dc.draw_call_type = RCT_INDEXED;
			break;
		}
	}
}
void mesh_render_info::draw_primitive(cgv::render::context& ctx, size_t primitive_index, bool skip_opaque, bool skip_blended, bool use_materials)
{
	// extract indices of to be drawn calls
	std::vector<size_t> dcis;
	size_t dci = 0;
	while (dci < draw_calls.size()) {
		while (dci < draw_calls.size() && material_primitive_start[dci][1] != primitive_index)
			++dci;
		if (skip_opaque)
			while (dci < draw_calls.size() && (draw_calls[dci].alpha_mode & AM_BLEND) == 0)
				++dci;
		if (skip_blended)
			while (dci < draw_calls.size() && (draw_calls[dci].alpha_mode & AM_BLEND) != 0)
				++dci;
		if (dci >= draw_calls.size())
			break;
		dcis.push_back(dci);
		++dci;
	}
	// draw extracted calls
	draw_call* prev_dc = 0;
	for (size_t i = 0; i < dcis.size(); ++i) {
		draw_call* next_dc = i + 1 < dcis.size() ? &draw_calls[dcis[i + 1]] : 0;
		draw(ctx, draw_calls[dcis[i]], prev_dc, next_dc, use_materials);
		prev_dc = &draw_calls[dcis[i]];
	}

}
void mesh_render_info::draw_wireframe(cgv::render::context& ctx)
{
	draw(ctx, wire_draw_call);
}
	}
}