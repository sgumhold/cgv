#include "scan_and_compact.h"

namespace cgv {
namespace gpgpu {

void scan_and_compact::destruct(const cgv::render::context& ctx) {

	vote_prog.destruct(ctx);
	scan_local_prog.destruct(ctx);
	scan_global_prog.destruct(ctx);
	compact_prog.destruct(ctx);

	delete_buffer(ctx, votes_buffer);
	delete_buffer(ctx, prefix_sums_buffer);
	delete_buffer(ctx, block_sums_buffer);

	is_initialized_ = false;
}

bool scan_and_compact::load_shader_programs(cgv::render::context& ctx) {

	bool res = true;
	std::string where = "cgv::gpgpu::scan_and_compact::load_shader_programs()";

	cgv::render::shader_define_map vote_defines;
	cgv::render::shader_code::set_define(vote_defines, "DATA_TYPE_DEFINITION", data_type_def, "");
	cgv::render::shader_define_map compact_defines = vote_defines;

	cgv::render::shader_code::set_define(vote_defines, "UNIFORM_DEFINITION", uniform_definition, "");
	cgv::render::shader_code::set_define(vote_defines, "VOTE_DEFINITION", vote_definition, "");

	cgv::render::shader_code::set_define(compact_defines, "MODE", mode, M_COPY_DATA);

	res = res && cgv::render::shader_library::load(ctx, vote_prog, vote_prog_name == "" ? "gpgpu_sac_vote" : vote_prog_name, vote_defines, true, where);
	res = res && cgv::render::shader_library::load(ctx, scan_local_prog, "gpgpu_sac_scan_local", true, where);
	res = res && cgv::render::shader_library::load(ctx, scan_global_prog, "gpgpu_sac_scan_global", true, where);
	res = res && cgv::render::shader_library::load(ctx, compact_prog, "gpgpu_sac_compact", compact_defines, true, where);

	return res;
}

bool scan_and_compact::init(cgv::render::context& ctx, size_t count) {

	is_initialized_ = false;

	if(!load_shader_programs(ctx))
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

	unsigned num_vote_ballots = calculate_num_groups(n + n_pad, 32u);

	size_t votes_size = num_vote_ballots * 4;
	size_t data_size = (n + n_pad) * sizeof(unsigned int);
	size_t blocksums_size = num_block_sums * sizeof(unsigned int);

	ensure_buffer(ctx, votes_buffer, data_size);
	ensure_buffer(ctx, prefix_sums_buffer, 4 * sizeof(unsigned int) + data_size / 4);
	ensure_buffer(ctx, block_sums_buffer, blocksums_size);

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

	is_initialized_ = true;
	return true;
}

unsigned scan_and_compact::execute(cgv::render::context& ctx, const cgv::render::vertex_buffer& in_buffer, const cgv::render::vertex_buffer& out_buffer, bool return_count) {

	in_buffer.bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	out_buffer.bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	votes_buffer.bind(ctx, 2);
	
	if(!vote_prog.is_enabled()) vote_prog.enable(ctx);
	dispatch_compute1d(num_scan_groups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	vote_prog.disable(ctx);

	prefix_sums_buffer.bind(ctx, 3);
	block_sums_buffer.bind(ctx, 4);

	scan_local_prog.enable(ctx);
	dispatch_compute1d(num_scan_groups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	scan_local_prog.disable(ctx);

	scan_global_prog.enable(ctx);
	dispatch_compute1d(1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	scan_global_prog.disable(ctx);

	compact_prog.enable(ctx);
	dispatch_compute1d(num_scan_groups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	compact_prog.disable(ctx);

	unsigned count = -1;

	if(return_count) {
		prefix_sums_buffer.bind(ctx);
		void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&count, ptr, sizeof(unsigned int));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		prefix_sums_buffer.unbind(ctx);
	}

	return count;
}

} // namespace gpgpu
} // namespace cgv
