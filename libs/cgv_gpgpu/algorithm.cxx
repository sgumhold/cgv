#include "algorithm.h"

#include <queue>

namespace cgv {
namespace gpgpu {

namespace detail {

class type_compiler {
public:
	void add(const sl::data_type& type) {
		if(type.is_compound())
			_types.push_back(type);
	}

	void add(const std::vector<sl::data_type>& types) {
		for(const sl::data_type& type : types)
			add(type);
	}

	void add(const sl::named_variable_list& variables) {
		for(const sl::named_variable& variable : variables)
			add(variable.type());
	}

	std::string compile() {
		// TODO: Ensure correct type definition order.
		std::set<std::string> unique_types;
		std::queue<sl::data_type> types_queue;
		std::vector<sl::data_type> ordered_types;

		for(const auto& type : _types)
			types_queue.push(type);
		
		while(!types_queue.empty()) {
			sl::data_type type = types_queue.front();
			types_queue.pop();

			if(unique_types.find(type.type_name()) == unique_types.end()) {
				unique_types.insert(type.type_name());
				ordered_types.push_back(type);
					
				for(const sl::named_variable& member : type.members()) {
					if(member.type().is_compound())
						types_queue.push(member.type());
				}
			} else {
				// TODO: Throw error if a type with the same name already exists?
			}
		}

		std::string typedefs;

		for(auto it = ordered_types.rbegin(); it != ordered_types.rend(); ++it) {
			const sl::data_type& type = *it;
			typedefs += sl::get_type_definition_string(type) + "\n";
		}

		return typedefs;
	}

private:
	std::vector<sl::data_type> _types;
};

}

std::string algorithm::get_type_name() const {
	return _type_name;
}

bool algorithm::is_initialized() const {
	return _is_initialized;
}

bool algorithm::init(cgv::render::context& ctx, const std::vector<compute_kernel_info>& kernel_infos, const cgv::render::shader_compile_options& config) {
	const std::string debug_context = "cgv::gpgpu::" + get_type_name();
	bool success = true;
	for(const auto& info : kernel_infos) {
		if(info.defines.empty()) {
			success &= info.kernel->init(ctx, info.name, config, debug_context);
		} else {
			cgv::render::shader_compile_options extended_config = config;
			for(const auto& define : info.defines)
				extended_config.defines[define.first] = define.second;
			success &= info.kernel->init(ctx, info.name, extended_config, debug_context);
		}
	}
	_is_initialized = success;
	return success;
}

void algorithm::destruct(const cgv::render::context& ctx) {
	_is_initialized = false;
}

void algorithm::set_buffer_binding_indices(const sl::named_buffer_list& buffers, uint32_t base_index) {
	_base_buffer_binding_index = base_index;
	_buffer_binding_indices.clear();
	uint32_t buffer_binding_index = _base_buffer_binding_index;
	for(const sl::named_buffer& buffer : buffers)
		_buffer_binding_indices[buffer.name()] = buffer_binding_index++;
}

void algorithm::set_image_binding_indices(const sl::named_image_list& images, uint32_t base_index) {
	_base_image_binding_index = base_index;
	_image_binding_indices.clear();
	uint32_t image_binding_index = _base_image_binding_index;
	for(const sl::named_image& image : images)
		_image_binding_indices[image.name()] = image_binding_index++;
}

void algorithm::set_texture_binding_indices(const sl::named_texture_list& textures, uint32_t base_index) {
	_base_texture_binding_index = base_index;
	_texture_binding_indices.clear();
	uint32_t texture_binding_index = _base_texture_binding_index;
	for(const sl::named_texture& texture : textures)
		_texture_binding_indices[texture.name()] = texture_binding_index++;
}

cgv::render::shader_compile_options algorithm::configure(const algorithm_configuration_info& info) {
	detail::type_compiler compiler;
	compiler.add(info.types);

	std::string arguments_str = "";

	if(info.arguments) {
		compiler.add(info.arguments->uniforms);
		for(const sl::named_buffer& buffer : info.arguments->buffers)
			compiler.add(buffer.variables());

		arguments_str += to_string(info.arguments->uniforms, "uniform") + "\n";
		arguments_str += to_string(info.arguments->buffers, info.default_buffer_count) + "\n";
		arguments_str += to_string(info.arguments->images, info.default_image_count) + "\n";
		arguments_str += to_string(info.arguments->textures, info.default_texture_count) + "\n";

		_buffer_binding_indices.clear();
		uint32_t buffer_binding_index = info.default_buffer_count;
		for(const sl::named_buffer& buffer : info.arguments->buffers)
			_buffer_binding_indices[buffer.name()] = buffer_binding_index++;

		_image_binding_indices.clear();
		uint32_t image_binding_index = info.default_image_count;
		for(const sl::named_image& image : info.arguments->images)
			_image_binding_indices[image.name()] = image_binding_index++;

		_texture_binding_indices.clear();
		uint32_t texture_binding_index = info.default_texture_count;
		for(const sl::named_texture& texture : info.arguments->textures)
			_texture_binding_indices[texture.name()] = texture_binding_index++;
	}

	std::string typedefs_str = compiler.compile();

	cgv::render::shader_compile_options config;
	config.snippets.push_back({ "typedefs", typedefs_str });
	config.snippets.push_back({ "arguments", arguments_str });
	return config;

}

cgv::render::shader_compile_options algorithm::get_configuration(const argument_definitions& arguments, const std::vector<sl::data_type> types) const {
	detail::type_compiler compiler;
	compiler.add(types);
	compiler.add(arguments.uniforms);
	for(const sl::named_buffer& buffer : arguments.buffers)
		compiler.add(buffer.variables());

	std::string typedefs = compiler.compile();

	std::string uniforms = to_string(arguments.uniforms, "uniform");
	std::string buffers = to_string(arguments.buffers, _base_buffer_binding_index);
	std::string images = to_string(arguments.images, _base_image_binding_index);
	std::string textures = to_string(arguments.textures, _base_texture_binding_index);

	cgv::render::shader_compile_options config;
	config.snippets.push_back({ "typedefs", typedefs });
	config.snippets.push_back({ "arguments", uniforms + "\n" + buffers + "\n" + images + "\n" + textures });
	return config;
}

bool algorithm::is_valid_range(device_buffer_iterator first, device_buffer_iterator last) {
	return compatible(first, last) && distance(first, last) > 0;
}

void algorithm::bind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	for(size_t i = 0; i < arguments.get_buffer_count(); ++i) {
		const buffer_binding* binding = arguments.get_buffer(i);
		auto it = _buffer_binding_indices.find(binding->name());
		if(it != _buffer_binding_indices.end())
			binding->bind(ctx, it->second);
	}

	for(size_t i = 0; i < arguments.get_image_count(); ++i) {
		const image_binding* binding = arguments.get_image(i);
		auto it = _image_binding_indices.find(binding->name());
		if(it != _image_binding_indices.end())
			binding->bind(ctx, it->second);
	}

	for(size_t i = 0; i < arguments.get_texture_count(); ++i) {
		const texture_binding* binding = arguments.get_texture(i);
		auto it = _texture_binding_indices.find(binding->name());
		if(it != _texture_binding_indices.end())
			binding->bind(ctx, it->second);
	}
}

void algorithm::unbind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	for(size_t i = 0; i < arguments.get_buffer_count(); ++i)
		arguments.get_buffer(i)->unbind(ctx);

	for(size_t i = 0; i < arguments.get_image_count(); ++i)
		arguments.get_image(i)->unbind(ctx);

	for(size_t i = 0; i < arguments.get_texture_count(); ++i)
		arguments.get_texture(i)->unbind(ctx);
}

void algorithm::bind_buffer_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	for(size_t i = 0; i < arguments.get_buffer_count(); ++i) {
		const buffer_binding* binding = arguments.get_buffer(i);
		auto it = _buffer_binding_indices.find(binding->name());
		if(it != _buffer_binding_indices.end())
			binding->bind(ctx, it->second);
	}
}

void algorithm::unbind_buffer_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	for(size_t i = 0; i < arguments.get_buffer_count(); ++i) {
		const buffer_binding* binding = arguments.get_buffer(i);
		binding->unbind(ctx);
	}
}

void algorithm::dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z) {
	glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}
/*
void algorithm::bind_image_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	for(size_t i = 0; i < arguments.get_image_count(); ++i) {
		const image_binding* binding = arguments.get_image(i);
		auto it = _image_binding_indices.find(binding->name());
		if(it != _image_binding_indices.end())
			binding->bind(ctx, it->second);
	}
}

void algorithm::unbind_image_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	for(size_t i = 0; i < arguments.get_image_count(); ++i) {
		const image_binding* binding = arguments.get_image(i);
		binding->unbind(ctx);
	}
}
*/

} // namespace gpgpu
} // namespace cgv
