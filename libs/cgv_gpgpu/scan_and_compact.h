#pragma once

#include "gpu_algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** GPU filter routine implemented using prefix-sum based counting. */
class CGV_API scan_and_compact : public gpu_algorithm {
public:
	enum Mode {
		M_CREATE_INDICES,
		M_COPY_DATA
	};

protected:
	Mode mode = M_COPY_DATA;

	unsigned n = 0;
	unsigned n_pad = 0;
	unsigned group_size = 0;
	unsigned num_groups = 0;
	unsigned num_scan_groups = 0;
	unsigned num_block_sums = 0;

	GLuint votes_ssbo = 0;
	GLuint prefix_sums_ssbo = 0;
	GLuint block_sums_ssbo = 0;

	std::string vote_prog_name = "";
	std::string data_type_def = "";
	std::string uniform_definition = "";
	std::string vote_definition = "";

	shader_program vote_prog;
	shader_program scan_local_prog;
	shader_program scan_global_prog;
	shader_program compact_prog;

	bool load_shader_programs(context& ctx);

	void delete_buffers();

public:
	scan_and_compact() : gpu_algorithm() {}

	void destruct(context& ctx);

	bool init(context& ctx, size_t count);

	unsigned execute(context& ctx, GLuint in_buffer, GLuint out_buffer, bool return_count = true);
	
	/** GLSL code to define the data type and structure of one element of the input data buffer.
		This effectively defines the contents of a struct used to represent one array element.
		The default value is "float x, y, z;" to map the buffer contents to an array of vec3, e.g.
		positions.
		Careful: vec3 may not be used directly with a shader storage buffer (due to a bug on some older drivers), hence the three
		separate coordinates! However, vec4 works as expected. */
	void set_data_type_override(const std::string& def) { data_type_def = def; }

	/** Resets the data type definition to an empty string, which will not override the default
		definition in the shader. */
	void reset_data_type_override() { data_type_def = ""; }

	/** GLSL code to define the uniforms used in the vote shader.
		Example: uniform float threshold;
		Multiple uniforms are possible.
		The default definition is empty, i.e. no uniforms are defined.
		Reserved uniform names: n, n_padded (do not use!)
		*/
	void set_uniform_definition_override(const std::string& def) { uniform_definition = def; }

	/** Resets the uniform definition for the vote shader. */
	void reset_uniform_definition_override() { uniform_definition = ""; }

	/** GLSL code to define the calculation of the vote value used to filter the data elements.
		This defines the body of a method that takes a data element as input and returns a boolean vote.
		Return <false> to exclude this element and <true> to include this element in the compacted array.
		The default definition just returns <false>.
		Variables:
		* reserved (expert use only)
			n, n_padded
		* input (read only)
			data_type value - the input data element of this shader invocation
		* output
			bool
		*/
	void set_vote_definition_override(const std::string& def) { vote_definition = def; }

	/** Resets the vote definition to an empty string, which will not override the default
		definition in the shader. */
	void reset_vote_definition_override() { vote_definition = ""; }

	void set_vote_prog_name(const std::string& name = "") { vote_prog_name = name; }

	void set_mode(Mode mode) { this->mode = mode; }

	shader_program& ref_vote_prog() { return vote_prog; }
};

}
}

#include <cgv/config/lib_end.h>
