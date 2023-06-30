#pragma once

#include <vector>
#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/textured_material.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
enum DrawCallType
{
	RCT_ARRAYS,
	RCT_INDEXED,
	RCT_ARRAYS_INSTANCED,
	RCT_INDEXED_INSTANCED
};
enum AlphaMode
{
	AM_OPAQUE = 0,
	AM_MASK = 1,
	AM_BLEND = 2,
	AM_MASK_AND_BLEND = 3
};
enum VertexAttributeID
{
	VA_BY_NAME = -1,
	VA_POSITION,
	VA_NORMAL,
	VA_TEXCOORD,
	VA_COLOR
};
struct vertex_attribute
{
	VertexAttributeID vertex_attribute_id;
	std::string name;
	type_descriptor element_type;
	uint32_t vbo_index;
	size_t byte_offset;
	size_t element_count;
	uint32_t stride;
};
struct CGV_API attribute_array
{
	std::vector<vertex_attribute> vas;
	attribute_array_binding* aab_ptr;
	shader_program* prog;

	void add_attribute(type_descriptor element_type, uint32_t vbo_index,
		size_t byte_offset, size_t element_count, uint32_t stride,
		VertexAttributeID vertex_attribute_id, std::string name = "");
};
struct draw_call {
	AlphaMode alpha_mode;
	float alpha_cutoff;
	DrawCallType draw_call_type;
	PrimitiveType primitive_type;
	uint32_t material_index;
	uint32_t aa_index;
	uint32_t vertex_offset;
	uint32_t count;
	cgv::type::info::TypeId index_type;
	void*    indices;
	uint32_t instance_count;
	shader_program* prog;
};
/** the mesh_render_info structure manages vertex buffer objects for 
	attribute and element buffers as well as an attribute array binding
	object. The vertex buffer can be constructed from a simple mesh and
	the attribute array binding is bound to a specific
	shader program which defines the attribute locations.  */
class CGV_API render_info
{
public:
	/// define index type
	typedef cgv::type::uint32_type idx_type;
protected:
	/// store materials
	std::vector<textured_material*> materials;
	/// store textures
	std::vector<texture*> textures;
	/// store buffers
	std::vector<vertex_buffer*> vbos;
	/// store attribute bindings
	std::vector<attribute_array> aas;
	/// store vector of render calls
	std::vector<draw_call> draw_calls;
	/// perform a single render call
	void draw(context& ctx, const draw_call& dc, const draw_call* prev_dc = 0, const draw_call* next_dc = 0, bool use_materials = true);
public:
	/// set vbo and vbe types
	render_info();
	/// give read access to materials
	const std::vector<textured_material*>& get_materials() const;
	/// give read access to vbos
	const std::vector<vertex_buffer*>& get_vbos() const;
	/// give read access to aabs
	const std::vector<attribute_array>& get_aas() const;
	/// give read access to texture
	const std::vector<texture*>& get_textures() const;
	/// give read access to draw calls
	const std::vector<draw_call>& get_draw_calls() const;
	/// give write access to materials
	std::vector<textured_material*>& ref_materials();
	/// give write access to vbos
	std::vector<vertex_buffer*>& ref_vbos();
	/// give write access to aabs
	std::vector<attribute_array>& ref_aas();
	/// give write access to texture
	std::vector<texture*>& ref_textures();
	/// give write access to draw calls
	std::vector<draw_call>& ref_draw_calls();
	/// bind all or specific aa to the passed shader program
	virtual bool bind(context& ctx, shader_program& prog, bool force_success, int aa_index = -1);
	/// execute all draw calls
	void draw_all(context& ctx, bool skip_opaque = false, bool skip_blended = false, bool use_materials = true);
	/// destruct render mesh info and free vertex buffer objects
	void destruct(cgv::render::context& ctx);
};
	}
}

#include <cgv/config/lib_end.h>