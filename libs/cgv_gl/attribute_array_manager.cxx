#include "attribute_array_manager.h"

namespace cgv {
namespace render {

attribute_array_manager::attribute_array_manager(VertexBufferUsage _default_usage) {
	default_usage = _default_usage;
}
bool attribute_array_manager::has_attribute(const context& ctx, int loc) const {
	return aab.is_array_enabled(ctx, loc);
}
attribute_array_manager::~attribute_array_manager() {
	if(aab.is_created() && aab.ctx_ptr && aab.ctx_ptr->make_current())
		destruct(*aab.ctx_ptr);
}
bool attribute_array_manager::set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes) {
	return ctx.set_attribute_array_void(&aab, loc, element_type, &vbo, reinterpret_cast<const void*>(offset_in_bytes), nr_elements, stride_in_bytes);
}
/// whether aam contains an index buffer
bool attribute_array_manager::has_index_buffer() const {
	auto iter = vbos.find(-1);
	if(iter == vbos.end())
		return false;
	return iter->second != 0;
}
///
void attribute_array_manager::remove_indices(const context& ctx) {
	vertex_buffer*& vbo_ptr = vbos[-1];
	if(vbo_ptr) {
		vbo_ptr->destruct(ctx);
		delete vbo_ptr;
		vbos[-1] = 0;
	}
}
bool attribute_array_manager::is_created() {
	return aab.is_created();
}
bool attribute_array_manager::init(context& ctx) {
	return aab.create(ctx);
}
bool attribute_array_manager::enable(context& ctx) {
	return aab.enable(ctx);
}
bool attribute_array_manager::disable(context& ctx) {
	return aab.disable(ctx);
}
///
void attribute_array_manager::destruct(const context& ctx) {
	for(auto& p : vbos) {
		if (p.second) {
			p.second->destruct(ctx);
			delete p.second;
			p.second = 0;
		}
	}
	vbos.clear();
	aab.destruct(ctx);
}

}
}
