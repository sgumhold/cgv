#include "sort.h"

namespace cgv {
namespace gpgpu {

bool sort::init(cgv::render::context& ctx, const sl::data_type& key_type, Order order, size_t size) {
	if(!is_type_supported(key_type))
		return false;

	_key_type = key_type;
	_order = order;
	
	const size_t max_size = 0xFFFFFFFF;
	if(size == 0 || size > max_size)
		return false;

	_num_keys = static_cast<uint32_t>(size);

	return v_init(ctx);
}

bool sort::init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, Order order, size_t size) {
	// TODO: Add support for up to 4-component vector value types. (Check one-sweep compatibility).
	// Maybe check key and value type support per sort implementation.
	if(!is_type_supported(value_type))// || value_component_count == 0 || value_component_count > 4)
		return false;

	_value_type = value_type;
	//_value_component_count = value_component_count;

	return init(ctx, key_type, order, size);
}

void sort::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer) {
	v_dispatch(ctx, &keys_buffer, nullptr);
}

void sort::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer) {
	v_dispatch(ctx, &keys_buffer, &values_buffer);
}

bool sort::is_type_supported(const sl::data_type& type) const {
	if(type.is_compound())
		return false;
	sl::Type base_type = type.type();
	return base_type == sl::Type::kUInt || base_type == sl::Type::kInt || base_type == sl::Type::kFloat;
}

} // namespace gpgpu
} // namespace cgv
