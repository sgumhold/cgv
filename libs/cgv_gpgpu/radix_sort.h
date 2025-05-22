#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API radix_sort : public algorithm {
public:
	enum class Order {
		kAscending,
		kDescending
	};

	radix_sort(const std::string& type_name, uint32_t radix);

	bool init(cgv::render::context& ctx, const sl::data_type& key_type, Order order, size_t size);
	bool init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, Order order, size_t size);
	
	virtual void destruct(const cgv::render::context& ctx) = 0;

	bool resize(cgv::render::context& ctx, size_t size);

	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer);
	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer);

protected:
	virtual bool v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) = 0;

	virtual bool v_resize(cgv::render::context& ctx) = 0;

	virtual void v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) = 0;

	// TODO: Make configurable.
	const uint32_t _key_width = 32;
	uint32_t _radix = 2;
	uint32_t _radix_mask = 1;
	uint32_t _radix_log = 1;
	uint32_t _radix_passes = _key_width;

	sl::data_type _key_type;
	sl::data_type _value_type;
	Order _order = Order::kAscending;
	uint32_t _num_keys = 0;

private:
	bool is_type_supported(const sl::data_type& type) const;	
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
