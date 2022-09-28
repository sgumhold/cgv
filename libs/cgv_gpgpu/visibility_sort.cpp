#include "visibility_sort.h"

namespace cgv {
namespace gpgpu {

void visibility_sort::destruct(context& ctx) {

	key_prog.destruct(ctx);
	scan_local_prog.destruct(ctx);
	scan_global_prog.destruct(ctx);
	scatter_prog.destruct(ctx);

	delete_buffers();

	_is_initialized = false;
}

bool visibility_sort::load_shader_programs(context& ctx) {

	bool res = true;
	std::string where = "radix_sort_4way::load_shader_programs()";

	shader_define_map key_defines;
	shader_code::set_define(key_defines, "ORDER", sort_order, SO_ASCENDING);
	shader_code::set_define(key_defines, "INITIALIZE_VALUES", value_init_override, true);
	shader_code::set_define(key_defines, "USE_AUXILIARY_BUFFER", auxiliary_type_def != "", false);
	shader_code::set_define(key_defines, "VALUE_TYPE_DEFINITION", value_type_def, std::string("uint"));

	if(data_type_def != "")
		key_defines["DATA_TYPE_DEFINITION"] = data_type_def;
	if(auxiliary_type_def != "")
		key_defines["AUXILIARY_TYPE_DEFINITION"] = auxiliary_type_def;
	if(key_definition != "")
		key_defines["KEY_DEFINITION"] = key_definition;

	res = res && cgv::render::shader_library::load(ctx, key_prog, "rs4x_keys", key_defines, true, where);
	res = res && cgv::render::shader_library::load(ctx, scan_local_prog, "rs4x_scan_local", true, where);
	res = res && cgv::render::shader_library::load(ctx, scan_global_prog, "rs4x_scan_global", true, where);
	res = res && cgv::render::shader_library::load(ctx, scatter_prog, "rs4x_scatter", { {"VALUE_TYPE_DEFINITION", value_type_def} }, true, where);

	return res;
}

void visibility_sort::delete_buffers() {

	delete_buffer(keys_in_ssbo);
	delete_buffer(keys_out_ssbo);
	delete_buffer(values_out_ssbo);
	delete_buffer(prefix_sums_ssbo);
	delete_buffer(block_sums_ssbo);
	delete_buffer(last_sum_ssbo);
}

bool visibility_sort::init(context& ctx, size_t count) {

	_is_initialized = false;

	if(!load_shader_programs(ctx))
		return false;

	delete_buffers();

	n = unsigned(count);
	group_size = 64;

	unsigned int block_size = 4 * group_size;

	// Calculate padding for n to next multiple of blocksize.
	n_pad = block_size - (n % (block_size));
	if(n % block_size == 0)
		n_pad = 0;

	num_groups = (n + n_pad + group_size - 1) / group_size;
	num_scan_groups = (n + n_pad + block_size - 1) / block_size;
	unsigned int block_sum_offset_shift = static_cast<unsigned int>(log2f(float(block_size)));

	num_block_sums = num_scan_groups;

	unsigned int num = 1;
	while(num_block_sums > num)
		num <<= 1;
	num_block_sums = num;

	size_t data_size = (n + n_pad) * sizeof(unsigned int);
	size_t blocksums_size = 4 * num_block_sums * sizeof(unsigned int);

	create_buffer(keys_in_ssbo, data_size);
	create_buffer(keys_out_ssbo, data_size);
	create_buffer(values_out_ssbo, value_component_count*data_size);
	create_buffer(prefix_sums_ssbo, data_size / 4);
	create_buffer(block_sums_ssbo, blocksums_size);
	create_buffer(last_sum_ssbo, 4 * sizeof(unsigned int));

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

	_is_initialized = true;
	return true;
}

void visibility_sort::execute(context& ctx, GLuint data_buffer, GLuint value_buffer, const vec3& eye_pos, const vec3& view_dir, GLuint auxiliary_buffer) {

	GLuint values_in_buffer = value_buffer;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, keys_in_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, values_in_buffer);
	if(auxiliary_type_def != "" && auxiliary_buffer > 0)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, auxiliary_buffer);

	key_prog.enable(ctx);
	key_prog.set_uniform(ctx, "eye_pos", eye_pos);
	key_prog.set_uniform(ctx, "view_dir", view_dir);
	glDispatchCompute(num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	key_prog.disable(ctx);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, prefix_sums_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, block_sums_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, last_sum_ssbo);

	for(unsigned int b = 0; b < 32; b += 2) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, keys_in_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, keys_out_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, values_in_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, values_out_ssbo);

		scan_local_prog.enable(ctx);
		glDispatchCompute(num_scan_groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		scan_local_prog.disable(ctx);

		scan_global_prog.enable(ctx);
		glDispatchCompute(4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		scan_global_prog.disable(ctx);

		scatter_prog.enable(ctx);
		glDispatchCompute(num_scan_groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		scatter_prog.disable(ctx);

		std::swap(keys_in_ssbo, keys_out_ssbo);
		std::swap(values_in_buffer, values_out_ssbo);
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
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

	switch(type) {
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

	if(component_count > 1) value_type_def = vec_type + std::to_string(component_count);
	else value_type_def = scalar_type;

	value_type = type;
}

void visibility_sort::set_auxiliary_type_override(const std::string& def) {

	auxiliary_type_def = def;
}

void visibility_sort::set_data_type_override(const std::string& def) {

	data_type_def = def;
}

void visibility_sort::set_key_definition_override(const std::string& def) {

	key_definition = def;
}

}
}
