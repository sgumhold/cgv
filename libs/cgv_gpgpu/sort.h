#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API sort : public algorithm {
public:
	enum class Order {
		kAscending,
		kDescending
	};

	sort(const std::string& type_name) : algorithm(type_name) {}

	bool init(cgv::render::context& ctx, const sl::data_type& key_type, Order order, size_t size);
	bool init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, Order order, size_t size);
	
	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer);
	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer);

protected:
	virtual bool v_init(cgv::render::context& ctx) = 0;

	virtual void v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) = 0;

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
