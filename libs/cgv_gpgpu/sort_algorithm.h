#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

enum class SortOrder {
	Ascending,
	Descending
};

class CGV_API sort_algorithm : public algorithm {
public:
	sort_algorithm(const std::string& type_name);
	virtual ~sort_algorithm() = default;

	virtual bool init(cgv::render::context& ctx, const sl::data_type& key_type, SortOrder order, size_t size) = 0;
	virtual bool init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, SortOrder order, size_t size) = 0;

	virtual void destruct(const cgv::render::context& ctx) = 0;

	bool resize(cgv::render::context& ctx, size_t size);

	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer);
	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer);

protected:
	bool is_size_supported(size_t size) const;

	virtual bool v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) = 0;

	virtual bool v_resize(cgv::render::context& ctx) = 0;

	virtual void v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) = 0;

	uint32_t _num_keys = 0;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
