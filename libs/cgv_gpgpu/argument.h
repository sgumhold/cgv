#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "sl.h"
#include "binding.h"
#include "representation.h"

namespace cgv {
namespace gpgpu {

enum class ArgumentType {
	Uniform,
	Buffer,
	Image,
	Texture
};

class argument_definition {
	friend struct argument_definitions;

public:
	argument_definition(const sl::data_type& type, const std::string& name) : _variable(type, name) {}

	argument_definition(const sl::data_type& type, const std::string& name, size_t array_size) : _variable(type, name, array_size) {}

	argument_definition(sl::tag::buffer, const sl::named_variable& variable, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", { variable } }, name), _memory_qualifiers(memory_qualifiers), _type(ArgumentType::Buffer) {}

	argument_definition(sl::tag::buffer, const sl::named_variable_list& variables, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", variables }, name), _memory_qualifiers(memory_qualifiers), _type(ArgumentType::Buffer) {}

	argument_definition(sl::tag::image, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable(sl::Type::Void, name), _texture_type(texture_type), _image_format(image_format), _memory_qualifiers(memory_qualifiers), _type(ArgumentType::Image) {}

	argument_definition(sl::tag::texture, cgv::render::TextureType texture_type, sl::SamplerBaseFormat sampler_base_format, const std::string& name) :
		_variable(sl::Type::Void, name), _texture_type(texture_type), _sampler_base_format(sampler_base_format), _type(ArgumentType::Texture) {}

private:
	sl::named_variable _variable;
	cgv::render::TextureType _texture_type = cgv::render::TextureType::TT_UNDEF;
	sl::SamplerBaseFormat _sampler_base_format = sl::SamplerBaseFormat::FloatingPoint;
	sl::ImageFormatLayoutQualifier _image_format = sl::ImageFormatLayoutQualifier::rgba8;
	sl::memory_qualifier_storage _memory_qualifiers;
	ArgumentType _type = ArgumentType::Uniform;
};

struct argument_definitions {
	argument_definitions() {}
	argument_definitions(std::initializer_list<argument_definition> arguments) : argument_definitions(arguments.begin(), arguments.end()) {}
	argument_definitions(const std::vector<argument_definition>& arguments) : argument_definitions(arguments.begin(), arguments.end()) {}

	template<class Iterator>
	argument_definitions(const Iterator begin, const Iterator end) {
		for(Iterator it = begin; it != end; ++it) {
			const argument_definition& argument = *it;
			switch(argument._type) {
			case ArgumentType::Uniform:
				uniforms.push_back(argument._variable);
				break;
			case ArgumentType::Buffer:
				buffers.push_back(sl::named_buffer(argument._variable.type().members(), argument._variable.name(), argument._memory_qualifiers.list()));
				break;
			case ArgumentType::Image:
				images.push_back(sl::named_image(argument._texture_type, argument._image_format, argument._variable.name(), argument._memory_qualifiers.list()));
				break;
			case ArgumentType::Texture:
				textures.push_back(sl::named_texture(argument._texture_type, argument._sampler_base_format, argument._variable.name()));
				break;
			default:
				break;
			}
		}
	}

	sl::named_variable_list uniforms;
	sl::named_buffer_list buffers;
	sl::named_image_list images;
	sl::named_texture_list textures;
};

class argument_bindings {
public:
	virtual size_t get_uniform_count() const {
		return 0;
	}

	virtual const uniform_binding* get_uniform(size_t index) const {
		return nullptr;
	}

	virtual size_t get_buffer_count() const {
		return 0;
	}

	virtual const buffer_binding* get_buffer(size_t index) const {
		return nullptr;
	}

	virtual size_t get_image_count() const {
		return 0;
	}

	virtual const image_binding* get_image(size_t index) const {
		return nullptr;
	}

	virtual size_t get_texture_count() const {
		return 0;
	}

	virtual const texture_binding* get_texture(size_t index) const {
		return nullptr;
	}
};

class argument_binding_list : public argument_bindings {
public:
	void bind_uniform(const std::string& name, bool value) {
		_uniform_bindings.emplace_back(name, value);
	}

	void bind_uniform(const std::string& name, int32_t value) {
		_uniform_bindings.emplace_back(name, value);
	}

	void bind_uniform(const std::string& name, uint32_t value) {
		_uniform_bindings.emplace_back(name, value);
	}

	void bind_uniform(const std::string& name, float value) {
		_uniform_bindings.emplace_back(name, value);
	}

	void bind_uniform(const std::string& name, double value) {
		_uniform_bindings.emplace_back(name, value);
	}

	template<typename T, cgv::type::uint32_type N>
	void bind_uniform(const std::string& name, const cgv::math::fvec<T, N>* value) {
		_uniform_bindings.emplace_back(name, value);
	}

	template<typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
	void bind_uniform(const std::string& name, const cgv::math::fmat<T, N, M>* value) {
		_uniform_bindings.emplace_back(name, value);
	}

	template<typename T, typename std::enable_if<type_representation<T>::value, bool>::type = true>
	void bind_uniform(const std::string& name, const T& value) {
		type_representation<T>::create_uniform_binding(_uniform_bindings, name, value);
	}

	template<typename T>
	void bind_uniform(const std::string& name, const sl::data_type& type, T value) {
		_uniform_bindings.emplace_back(name, type, value);
	}

	template<typename T, typename std::enable_if<std::is_base_of<cgv::render::vertex_buffer, T>::value, bool>::type = true>
	void bind_buffer(const std::string& name, const T& buffer) {
		_buffer_bindings.emplace_back(name, buffer);
	}

	void bind_image(const std::string& name, const cgv::render::texture& texture) {
		_image_bindings.emplace_back(name, const_cast<cgv::render::texture&>(texture));
	}

	void bind_texture(const std::string& name, const cgv::render::texture& texture) {
		_texture_bindings.emplace_back(name, const_cast<cgv::render::texture&>(texture));
	}

	size_t get_uniform_count() const override {
		return _uniform_bindings.size();
	}

	const uniform_binding* get_uniform(size_t index) const override {
		return &_uniform_bindings[index];
	}

	size_t get_buffer_count() const override {
		return _buffer_bindings.size();
	}

	const buffer_binding* get_buffer(size_t index) const override {
		return &_buffer_bindings[index];
	}

	size_t get_image_count() const override {
		return _image_bindings.size();
	}

	const image_binding* get_image(size_t index) const override {
		return &_image_bindings[index];
	}

	size_t get_texture_count() const override {
		return _texture_bindings.size();
	}

	const texture_binding* get_texture(size_t index) const override {
		return &_texture_bindings[index];
	}

private:
	uniform_binding_list _uniform_bindings;
	buffer_binding_list _buffer_bindings;
	image_binding_list _image_bindings;
	texture_binding_list _texture_bindings;
};

class argument_binding_struct : public argument_bindings {
public:
	argument_binding_struct() {}

	argument_binding_struct(const argument_binding_struct& other) = delete;
	
	argument_binding_struct& operator=(const argument_binding_struct& other) = delete;

	size_t get_uniform_count() const override {
		return _uniform_bindings.size();
	}

	const uniform_binding* get_uniform(size_t index) const override {
		return _uniform_bindings[index];
	}

	size_t get_buffer_count() const override {
		return _buffer_bindings.size();
	}

	const buffer_binding* get_buffer(size_t index) const override {
		return _buffer_bindings[index];
	}

	size_t get_image_count() const override {
		return _image_bindings.size();
	}

	const image_binding* get_image(size_t index) const override {
		return _image_bindings[index];
	}

	size_t get_texture_count() const override {
		return _texture_bindings.size();
	}

	const texture_binding* get_texture(size_t index) const override {
		return _texture_bindings[index];
	}

protected:
	void connect(std::initializer_list<uniform_binding*> uniforms) {
		_uniform_bindings = uniforms;
	}

	void connect(std::initializer_list<buffer_binding*> buffers) {
		_buffer_bindings = buffers;
	}

	void connect(std::initializer_list<image_binding*> images) {
		_image_bindings = images;
	}

	void connect(std::initializer_list<texture_binding*> textures) {
		_texture_bindings = textures;
	}

private:
	std::vector<uniform_binding*> _uniform_bindings;
	std::vector<buffer_binding*> _buffer_bindings;
	std::vector<image_binding*> _image_bindings;
	std::vector<texture_binding*> _texture_bindings;
};

} // namespace gpgpu
} // namespace cgv
