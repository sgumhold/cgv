#include <cgv/base/base.h>
#include "render_info.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl_context.h>

namespace cgv {
	namespace render {

void attribute_array::add_attribute(type_descriptor element_type, uint32_t vbo_index,
	size_t byte_offset, size_t element_count, uint32_t stride,
	VertexAttributeID vertex_attribute_id, std::string name)
{
	cgv::render::vertex_attribute va;
	va.byte_offset = byte_offset;
	va.element_count = element_count;
	va.element_type = element_type;
	va.stride = stride;
	va.vbo_index = vbo_index;
	va.vertex_attribute_id = vertex_attribute_id;
	if (vertex_attribute_id == cgv::render::VA_BY_NAME)
		va.name = name;
	vas.push_back(va);
}

/// perform a single render call
void render_info::draw(context& ctx, const draw_call& dc, const draw_call* prev_dc, const draw_call* next_dc, bool use_materials)
{
	// ensure program is enabled
	bool new_prog = true;
	if (prev_dc && prev_dc->prog == dc.prog)
		new_prog = false;
	bool prog_enabled = false;
	if (new_prog && dc.prog && ctx.get_current_program() != dc.prog) {
		dc.prog->enable(ctx);
		prog_enabled = true;
	}
	if ((dc.alpha_mode & AM_MASK) != 0) {
		dc.prog->set_uniform(ctx, "alpha_test", true);
		dc.prog->set_uniform(ctx, "alpha_cutoff", dc.alpha_cutoff);
	}
	bool blend;
	GLenum blend_src, blend_dst;
	if ((dc.alpha_mode & AM_BLEND) != 0) {
		blend = glIsEnabled(GL_BLEND);
		glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
		glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// ensure material is enabled
	bool new_material = true;
	if (prev_dc && prev_dc->material_index == dc.material_index)
		new_material = false;
	if (new_prog || (new_material && dc.material_index >= 0 && dc.material_index < materials.size())) {
		if (dc.material_index != -1 && use_materials)
			ctx.enable_material(*materials[dc.material_index]);
		else {
			if (ctx.get_current_material())
				ctx.set_material(*ctx.get_current_material());
		}
	}

	// ensure aab is enabled
	bool new_aab = true;
	if (prev_dc && prev_dc->aa_index == dc.aa_index)
		new_aab = false;
	if (new_aab && dc.aa_index >= 0 && dc.aa_index < aas.size())
		aas[dc.aa_index].aab_ptr->enable(ctx);

	// perform render call
	GLuint pt = gl::map_to_gl(dc.primitive_type);
	switch (dc.draw_call_type) {
	case RCT_ARRAYS:
		glDrawArrays(pt, dc.vertex_offset, dc.count);
		break;
	case RCT_INDEXED:
		glDrawElements(pt, dc.count, gl::map_to_gl(dc.index_type), dc.indices);
		break;
	case RCT_ARRAYS_INSTANCED:
		glDrawArraysInstanced(pt, dc.vertex_offset, dc.count, dc.instance_count);
		break;
	case RCT_INDEXED_INSTANCED:
		glDrawElementsInstanced(pt, dc.count, gl::map_to_gl(dc.index_type), dc.indices, dc.instance_count);
		break;
	}

	// disable aab
	if (!next_dc || next_dc->aa_index != dc.aa_index)
		aas[dc.aa_index].aab_ptr->disable(ctx);

	// disable material
	if (dc.material_index != -1 && (!next_dc || next_dc->material_index != dc.material_index))
		ctx.disable_material(*materials[dc.material_index]);

	// disable program
	if ((!next_dc || next_dc->prog != dc.prog) && prog_enabled)
		dc.prog->disable(ctx);

	if ((dc.alpha_mode & AM_BLEND) != 0) {
		if (!blend)
			glDisable(GL_BLEND);
		glBlendFunc(blend_src, blend_dst);
	}
	if ((dc.alpha_mode & AM_MASK) != 0)
		dc.prog->set_uniform(ctx, "alpha_test", false);
}

/// set vbo and vbe types
render_info::render_info()
{

}

/// give read access to materials
const std::vector<textured_material*>& render_info::get_materials() const
{
	return materials;
}

/// give read access to vbos
const std::vector<vertex_buffer*>& render_info::get_vbos() const
{
	return vbos;
}

/// give read access to aabs
const std::vector<attribute_array>& render_info::get_aas() const
{
	return aas;
}
/// give read access to texture
const std::vector<texture*>& render_info::get_textures() const
{
	return textures;
}

/// give read access to draw calls
const std::vector<draw_call>& render_info::get_draw_calls() const
{
	return draw_calls;
}

/// give write access to materials
std::vector<textured_material*>& render_info::ref_materials() 
{
	return materials; 
}

/// give write access to vbos
std::vector<vertex_buffer*>& render_info::ref_vbos()
{
	return vbos;
}

/// give write access to aabs
std::vector<attribute_array>& render_info::ref_aas()
{
	return aas;
}
/// give write access to texture
std::vector<texture*>& render_info::ref_textures()
{
	return textures;
}

/// give write access to draw calls
std::vector<draw_call>& render_info::ref_draw_calls()
{
	return draw_calls;
}

/// bind all or specific draw call to the passed shader program
bool render_info::bind(context& ctx, shader_program& prog, bool force_success, int aa_index)
{
	int begin = std::max(0, aa_index);
	int end = aa_index == -1 ? (int)ref_aas().size() : aa_index + 1;
	int ai;
	bool success = true;
	std::vector<int> locs;
	for (ai = begin; ai < end; ++ai) {
		attribute_array& aa = ref_aas()[ai];

		// first check whether attributes are available in program
		for (const auto& va : aa.vas) {
			int loc = -1;
			switch (va.vertex_attribute_id) {
			case VA_POSITION: loc = prog.get_position_index(); break;
			case VA_NORMAL: loc = prog.get_normal_index(); break;
			case VA_TEXCOORD: loc = prog.get_texcoord_index(); break;
			case VA_COLOR: loc = prog.get_color_index(); break;
			default: loc = prog.get_attribute_location(ctx, va.name); break;
			}
			if (loc == -1) {
				success = false;
				if (!force_success)
					return false;
			}
			locs.push_back(loc);
		}
	}
	int li = 0;
	for (ai = begin; ai < end; ++ai) {
		attribute_array& aa = ref_aas()[ai];
		// disable all attribute arrays
		for (int i=0; i<16; ++i)
			aa.aab_ptr->disable_array(ctx, i);

		for (const auto& va : aa.vas) {
			if (locs[li] == -1) {
				++li;
				continue;
			}
			aa.aab_ptr->set_attribute_array(ctx, locs[li], va.element_type,
				*ref_vbos()[va.vbo_index], va.byte_offset, 
				va.element_count, va.stride);
			++li;
		}
		aa.prog = &prog;
	}
	for (auto& dc : ref_draw_calls()) {
		if (aa_index == -1 || dc.aa_index == aa_index)
			dc.prog = &prog;
	}
	return success;
}

///
void render_info::draw_all(context& ctx, bool skip_opaque, bool skip_blended, bool use_materials)
{
	// extract indices of to be drawn calls
	std::vector<size_t> dcis;
	size_t dci = 0;
	while (dci < draw_calls.size()) {
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

/// destruct render mesh info and free vertex buffer objects
void render_info::destruct(cgv::render::context& ctx)
{
	// destruct textures
	for (auto& t : textures) {
		t->destruct(ctx);
		delete t;
		t = 0;
	}
	textures.clear();

	// destruct materials
	for (auto& m : materials) {
		delete m;
		m = 0;
	}
	materials.clear();

	// destruct aabs
	for (auto& aa : aas) {
		aa.aab_ptr->destruct(ctx);
		delete aa.aab_ptr;
		aa.aab_ptr = 0;
	}
	aas.clear();

	// destruct vbos
	for (auto& v : vbos) {
		v->destruct(ctx);
		delete v;
		v = 0;
	}
	vbos.clear();
	draw_calls.clear();
}


	}
}