#include "gpu_sorter.h"

namespace cgv {
namespace glutil {

void gpu_sorter::create_buffer(GLuint& buffer, size_t size) {

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, (void*)0, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void gpu_sorter::delete_buffer(GLuint& buffer) {

	if(buffer != 0) {
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}
}

void gpu_sorter::delete_buffers() {

	delete_buffer(keys_in_ssbo);
	delete_buffer(keys_out_ssbo);
	delete_buffer(values_out_ssbo);
	delete_buffer(prefix_sum_ssbo);
	delete_buffer(blocksums_ssbo);
	delete_buffer(scratch_ssbo);
}

void gpu_sorter::begin_time_query() {

	time_query = 0;
	glGenQueries(1, &time_query);

	glBeginQuery(GL_TIME_ELAPSED, time_query);
}

void gpu_sorter::end_time_query() {

	glEndQuery(GL_TIME_ELAPSED);

	GLint done = false;
	while(!done) {
		glGetQueryObjectiv(time_query, GL_QUERY_RESULT_AVAILABLE, &done);
	}
	GLuint64 elapsed_time = 0;
	glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &elapsed_time);

	++measurements;
	float alpha = 1.0f / static_cast<float>(measurements);
	avg_time = (1.0f - alpha) * avg_time + alpha * (elapsed_time / 1000000.0f);

	if(measurements == 32) {
		std::cout << "Sorting done in " << avg_time << " ms -> " << static_cast<float>(n) / (1000.0f * avg_time) << " M/s" << std::endl;
		measurements = 0;
		avg_time = 0.0f;
	}
}

void gpu_sorter::set_value_format(cgv::type::info::TypeId type, unsigned component_count) {

	if(component_count < 1) {
		std::cout << "gpu_sorter::set_value_format() ... cannot have 0 components, using 1 as default" << std::endl;
		component_count = 1;
	}

	if(component_count > 4) {
		std::cout << "gpu_sorter::set_value_format() ... cannot have more than 4 components, using 4 as default" << std::endl;
		component_count = 4;
	}

	value_component_count = component_count;

	std::string type_prefix = "";

	switch(type) {
	case TI_UINT32:
	{
		if(component_count > 1) value_type_def = "uvec" + std::to_string(component_count);
		else value_type_def = "uint";
		break;
	}
	case TI_INT32:
	{
		if(component_count > 1) value_type_def = "ivec" + std::to_string(component_count);
		else value_type_def = "int";
		break;
	}
	case TI_FLT32:
	{
		if(component_count > 1) value_type_def = "vec" + std::to_string(component_count);
		else value_type_def = "float";
		break;
	}
	default:
		std::cout << "gpu_sorter::set_value_format() ... value type not supported, using unsigned int (uint)" << std::endl;
		if(component_count > 1) value_type_def = "uvec" + std::to_string(component_count);
		else value_type_def = "uint";
		value_type = TI_UINT32;
		return;
	}

	value_type = type;
}

void gpu_sorter::set_auxiliary_type_override(const std::string& def) {

	auxiliary_type_def = def;
}

void gpu_sorter::set_data_type_override(const std::string& def) {

	data_type_def = def;
}

void gpu_sorter::set_key_definition_override(const std::string& def) {

	key_definition = def;
}

}
}
