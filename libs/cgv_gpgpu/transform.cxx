#include "transform.h"

#include <queue>

namespace cgv {
namespace gpgpu {

transform::transform() : algorithm("transform") {
	register_kernel(kernel, "gpgpu_transform");
}

bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const std::string& unary_operation) {
	// TODO: remove explicit type
	return init(ctx, input_type, output_type, sl::named_variable_list{}, unary_operation);
}

bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const std::string& unary_operation) {
	if(!input_type.is_valid() || !output_type.is_valid())
		return false;
	cgv::render::shader_compile_options config = get_configuration(input_type, output_type, arguments, unary_operation);
	return init_kernels(ctx, config);
}





bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const compute_kernel_arguments_declaration& arguments, const std::string& unary_operation) {
	if(!input_type.is_valid() || !output_type.is_valid())
		return false;
	//cgv::render::shader_compile_options config = get_configuration(input_type, output_type, arguments, unary_operation);


	cgv::render::shader_compile_options config;
	//config.snippets.push_back({ "input_type_def", sl::get_typedef_str("input_type", input_type) });
	//config.snippets.push_back({ "output_type_def", sl::get_typedef_str("output_type", output_type) });
	config.snippets.push_back({ "operation", unary_operation });


	
	buffer_binding_indices.clear();
	size_t buffer_binding_index = base_buffer_binding_index;
	for(const sl::named_buffer& buffer : arguments.buffers)
		buffer_binding_indices[buffer.name()] = buffer_binding_index++;



	std::set<std::string> unique_types;
	std::queue<const sl::data_type*> types_queue;
	std::vector<const sl::data_type*> ordered_types;

	const auto& push_type_if_not_basic = [&types_queue](const sl::data_type* type) {
		if(!type->is_basic_type())
			types_queue.push(type);
	};

	const auto& push_types = [&push_type_if_not_basic, &types_queue](const sl::named_variable_list& variables) {
		for(const sl::named_variable& variable : variables) {
			push_type_if_not_basic(variable.type());
			//if(!variable.type()->is_basic_type())
			//	types_queue.push(variable.type());
		}
	};


	push_type_if_not_basic(&input_type);
	push_type_if_not_basic(&output_type);

	push_types(arguments.uniforms);
	for(const sl::named_buffer& buffer : arguments.buffers) {
		push_types(buffer.variables());
	}
	
	while(!types_queue.empty()) {
		const sl::data_type* type = types_queue.front();
		types_queue.pop();

		if(unique_types.find(type->type_name()) == unique_types.end()) {
			unique_types.insert(type->type_name());
			push_types(type->members());
			ordered_types.push_back(type);
		} else {
			// TODO: type with same name, check pointer for equality to report error on duplicate type definition
		}
	}

	std::string typedefs;
	typedefs += sl::get_typedef_str("input_type", input_type) + "\n";
	typedefs += sl::get_typedef_str("output_type", output_type) + "\n";

	for(auto it = ordered_types.rbegin(); it != ordered_types.rend(); ++it) {
		const sl::data_type* type = *it;
		typedefs += sl::get_typedef_str(type->type_name(), *type) + "\n";
	}


	std::string uniforms = to_string(arguments.uniforms, "uniform");
	std::string buffers = to_string(arguments.buffers, base_buffer_binding_index);

	std::string args = uniforms + "\n" + buffers;
	
	config.snippets.push_back({ "typedefs", typedefs });

	config.snippets.push_back({ "arguments", args });
	


	return init_kernels(ctx, config);
}







bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const ckadl& arguments, const std::string& unary_operation) {
	if(!input_type.is_valid() || !output_type.is_valid())
		return false;
	//cgv::render::shader_compile_options config = get_configuration(input_type, output_type, arguments, unary_operation);


	cgv::render::shader_compile_options config;
	//config.snippets.push_back({ "input_type_def", sl::get_typedef_str("input_type", input_type) });
	//config.snippets.push_back({ "output_type_def", sl::get_typedef_str("output_type", output_type) });
	config.snippets.push_back({ "operation", unary_operation });



	buffer_binding_indices.clear();
	size_t buffer_binding_index = base_buffer_binding_index;
	for(const sl::named_buffer& buffer : arguments.buffers)
		buffer_binding_indices[buffer.name()] = buffer_binding_index++;



	std::set<std::string> unique_types;
	std::queue<const sl::data_type*> types_queue;
	std::vector<const sl::data_type*> ordered_types;

	const auto& push_type_if_not_basic = [&types_queue](const sl::data_type* type) {
		if(!type->is_basic_type())
			types_queue.push(type);
	};

	const auto& push_types = [&push_type_if_not_basic, &types_queue](const sl::named_variable_list& variables) {
		for(const sl::named_variable& variable : variables) {
			push_type_if_not_basic(variable.type());
			//if(!variable.type()->is_basic_type())
			//	types_queue.push(variable.type());
		}
	};


	push_type_if_not_basic(&input_type);
	push_type_if_not_basic(&output_type);

	push_types(arguments.uniforms);
	for(const sl::named_buffer& buffer : arguments.buffers) {
		push_types(buffer.variables());
	}

	while(!types_queue.empty()) {
		const sl::data_type* type = types_queue.front();
		types_queue.pop();

		if(unique_types.find(type->type_name()) == unique_types.end()) {
			unique_types.insert(type->type_name());
			push_types(type->members());
			ordered_types.push_back(type);
		} else {
			// TODO: type with same name, check pointer for equality to report error on duplicate type definition
		}
	}

	std::string typedefs;
	typedefs += sl::get_typedef_str("input_type", input_type) + "\n";
	typedefs += sl::get_typedef_str("output_type", output_type) + "\n";

	for(auto it = ordered_types.rbegin(); it != ordered_types.rend(); ++it) {
		const sl::data_type* type = *it;
		typedefs += sl::get_typedef_str(type->type_name(), *type) + "\n";
	}


	std::string uniforms = to_string(arguments.uniforms, "uniform");
	std::string buffers = to_string(arguments.buffers, base_buffer_binding_index);

	std::string args = uniforms + "\n" + buffers;

	config.snippets.push_back({ "typedefs", typedefs });

	config.snippets.push_back({ "arguments", args });



	return init_kernels(ctx, config);
}











bool transform::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* input_buffer, const cgv::render::vertex_buffer* output_buffer, size_t count, const compute_kernel_arguments& arguments) {
	input_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	kernel.enable(ctx);
	kernel.set_argument(ctx, "u_count", static_cast<uint32_t>(count));
	kernel.set_arguments(ctx, arguments);

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = div_round_up(static_cast<uint32_t>(count), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	kernel.disable(ctx);

	input_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	return true;
}






bool transform::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* input_buffer, const cgv::render::vertex_buffer* output_buffer, size_t count, const compute_kernel_arguments2& arguments) {
	input_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	kernel.enable(ctx);
	kernel.set_argument(ctx, "u_count", static_cast<uint32_t>(count));
	kernel.set_arguments(ctx, arguments);

	for(size_t i = 0; i < arguments.get_buffer_count(); ++i) {
		const buffer_binding& binding = arguments.get_buffer(i);
		auto it = buffer_binding_indices.find(binding.name());
		if(it != buffer_binding_indices.end())
			binding.bind(ctx, it->second);
	}

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = div_round_up(static_cast<uint32_t>(count), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	kernel.disable(ctx);

	input_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	for(size_t i = 0; i < arguments.get_buffer_count(); ++i) {
		const buffer_binding& binding = arguments.get_buffer(i);
		binding.unbind(ctx);
	}

	return true;
}









cgv::render::shader_compile_options transform::get_configuration(const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const std::string& unary_operation) const {
	cgv::render::shader_compile_options config;

	// TODO: Ensure type definition order and definition of nested struct types.
	// Ensure data types are only defined once (e.g. when input type uses the same data type as one of the buffers or uniforms). Must be sorted out on algorithm level.

	config.snippets.push_back({ "input_type_def", sl::get_typedef_str("input_type", input_type) });
	config.snippets.push_back({ "output_type_def", sl::get_typedef_str("output_type", output_type) });
	config.snippets.push_back({ "operation", unary_operation });
	config.snippets.push_back({ "arguments", to_string(arguments, "uniform") });
	return config;
}

} // namespace gpgpu
} // namespace cgv
