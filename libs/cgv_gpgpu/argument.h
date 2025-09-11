#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "sl.h"
#include "binding.h"

namespace cgv {
namespace gpgpu {

// TODO: provide iterator (or vector) initializers in all constructors with initializer lists.

enum class ArgumentType {
	kUniform,
	kBuffer,
	kImage,
	kTexture
};

class argument_definition {
	friend struct argument_definitions;

public:
	argument_definition(const sl::data_type& type, const std::string& name) : _variable(type, name) {}

	argument_definition(const sl::data_type& type, const std::string& name, size_t array_size) : _variable(type, name, array_size) {}

	argument_definition(sl::tag::buffer, const sl::named_variable& variable, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", { variable } }, name), _memory_qualifiers(memory_qualifiers), _type(ArgumentType::kBuffer) {}

	argument_definition(sl::tag::buffer, const sl::named_variable_list& variables, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", variables }, name), _memory_qualifiers(memory_qualifiers), _type(ArgumentType::kBuffer) {}

	argument_definition(sl::tag::image, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable(sl::Type::kVoid, name), _texture_type(texture_type), _image_format(image_format), _memory_qualifiers(memory_qualifiers), _type(ArgumentType::kImage) {}

	argument_definition(sl::tag::texture, cgv::render::TextureType texture_type, sl::SamplerBaseFormat sampler_base_format, const std::string& name) :
		_variable(sl::Type::kVoid, name), _texture_type(texture_type), _sampler_base_format(sampler_base_format), _type(ArgumentType::kTexture) {}

private:
	sl::named_variable _variable;
	cgv::render::TextureType _texture_type = cgv::render::TextureType::TT_UNDEF;
	sl::SamplerBaseFormat _sampler_base_format = sl::SamplerBaseFormat::kFloatingPoint;
	sl::ImageFormatLayoutQualifier _image_format = sl::ImageFormatLayoutQualifier::k_rgba8;
	sl::memory_qualifier_storage _memory_qualifiers;
	ArgumentType _type = ArgumentType::kUniform;
};

struct argument_definitions {
	argument_definitions() {}

	argument_definitions(std::initializer_list<argument_definition> arguments) {
		for(const argument_definition& argument : arguments) {
			switch(argument._type) {
			case ArgumentType::kUniform:
				uniforms.push_back(argument._variable);
				break;
			case ArgumentType::kBuffer:
				buffers.push_back(sl::named_buffer(argument._variable.type().members(), argument._variable.name(), argument._memory_qualifiers.list()));
				break;
			case ArgumentType::kImage:
				images.push_back(sl::named_image(argument._texture_type, argument._image_format, argument._variable.name(), argument._memory_qualifiers.list()));
				break;
			case ArgumentType::kTexture:
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

static cgv::render::type_descriptor get_type_descriptor(sl::data_type type) {
	using namespace cgv::render;
	using namespace cgv::type;
	switch(type.type()) {
	case sl::Type::kBool: return element_descriptor_traits<bool>::get_type_descriptor({});
	case sl::Type::kInt: return element_descriptor_traits<int32_type>::get_type_descriptor({});
	case sl::Type::kUInt: return element_descriptor_traits<uint32_type>::get_type_descriptor({});
	case sl::Type::kFloat: return element_descriptor_traits<flt32_type>::get_type_descriptor({});
	case sl::Type::kDouble: return element_descriptor_traits<flt64_type>::get_type_descriptor({});
	case sl::Type::kBVec2: return element_descriptor_traits<bvec2>::get_type_descriptor({});
	case sl::Type::kBVec3: return element_descriptor_traits<bvec3>::get_type_descriptor({});
	case sl::Type::kBVec4: return element_descriptor_traits<bvec4>::get_type_descriptor({});
	case sl::Type::kIVec2: return element_descriptor_traits<ivec2>::get_type_descriptor({});
	case sl::Type::kIVec3: return element_descriptor_traits<ivec3>::get_type_descriptor({});
	case sl::Type::kIVec4: return element_descriptor_traits<ivec4>::get_type_descriptor({});
	case sl::Type::kUVec2: return element_descriptor_traits<uvec2>::get_type_descriptor({});
	case sl::Type::kUVec3: return element_descriptor_traits<uvec3>::get_type_descriptor({});
	case sl::Type::kUVec4: return element_descriptor_traits<uvec4>::get_type_descriptor({});
	case sl::Type::kVec2: return element_descriptor_traits<vec2>::get_type_descriptor({});
	case sl::Type::kVec3: return element_descriptor_traits<vec3>::get_type_descriptor({});
	case sl::Type::kVec4: return element_descriptor_traits<vec4>::get_type_descriptor({});
	case sl::Type::kDVec2: return element_descriptor_traits<dvec2>::get_type_descriptor({});
	case sl::Type::kDVec3: return element_descriptor_traits<dvec3>::get_type_descriptor({});
	case sl::Type::kDVec4: return element_descriptor_traits<dvec4>::get_type_descriptor({});
	case sl::Type::kMat2: return element_descriptor_traits<mat2>::get_type_descriptor({});
	case sl::Type::kMat3: return element_descriptor_traits<mat3>::get_type_descriptor({});
	case sl::Type::kMat4: return element_descriptor_traits<mat4>::get_type_descriptor({});
	case sl::Type::kMat2x3: return element_descriptor_traits<cgv::math::fmat<float, 2u, 3u>>::get_type_descriptor({});
	case sl::Type::kMat2x4: return element_descriptor_traits<cgv::math::fmat<float, 2u, 4u>>::get_type_descriptor({});
	case sl::Type::kMat3x2: return element_descriptor_traits<cgv::math::fmat<float, 3u, 2u>>::get_type_descriptor({});
	case sl::Type::kMat3x4: return element_descriptor_traits<cgv::math::fmat<float, 3u, 4u>>::get_type_descriptor({});
	case sl::Type::kMat4x2: return element_descriptor_traits<cgv::math::fmat<float, 4u, 2u>>::get_type_descriptor({});
	case sl::Type::kMat4x3: return element_descriptor_traits<cgv::math::fmat<float, 4u, 3u>>::get_type_descriptor({});
	case sl::Type::kDMat2: return element_descriptor_traits<dmat2>::get_type_descriptor({});
	case sl::Type::kDMat3: return element_descriptor_traits<dmat3>::get_type_descriptor({});
	case sl::Type::kDMat4: return element_descriptor_traits<dmat4>::get_type_descriptor({});
	case sl::Type::kDMat2x3: return element_descriptor_traits<cgv::math::fmat<double, 2u, 3u>>::get_type_descriptor({});
	case sl::Type::kDMat2x4: return element_descriptor_traits<cgv::math::fmat<double, 2u, 4u>>::get_type_descriptor({});
	case sl::Type::kDMat3x2: return element_descriptor_traits<cgv::math::fmat<double, 3u, 2u>>::get_type_descriptor({});
	case sl::Type::kDMat3x4: return element_descriptor_traits<cgv::math::fmat<double, 3u, 4u>>::get_type_descriptor({});
	case sl::Type::kDMat4x2: return element_descriptor_traits<cgv::math::fmat<double, 4u, 2u>>::get_type_descriptor({});
	case sl::Type::kDMat4x3: return element_descriptor_traits<cgv::math::fmat<double, 4u, 3u>>::get_type_descriptor({});
	default: return type_descriptor(cgv::type::info::type_id<void>::get_id());
	}
}

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
	template<typename T, typename std::enable_if<sl::traits::is_fundamental_sl_type<T>::value, bool>::type = true>
	void bind_uniform(const std::string& name, const T& value) {
		_uniform_bindings.emplace_back(name, value);
	}

	template<typename T, typename std::enable_if<sl::traits::is_fundamental_sl_type<T>::value, bool>::type = true>
	void bind_uniform(sl::data_type type, const std::string& name, const T& value) {
		_uniform_bindings.emplace_back(name, get_type_descriptor(type), &value);
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
