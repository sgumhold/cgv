#pragma once

#include "gpu_algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** GPU sorting routine specialization for distance based sorting implemented using a prefix-sum based radix sort. */
class CGV_API visibility_sort : public gpu_algorithm {
public:
	enum SortOrder {
		SO_ASCENDING = 0,
		SO_DESCENDING = 1
	};

protected:
	SortOrder sort_order = SO_ASCENDING;
	bool value_init_override = true;
	cgv::type::info::TypeId value_type = TI_UINT32;
	unsigned value_component_count = 1;
	std::string value_type_def = "uint";
	std::string data_type_def = "";
	std::string auxiliary_type_def = "";
	std::string key_definition = "";
	std::string uniform_definition = "";

	unsigned n = 0;
	unsigned n_pad = 0;
	unsigned group_size = 0;
	unsigned num_block_sums = 0;
	unsigned num_groups = 0;
	unsigned num_scan_groups = 0;

	/// buffer objects
	cgv::render::vertex_buffer keys_in_buffer;
	cgv::render::vertex_buffer keys_out_buffer;
	cgv::render::vertex_buffer values_out_buffer;
	cgv::render::vertex_buffer prefix_sums_buffer;
	cgv::render::vertex_buffer block_sums_buffer;
	cgv::render::vertex_buffer last_sum_buffer;

	/// shader programs
	cgv::render::shader_program key_prog;
	cgv::render::shader_program scan_local_prog;
	cgv::render::shader_program scan_global_prog;
	cgv::render::shader_program scatter_prog;

	bool load_shader_programs(cgv::render::context& ctx);

public:
	visibility_sort() : gpu_algorithm() {}

	void destruct(const cgv::render::context& ctx);

	bool init(cgv::render::context& ctx, size_t count);

	void execute(cgv::render::context& ctx, const cgv::render::vertex_buffer& data_buffer, const cgv::render::vertex_buffer& value_buffer, const vec3& eye_pos, const vec3& view_dir, const cgv::render::vertex_buffer* auxiliary_buffer = nullptr);

	/// returns the number of padding elements used to make the total count a multiple of group size
	unsigned int get_padding() { return n_pad; }

	/// the number of OpenGL work groups used
	unsigned int get_num_groups() { return num_groups; }

	/// whether to sort ascending or descending
	void set_sort_order(SortOrder order) { sort_order = order; }

	/** Specifies the format of the to-be-sorted values consisting of type and
		component count and maps to the GLSL type.
		Supported types are TI_INT32, TI_UINT32 and TI_FLT32.
		The component count must not exceed 4.
		Example: to sort a pair of two unsigned integers use: (TI_UINT32, 2),
		which wil use uvec2 in the shader. */
	void set_value_format(cgv::type::info::TypeId type, unsigned component_count);

	/** Specifies whether to initialize all values in ascending order based on
		their array index in the initial key generation on each sort run. This
		may only be useful when sorting a single index buffer for visibility sorting etc.
		When sorting pairs of indices or predetermined indices, set this to false.*/
	void initialize_values_on_sort(bool flag) { value_init_override = flag; }

	/** GLSL code to define the data type and structure of one element of the input data buffer.
		This effectively defines the contents of a struct used to represent one array element.
		The default value is "float x, y, z;" to map the buffer contents to an array of vec3, e.g.
		positions.
		Careful: vec3 max not be used directly with a shader storage buffer (due to a bug on some older drivers), hence the three
		separate coordinates! However, vec4 works as expected. */
	void set_data_type_override(const std::string& def) { data_type_def = def; }

	/** Resets the data type definition to an empty string, which will not override the default
		definition in the shader. */
	void reset_data_type_override() { data_type_def = ""; }

	/** GLSL code to define the data type and structure of one element of the auxiliary data buffer.
		This will activate an additional buffer that may be used to access additional data needed
		for calculations. The auxiliary data is accessible through the aux_values array and is
		represented with the aux_type type. */
	void set_auxiliary_type_override(const std::string& def) { auxiliary_type_def = def; }

	/** Resets the auxiliary data type definition to an empty string, which will also deactivate
		the usage of an auxiliary buffer. */
	void reset_auxiliary_type_override() { auxiliary_type_def = ""; }

	/** GLSL code to define the calculation of the key value used to sort the buffer.
		The default definition expects positions as vec3 in the data buffer and calculates the
		distance to the eye position as the key.
		!Important:
			Key definition must define a variable of type float named key as the output.
		Variables:
		* reserved (expert use only)
			keys, n, n_padded, idx
		* input (read only)
			data - the input data buffer as given to the sort method
			eye_pos - the eye position as given to the sort method (set to zero if not used)
			view_dir - the view direction as given to the sort method (set to zero if not used)
		* in/output
			values - the values to be sorted
		*/
	void set_key_definition_override(const std::string& def) { key_definition = def; }

	/** Resets the key definition to an empty string, which will not override the default
		definition in the shader. */
	void reset_key_definition_override() { key_definition = ""; }

	/** GLSL code to define the uniforms used in the key shader.
		Example: uniform float threshold;
		Multiple uniforms are possible.
		The default definition is empty, i.e. no uniforms are defined.
		Reserved uniform names: n, n_padded, eye_pos, view_dir (do not use!)
		*/
	void set_uniform_definition_override(const std::string& def) { uniform_definition = def; }

	/** Resets the uniform definition for the vote shader. */
	void reset_uniform_definition_override() { uniform_definition = ""; }

	cgv::render::shader_program& ref_key_prog() { return key_prog; }
};

}
}

#include <cgv/config/lib_end.h>
