#pragma once

#include "sort_algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API radix_sort : public sort_algorithm {
public:
	radix_sort(const std::string& type_name, uint32_t radix);
	
	bool init(cgv::render::context& ctx, const sl::data_type& key_type, SortOrder order, size_t size) override;
	bool init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, SortOrder order, size_t size) override;

	void destruct(const cgv::render::context& ctx) override = 0;

protected:
	bool v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) override = 0;

	bool v_resize(cgv::render::context& ctx) override = 0;

	void v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) override = 0;

	// TODO: Make configurable.
	const uint32_t _key_width = 32;
	uint32_t _radix = 2;
	uint32_t _radix_mask = 1;
	uint32_t _radix_log = 1;
	uint32_t _radix_passes = _key_width;

	sl::data_type _key_type;
	sl::data_type _value_type;

private:
	bool is_type_supported(const sl::data_type& type) const;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
