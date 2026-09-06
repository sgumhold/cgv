#include "radix_sort.h"

#include <cgv/utils/scan.h>

namespace cgv {
namespace gpgpu {

radix_sort::radix_sort(const std::string& type_name, uint32_t radix) : sort_algorithm(type_name), _radix(radix) {
	_radix_mask = _radix - 1;
	_radix_log = static_cast<uint32_t>(std::log2(static_cast<double>(_radix)));
	_radix_passes = _key_width / _radix_log;
}

bool radix_sort::init(cgv::render::context& ctx, const sl::data_type& key_type, SortOrder order, size_t size) {
	if(!is_type_supported(key_type)) {
		raise_unsupported_type_error(key_type);
		return false;
	}

	_key_type = key_type;

	if(!is_size_supported(size)) {
		raise_error(errc::size_too_large);
		return false;
	}

	_num_keys = static_cast<uint32_t>(size);

	cgv::render::shader_compile_options config;
	config.define_macro("KEY_TYPE", "KEY_" + cgv::utils::to_upper(to_string(_key_type.type())));

	std::string value_typename = (_value_type.is_void() /* || _value_type.component_count() > 1 */) ?
		"CUSTOM" :
		cgv::utils::to_upper(to_string(_value_type.type()));
	config.define_macro("VALUE_TYPE", "VALUE_" + value_typename);

	config.define_macro("VALUE_TYPEDEF", to_string(_value_type.type()));

	config.define_macro("SORT_ASCENDING", order == SortOrder::Ascending);
	config.define_macro("SORT_PAIRS", !_value_type.is_void());

	config.define_macro("RADIX", _radix);
	config.define_macro("RADIX_MASK", _radix - 1);
	config.define_macro("RADIX_LOG", _radix_log);
	config.define_macro("RADIX_PASSES", _radix_passes);

	return v_init(ctx, config);
}

bool radix_sort::init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, SortOrder order, size_t size) {
	// TODO: Add support for up to 4-component vector value types? (Check one-sweep compatibility).
	// Maybe check key and value type support per sort implementation.
	if(!is_type_supported(value_type)) { // || value_component_count == 0 || value_component_count > 4)
		raise_unsupported_type_error(key_type);
		return false;
	}

	_value_type = value_type;
	//_value_component_count = value_component_count;

	return init(ctx, key_type, order, size);
}

bool radix_sort::is_type_supported(const sl::data_type& type) const {
	if(type.is_compound())
		return false;
	sl::Type base_type = type.type();
	return base_type == sl::Type::UInt || base_type == sl::Type::Int || base_type == sl::Type::Float;
}

} // namespace gpgpu
} // namespace cgv
