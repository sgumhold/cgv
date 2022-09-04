#include "scan_and_compact.h"

namespace cgv {
namespace gpgpu {

void scan_and_compact::destruct(context& ctx) {

	vote_prog.destruct(ctx);
	scan_local_prog.destruct(ctx);
	scan_global_prog.destruct(ctx);
	compact_prog.destruct(ctx);

	delete_buffers();

	_is_initialized = false;
}

bool scan_and_compact::load_shader_programs(context& ctx) {

	bool res = true;
	std::string where = "scan_and_compact::load_shader_programs()";

	shader_define_map vote_defines, compact_defines;

	if(data_type_def != "") {
		vote_defines["DATA_TYPE_DEFINITION"] = data_type_def;
		compact_defines = vote_defines;
	}
	if(uniform_definition != "")
		vote_defines["UNIFORM_DEFINITION"] = uniform_definition;
	if(vote_definition != "")
		vote_defines["VOTE_DEFINITION"] = vote_definition;
	
	if(mode == M_CREATE_INDICES)
		compact_defines["CREATE_INDICES"] = "1";

	res = res && cgv::glutil::shader_library::load(ctx, vote_prog, vote_prog_name == "" ? "sac_vote" : vote_prog_name, vote_defines, true, where);
	res = res && cgv::glutil::shader_library::load(ctx, scan_local_prog, "sac_scan_local", true, where);
	res = res && cgv::glutil::shader_library::load(ctx, scan_global_prog, "sac_scan_global", true, where);
	res = res && cgv::glutil::shader_library::load(ctx, compact_prog, "sac_compact", compact_defines, true, where);

	return res;
}

void scan_and_compact::delete_buffers() {

	delete_buffer(votes_ssbo);
	delete_buffer(prefix_sums_ssbo);
	delete_buffer(block_sums_ssbo);
}

bool scan_and_compact::init(context& ctx, size_t count) {

	_is_initialized = false;

	if(!load_shader_programs(ctx))
		return false;

	delete_buffers();

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

	unsigned num_vote_ballots = calculate_num_groups(n + n_pad, 32);

	size_t votes_size = num_vote_ballots * 4;
	size_t data_size = (n + n_pad) * sizeof(unsigned int);
	size_t blocksums_size = num_block_sums * sizeof(unsigned int);

	create_buffer(votes_ssbo, data_size);
	create_buffer(prefix_sums_ssbo, 4 * sizeof(unsigned int) + data_size / 4);
	create_buffer(block_sums_ssbo, blocksums_size);

	vote_prog.enable(ctx);
	vote_prog.set_uniform(ctx, "n", n);
	vote_prog.set_uniform(ctx, "n_padded", n + n_pad);
	vote_prog.disable(ctx);

	scan_local_prog.enable(ctx);
	scan_local_prog.set_uniform(ctx, "n", n + n_pad);
	scan_local_prog.set_uniform(ctx, "n_scan_groups", num_scan_groups);
	scan_local_prog.disable(ctx);

	scan_global_prog.enable(ctx);
	scan_global_prog.set_uniform(ctx, "n_block_sums", num_block_sums);
	scan_global_prog.set_uniform(ctx, "last_block_sum_idx", ((n + n_pad) >> block_sum_offset_shift) - 1);
	scan_global_prog.disable(ctx);

	compact_prog.enable(ctx);
	compact_prog.set_uniform(ctx, "n", n + n_pad);
	compact_prog.disable(ctx);

	_is_initialized = true;
	return true;
}

unsigned scan_and_compact::execute(context& ctx, GLuint data_in_buffer, GLuint data_out_buffer, bool return_count) {

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_in_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data_out_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, votes_ssbo);
	
	if(!vote_prog.is_enabled()) vote_prog.enable(ctx);
	glDispatchCompute(num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	vote_prog.disable(ctx);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, prefix_sums_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, block_sums_ssbo);

	scan_local_prog.enable(ctx);
	glDispatchCompute(num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	scan_local_prog.disable(ctx);

	scan_global_prog.enable(ctx);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	scan_global_prog.disable(ctx);

	compact_prog.enable(ctx);
	glDispatchCompute(num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	compact_prog.disable(ctx);

	unsigned count = -1;

	if(return_count) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefix_sums_ssbo);
		void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&count, ptr, sizeof(unsigned int));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);

	return count;
}

}
}
