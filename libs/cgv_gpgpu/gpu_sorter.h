#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_glutil/shader_library.h>

#include <iostream>

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace gpgpu {

/** Definition of an interface for highly parallel gpu sorting routines. */
class CGV_API gpu_sorter : public render_types {
public:
	enum SortOrder {
		SO_ASCENDING = 0,
		SO_DESCENDING = 1
	};

private:
	GLuint time_query = 0;
	unsigned measurements = 0;
	float avg_time = 0.0f;

protected:
	SortOrder sort_order = SO_ASCENDING;
	bool value_init_override = true;
	cgv::type::info::TypeId value_type = TI_UINT32;
	unsigned value_component_count = 1;
	std::string value_type_def = "uint";
	std::string data_type_def = "";
	std::string auxiliary_type_def = "";
	std::string key_definition = "";

	unsigned n = 0;
	unsigned n_pad = 0;
	unsigned group_size = 0;
	unsigned num_block_sums = 0;
	unsigned num_groups = 0;
	unsigned num_scan_groups = 0;

	/// buffer objects
	GLuint keys_in_ssbo = 0;
	GLuint keys_out_ssbo = 0;
	GLuint values_out_ssbo = 0;
	GLuint prefix_sums_ssbo = 0;
	GLuint block_sums_ssbo = 0;
	GLuint last_sum_ssbo = 0;

	/// shader programs
	shader_program key_prog;
	shader_program scan_local_prog;
	shader_program scan_global_prog;
	shader_program scatter_prog;

	void create_buffer(GLuint& buffer, size_t size);

	void delete_buffer(GLuint& buffer);

	virtual void delete_buffers();

	virtual bool load_shader_programs(context& ctx) = 0;

	void begin_time_query();

	double end_time_query();

	template<typename T>
	std::vector<T> read_ssbo(GLuint ssbo, unsigned int n) {

		std::vector<T> data(n, (T)0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&data[0], ptr, data.size() * sizeof(T));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return data;
	}

public:
	gpu_sorter() {}
	~gpu_sorter() {

		delete_buffers();
	}

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
		When sorting paris of indices or predetermined indices, set this to false.*/
	void initialize_values_on_sort(bool flag) { value_init_override = flag; }

	/** GLSL code to define the data type and structure of one element of the input data buffer.
		This effectively defines the contents of a struct used to represent one array element.
		The default value is "float x, y, z;" to map the buffer contents to an array of vec3, e.g.
		positions.
		Careful: vec3 max not be used directly with a shader storage buffer (due to a bug on some older drivers), hence the three
		separate coordinates! However, vec4 works as expected. */
	void set_data_type_override(const std::string& def);

	/** Resets the data type definition to an empty string, which will not override the default
		definition in the shader. */
	void reset_data_type_override() { data_type_def = ""; }

	/** GLSL code to define the data type and structure of one element of the auxiliary data buffer.
		This will activate an additional buffer that may be used to access additional data needed
		for calculations. The auxiliary data is accessible through the aux_values array and is
		represented with the aux_type type. */
	void set_auxiliary_type_override(const std::string& def);

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
	void set_key_definition_override(const std::string& def);

	/** Resets the key definition to an empty string, which will not override the default
		definition in the shader. */
	void reset_key_definition_override() { key_definition = ""; }

	virtual bool init(context& ctx, size_t count) = 0;
	virtual double sort(context& ctx, GLuint data_buffer, GLuint value_buffer, const vec3& eye_pos, const vec3& view_dir, GLuint auxiliary_buffer = 0) = 0;
};

}
}

#include <cgv/config/lib_end.h>

