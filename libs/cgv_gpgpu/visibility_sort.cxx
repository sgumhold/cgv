#include "visibility_sort.h"

namespace cgv {
namespace gpgpu {

void visibility_sort::destruct(const cgv::render::context& ctx) {

	key_prog.destruct(ctx);
	scan_local_prog.destruct(ctx);
	scan_global_prog.destruct(ctx);
	scatter_prog.destruct(ctx);

	delete_buffer(ctx, keys_in_buffer);
	delete_buffer(ctx, keys_out_buffer);
	delete_buffer(ctx, values_out_buffer);
	delete_buffer(ctx, prefix_sums_buffer);
	delete_buffer(ctx, block_sums_buffer);
	delete_buffer(ctx, last_sum_buffer);

	is_initialized_ = false;
}

bool visibility_sort::load_shader_programs(cgv::render::context& ctx) {

	bool res = true;
	std::string where = "cgv::gpgpu::visibility_sort::load_shader_programs()";

	cgv::render::shader_define_map key_defines;
	cgv::render::shader_code::set_define(key_defines, "ORDER", sort_order, SO_ASCENDING);
	cgv::render::shader_code::set_define(key_defines, "INITIALIZE_VALUES", value_init_override, true);
	cgv::render::shader_code::set_define(key_defines, "USE_AUXILIARY_BUFFER", auxiliary_type_def != "", false);
	cgv::render::shader_code::set_define(key_defines, "VALUE_TYPE_DEFINITION", value_type_def, "uint");
	cgv::render::shader_code::set_define(key_defines, "DATA_TYPE_DEFINITION", data_type_def, "");
	cgv::render::shader_code::set_define(key_defines, "AUXILIARY_TYPE_DEFINITION", auxiliary_type_def, "");
	cgv::render::shader_code::set_define(key_defines, "KEY_DEFINITION", key_definition, "");
	cgv::render::shader_code::set_define(key_defines, "UNIFORM_DEFINITION", uniform_definition, "");
	
	cgv::render::shader_define_map scatter_defines;
	cgv::render::shader_code::set_define(scatter_defines, "VALUE_TYPE_DEFINITION", value_type_def, "uint");
	
	res = res && cgv::render::shader_library::load(ctx, key_prog, "gpgpu_rs4x_keys", key_defines, true, where);
	res = res && cgv::render::shader_library::load(ctx, scan_local_prog, "gpgpu_rs4x_scan_local", true, where);
	res = res && cgv::render::shader_library::load(ctx, scan_global_prog, "gpgpu_rs4x_scan_global", true, where);
	res = res && cgv::render::shader_library::load(ctx, scatter_prog, "gpgpu_rs4x_scatter", scatter_defines, true, where);

	return res;
}

bool visibility_sort::init(cgv::render::context& ctx, size_t count) {

	is_initialized_ = false;

	if(!load_shader_programs(ctx) || count == 0)
		return false;

	n = unsigned(count);
	group_size = 64;

	unsigned int block_size = 4 * group_size;

	// Calculate padding for n to next multiple of blocksize.
	n_pad = calculate_padding(n, block_size);

	num_groups = calculate_num_groups(n + n_pad, group_size);
	num_scan_groups = calculate_num_groups(n + n_pad, block_size);

	unsigned int block_sum_offset_shift = static_cast<unsigned int>(log2f(float(block_size)));

	num_block_sums = num_scan_groups;

	unsigned int num = 1;
	while(num_block_sums > num)
		num <<= 1;
	num_block_sums = num;

	size_t data_size = (n + n_pad) * sizeof(unsigned int);
	size_t blocksums_size = 4 * num_block_sums * sizeof(unsigned int);

	ensure_buffer(ctx, keys_in_buffer, data_size);
	ensure_buffer(ctx, keys_out_buffer, data_size);
	ensure_buffer(ctx, values_out_buffer, value_component_count * data_size);
	ensure_buffer(ctx, prefix_sums_buffer, data_size / 4);
	ensure_buffer(ctx, block_sums_buffer, blocksums_size);
	ensure_buffer(ctx, last_sum_buffer, 4 * sizeof(unsigned int));

	key_prog.enable(ctx);
	key_prog.set_uniform(ctx, "n", n);
	key_prog.set_uniform(ctx, "n_padded", n + n_pad);
	key_prog.disable(ctx);

	scan_local_prog.enable(ctx);
	scan_local_prog.set_uniform(ctx, "n", n + n_pad);
	scan_local_prog.set_uniform(ctx, "n_scan_groups", num_scan_groups);
	scan_local_prog.set_uniform(ctx, "n_block_sums", num_block_sums);
	scan_local_prog.disable(ctx);

	scan_global_prog.enable(ctx);
	scan_global_prog.set_uniform(ctx, "n_block_sums", num_block_sums);
	scan_global_prog.disable(ctx);

	scatter_prog.enable(ctx);
	scatter_prog.set_uniform(ctx, "n", n + n_pad);
	scatter_prog.set_uniform(ctx, "n_block_sums", num_block_sums);
	scatter_prog.set_uniform(ctx, "last_block_sum_idx", ((n + n_pad) >> block_sum_offset_shift) - 1);
	scatter_prog.disable(ctx);

	is_initialized_ = true;
	return true;
}

void visibility_sort::execute(cgv::render::context& ctx, const cgv::render::vertex_buffer& data_buffer, const cgv::render::vertex_buffer& value_buffer, const vec3& eye_pos, const vec3& view_dir, const cgv::render::vertex_buffer* auxiliary_buffer) {

	const cgv::render::vertex_buffer* keys_in_buffer_ptr = &keys_in_buffer;
	const cgv::render::vertex_buffer* keys_out_buffer_ptr = &keys_out_buffer;
	const cgv::render::vertex_buffer* values_in_buffer_ptr = &value_buffer;
	const cgv::render::vertex_buffer* values_out_buffer_ptr = &values_out_buffer;

	data_buffer.bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	keys_in_buffer_ptr->bind(ctx, 1);
	values_in_buffer_ptr->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);
	if(auxiliary_type_def != "" && auxiliary_buffer)
		auxiliary_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 3);

	key_prog.enable(ctx);
	key_prog.set_uniform(ctx, "eye_pos", eye_pos);
	key_prog.set_uniform(ctx, "view_dir", view_dir);
	dispatch_compute1d(num_scan_groups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	key_prog.disable(ctx);

	prefix_sums_buffer.bind(ctx, 4);
	block_sums_buffer.bind(ctx, 5);
	last_sum_buffer.bind(ctx, 6);

	for(unsigned int b = 0; b < 32; b += 2) {
		keys_in_buffer_ptr->bind(ctx, 0);
		keys_out_buffer_ptr->bind(ctx, 1);
		values_in_buffer_ptr->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);
		values_out_buffer_ptr->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 3);

		scan_local_prog.enable(ctx);
		dispatch_compute1d(num_scan_groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		scan_local_prog.disable(ctx);

		scan_global_prog.enable(ctx);
		dispatch_compute1d(4);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		scan_global_prog.disable(ctx);

		scatter_prog.enable(ctx);
		dispatch_compute1d(num_scan_groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		scatter_prog.disable(ctx);

		std::swap(keys_in_buffer_ptr, keys_out_buffer_ptr);
		std::swap(values_in_buffer_ptr, values_out_buffer_ptr);
	}
}

void visibility_sort::set_value_format(cgv::type::info::TypeId type, unsigned component_count) {

	if(component_count < 1) {
		std::cout << "gpu_sorter::set_value_format() ... cannot have 0 components, using 1 as default" << std::endl;
		component_count = 1;
	}

	if(component_count > 4) {
		std::cout << "gpu_sorter::set_value_format() ... cannot have more than 4 components, using 4 as default" << std::endl;
		component_count = 4;
	}

	value_component_count = component_count;

	std::string scalar_type = "uint";
	std::string vec_type = "uvec";

	switch (type) {
	case TI_UINT32:
		scalar_type = "uint";
		vec_type = "uvec";
		break;
	case TI_INT32:
		scalar_type = "int";
		vec_type = "ivec";
		break;
	case TI_FLT32:
		scalar_type = "float";
		vec_type = "vec";
		break;
	default:
		std::cout << "gpu_sorter::set_value_format() ... value type not supported, using unsigned int (uint or uvec)" << std::endl;
		type = TI_UINT32;
		break;
	}

	if(component_count > 1)
		value_type_def = vec_type + std::to_string(component_count);
	else
		value_type_def = scalar_type;

	value_type = type;
}

} // namespace gpgpu
} // namespace cgv
