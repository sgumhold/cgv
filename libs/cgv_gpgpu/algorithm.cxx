#include "algorithm.h"

#include "type_compiler.h"

namespace cgv {
namespace gpgpu {

GLbitfield get_associated_memory_barrier_bits(cgv::render::VertexBufferType buffer_type) {
	static const std::array<GLbitfield, 9> bits = {
		GL_ALL_BARRIER_BITS,
		GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,
		GL_ELEMENT_ARRAY_BARRIER_BIT,
		GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT,
		GL_UNIFORM_BARRIER_BIT,
		GL_TRANSFORM_FEEDBACK_BARRIER_BIT,
		GL_SHADER_STORAGE_BARRIER_BIT,
		GL_ATOMIC_COUNTER_BARRIER_BIT,
		GL_COMMAND_BARRIER_BIT
	};
	return bits[static_cast<int32_t>(buffer_type) + 1]; // Add 1 since the enum starts at -1.
}

std::string algorithm::get_type_name() const {
	return _type_name;
}

bool algorithm::is_initialized() const {
	return _is_initialized;
}

cgv::render::shader_compile_options algorithm::get_compile_options(const algorithm_create_info& create_info) {
	type_compiler compiler;
	compiler.add_types(create_info.types);

	std::string arguments_str = "";

	if(create_info.arguments) {
		const argument_definitions* arguments = create_info.arguments;

		compiler.add_types(arguments->uniforms);
		for(const sl::named_buffer& buffer : arguments->buffers)
			compiler.add_types(buffer.variables());

		arguments_str += to_string(arguments->uniforms, "uniform") + "\n";
		arguments_str += to_string(arguments->buffers, create_info.default_buffer_count) + "\n";
		arguments_str += to_string(arguments->images, create_info.default_image_count) + "\n";
		arguments_str += to_string(arguments->textures, create_info.default_texture_count) + "\n";

		_buffer_binding_indices.clear();
		uint32_t buffer_binding_index = create_info.default_buffer_count;
		for(const sl::named_buffer& buffer : arguments->buffers)
			_buffer_binding_indices[buffer.name()] = buffer_binding_index++;

		_image_binding_indices.clear();
		uint32_t image_binding_index = create_info.default_image_count;
		for(const sl::named_image& image : arguments->images)
			_image_binding_indices[image.name()] = image_binding_index++;

		_texture_binding_indices.clear();
		uint32_t texture_binding_index = create_info.default_texture_count;
		for(const sl::named_texture& texture : arguments->textures)
			_texture_binding_indices[texture.name()] = texture_binding_index++;
	}

	compile_result compiled = compiler.compile();

	if(compiled.err)
		_last_error = compiled.err;

	std::string typedefs_str = compiled.str;

	for(const auto& def : create_info.typedefs)
		typedefs_str += sl::get_type_alias_string(def.first, def.second) + "\n";

	cgv::render::shader_compile_options options;
	options.define_macro("LOCAL_SIZE_X", _group_size);
	options.define_macro("LOCAL_SIZE_Y", 1);
	options.define_macro("LOCAL_SIZE_Z", 1);
	options.define_snippet("typedefs", typedefs_str);
	options.define_snippet("arguments", arguments_str);

	options.extend(create_info.options, true);
	
	return options;
}

bool algorithm::init(cgv::render::context& ctx, const algorithm_create_info& create_info, const std::vector<compute_kernel_info>& kernel_infos) {
	for(const sl::data_type& type : create_info.types) {
		if(!type.is_valid()) {
			raise_error(errc::invalid_type, get_type_error_description(type));
			return false;
		}
	}

	cgv::render::shader_compile_options options = get_compile_options(create_info);
	if(_last_error)
		return false;

	const std::string debug_context = "cgv::gpgpu::" + get_type_name();
	bool success = true;
	_is_initialized = false;

	for(const auto& info : kernel_infos) {
		if(info.options.empty()) {
			success &= info.kernel->init(ctx, info.name, options, debug_context);
		} else {
			cgv::render::shader_compile_options extended_options = options;
			extended_options.extend(info.options, true);
			success &= info.kernel->init(ctx, info.name, extended_options, debug_context);
		}
		
		if(!success) {
			raise_error(errc::kernel_not_initialized, info.name);
			return false;
		}
	}
	_is_initialized = success;
	return success;
}

void algorithm::destruct(const cgv::render::context& ctx) {
	_is_initialized = false;
}

bool algorithm::is_valid_range(device_buffer_iterator first, device_buffer_iterator last) const {
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

void algorithm::dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z) {
	glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

void algorithm::optional_memory_barrier(cgv::render::VertexBufferType buffer_type) {
	if(buffer_type != cgv::render::VertexBufferType::VBT_STORAGE)
		glMemoryBarrier(get_associated_memory_barrier_bits(buffer_type));
}

void algorithm::raise_error(errc e, const std::string& description) {
	_last_error = { e, description };
}

void algorithm::raise_unsupported_type_error(const sl::data_type& type) {
	raise_error(errc::type_not_supported, get_type_error_description(type));
}

void algorithm::raise_not_enough_shared_memory_error(size_t available, size_t required) {
	std::string description = "can only fit " + std::to_string(available) + " of " + std::to_string(required) + " elements; consider reducing group size";
	raise_error(errc::not_enough_shared_memory, description);
}

std::string algorithm::get_type_error_description(const sl::data_type& type) const {
	std::string name = type.type_name();
	if(name.empty())
		return "<empty_name>";
	if(type.is_compound())
		return std::string("struct ") + name;
	return name;
}

} // namespace gpgpu
} // namespace cgv
