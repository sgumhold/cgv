#include "radix_sort.h"

#include <cgv/utils/scan.h>

namespace cgv {
namespace gpgpu {

radix_sort::radix_sort(const std::string& type_name, uint32_t radix) : algorithm(type_name), _radix(radix) {
	_radix_mask = _radix - 1;
	_radix_log = static_cast<uint32_t>(std::log2(static_cast<double>(_radix)));
	_radix_passes = _key_width / _radix_log;
}

bool radix_sort::init(cgv::render::context& ctx, const sl::data_type& key_type, Order order, size_t size) {
	if(!is_type_supported(key_type))
		return false;

	_key_type = key_type;
	_order = order;
	
	const size_t max_size = 0xFFFFFFFF;
	if(size == 0 || size > max_size)
		return false;

	_num_keys = static_cast<uint32_t>(size);

	cgv::render::shader_compile_options config;
	config.defines["KEY_TYPE"] = "KEY_" + cgv::utils::to_upper(to_string(_key_type.type()));

	std::string value_typename = (_value_type.is_void() /* || _value_type.component_count() > 1 */) ?
		"CUSTOM" :
		cgv::utils::to_upper(to_string(_value_type.type()));
	config.defines["VALUE_TYPE"] = "VALUE_" + value_typename;

	config.defines["VALUE_TYPEDEF"] = to_string(_value_type.type());

	config.defines["SORT_ASCENDING"] = _order == Order::kAscending ? "1" : "0";
	config.defines["SORT_PAIRS"] = _value_type.is_void() ? "0" : "1";

	config.defines["RADIX"] = std::to_string(_radix);
	config.defines["RADIX_MASK"] = std::to_string(_radix - 1);
	config.defines["RADIX_LOG"] = std::to_string(_radix_log);
	config.defines["RADIX_PASSES"] = std::to_string(_radix_passes);

	return v_init(ctx, config);
}

bool radix_sort::init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, Order order, size_t size) {
	// TODO: Add support for up to 4-component vector value types. (Check one-sweep compatibility).
	// Maybe check key and value type support per sort implementation.
	if(!is_type_supported(value_type))// || value_component_count == 0 || value_component_count > 4)
		return false;

	_value_type = value_type;
	//_value_component_count = value_component_count;

	return init(ctx, key_type, order, size);
}

bool radix_sort::resize(cgv::render::context& ctx, size_t size) {
	if(static_cast<size_t>(_num_keys) == size)
		return true;

	const size_t max_size = 0xFFFFFFFF;
	if(size == 0 || size > max_size)
		return false;

	_num_keys = static_cast<uint32_t>(size);
	return v_resize(ctx);
}

void radix_sort::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer) {
	v_dispatch(ctx, &keys_buffer, nullptr);
}

void radix_sort::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer) {
	v_dispatch(ctx, &keys_buffer, &values_buffer);
}

bool radix_sort::is_type_supported(const sl::data_type& type) const {
	if(type.is_compound())
		return false;
	sl::Type base_type = type.type();
	return base_type == sl::Type::kUInt || base_type == sl::Type::kInt || base_type == sl::Type::kFloat;
}

} // namespace gpgpu
} // namespace cgv
