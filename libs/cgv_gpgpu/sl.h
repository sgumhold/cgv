#pragma once

#include <memory>
#include <string>
#include <vector>

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/interval.h>
#include <cgv/render/context.h>

#include "lib_begin.h"

namespace sl {

namespace traits {

// Todo: Make use of the inline keyword if the framework is ever updated to at least C++17.

template<class T>
/*inline*/ constexpr bool is_instance_of_fvec_v = std::false_type{};

template<class T, cgv::type::uint32_type N>
/*inline*/ constexpr bool is_instance_of_fvec_v<cgv::math::fvec<T, N>> = std::true_type{};

template<class T>
/*inline*/ constexpr bool is_instance_of_fmat_v = std::false_type{};

template<class T, cgv::type::uint32_type N, cgv::type::uint32_type M>
/*inline*/ constexpr bool is_instance_of_fmat_v<cgv::math::fmat<T, N, M>> = std::true_type{};

// TODO: Move is_instance_of(_v) to cgv/type/traits.
template<class T, template<class...> class U>
/*inline*/ constexpr bool is_instance_of_v = std::false_type{};

template<template<class...> class U, class ...Vs>
/*inline*/ constexpr bool is_instance_of_v<U<Vs...>, U> = std::true_type{};

template<class T>
/*inline*/ constexpr bool is_fundamental_sl_scalar_type_v =
std::is_same_v<T, bool> ||
std::is_same_v<T, std::int32_t> ||
std::is_same_v<T, std::uint32_t> ||
std::is_same_v<T, float> ||
std::is_same_v<T, double>;

template<class T>
struct is_fundamental_sl_scalar_type : std::bool_constant<is_fundamental_sl_scalar_type_v<T>> {};

template<class T>
/*inline*/ constexpr bool is_fundamental_sl_type_v =
is_fundamental_sl_scalar_type_v<T> ||
is_instance_of_fvec_v<T> ||
is_instance_of_fmat_v<T>;

template<class T>
struct is_fundamental_sl_type : std::bool_constant<is_fundamental_sl_type_v<T>> {};

} // namespace traits

enum class Type : int32_t {
	Void = 0,

	Bool,
	Int,
	UInt,
	Float,
	Double,

	BVec2,
	BVec3,
	BVec4,

	IVec2,
	IVec3,
	IVec4,

	UVec2,
	UVec3,
	UVec4,

	Vec2,
	Vec3,
	Vec4,

	DVec2,
	DVec3,
	DVec4,

	Mat2,
	Mat3,
	Mat4,

	Mat2x3,
	Mat2x4,
	Mat3x2,
	Mat3x4,
	Mat4x2,
	Mat4x3,

	DMat2,
	DMat3,
	DMat4,

	DMat2x3,
	DMat2x4,
	DMat3x2,
	DMat3x4,
	DMat4x2,
	DMat4x3,

	Struct
};

struct type_info {
	Type component_type;	// the base data type of vector or matrix components
	uint8_t column_count;	// the number of columns (1 for scalars and vectors)
	uint8_t row_count;		// the number of rows (1 for scalars)
	uint8_t base_size;		// according to layout std430, internal use only
	uint8_t base_alignment;	// according to layout std430
};

extern CGV_API std::string to_string(Type type);

extern CGV_API type_info get_type_info(Type type);

/// Indicator for variable size arrays that are typically used in buffers.
static constexpr auto varsize{ static_cast<size_t>(-1) };

// Forward declarations of types used in data_type.
struct type_definition;
class named_variable;
using named_variable_list = std::vector<named_variable>;

class CGV_API data_type {
public:
	data_type();

	data_type(Type type);

	data_type(const std::string& name, const named_variable_list& members);

	Type type() const;

	named_variable_list members() const;

	std::string type_name() const;

	bool is_valid() const;

	bool is_void() const;

	bool is_scalar() const;

	bool is_vector() const;

	bool is_matrix() const;

	bool is_compound() const;

	/// @brief Return the size of one instance of this data_type in bytes without padding for alignment.
	size_t size_in_bytes() const;

	/// @brief Return the memory alignment of this data_type in bytes according to GLSL layout std430.
	size_t alignment_in_bytes() const;

private:
	Type _base_type = Type::Void;
	std::shared_ptr<type_definition> _definition;
};

extern CGV_API size_t get_aligned_size(data_type type);

extern CGV_API std::string get_type_definition_string(data_type type);

extern CGV_API std::string get_alias_string(const std::string& alias, const std::string& type);

extern CGV_API std::string get_type_alias_string(const std::string& alias, data_type type);

extern CGV_API cgv::render::type_descriptor get_type_descriptor(const data_type& type);

template<class T>
bool matches_type(const data_type& type, const T& value) {
	if(!type.is_valid() || type.is_compound())
		return false;

	return get_type_descriptor(type) == cgv::render::element_descriptor_traits<T>::get_type_descriptor({});
}

struct type_definition {
	std::string type_name;
	named_variable_list members;
};

class named_object {
public:
	named_object(const std::string& name) : _name(name) {}

	const std::string& name() const {
		return _name;
	}

private:
	std::string _name;
};

class named_variable : public named_object {
public:
	named_variable(const data_type& type, const std::string& name) : named_object(name), _type(type) {}

	named_variable(const data_type& type, const std::string& name, size_t array_size) : named_variable(type, name) {
		_array_size = array_size;
	}

	const data_type& type() const {
		return _type;
	}

	size_t array_size() const {
		return _array_size;
	}

private:
	data_type _type;
	size_t _array_size = 0;
};

extern CGV_API std::string to_string(const named_variable& variable);

extern CGV_API std::string to_string(const named_variable_list& variables);

extern CGV_API std::string to_string(const named_variable_list& variables, const std::string& prefix);

enum class MemoryQualifier : int32_t {
	None = 0,
	Coherent = 1,
	Volatile = 2,
	Restrict = 4,
	ReadOnly = 8,
	WriteOnly = 16
};

extern CGV_API std::string to_string(MemoryQualifier qualifier);

using memory_qualifier_list = std::vector<MemoryQualifier>;

extern CGV_API std::string to_string(const memory_qualifier_list& qualifiers);

class CGV_API memory_qualifier_storage {
public:
	memory_qualifier_storage() {}

	memory_qualifier_storage(const memory_qualifier_list& qualifiers);

	memory_qualifier_list list() const;

private:
	int32_t _mask = 0;
};

class named_buffer : public named_object {
public:
	named_buffer(const named_variable_list& variables, const std::string& name, const memory_qualifier_list& memory_qualifiers = {}) : named_object(name), _variables(variables), _memory_qualifiers(memory_qualifiers) {}

	named_buffer(const named_variable& variable, const std::string& name, const memory_qualifier_list& memory_qualifiers = {}) : named_buffer(named_variable_list{ variable }, name, memory_qualifiers) {}

	const named_variable_list& variables() const {
		return _variables;
	}

	memory_qualifier_list memory_qualifiers() const {
		return _memory_qualifiers.list();
	}

private:
	named_variable_list _variables;
	memory_qualifier_storage _memory_qualifiers;
};

extern CGV_API std::string to_string(const named_buffer& buffer, size_t location);

using named_buffer_list = std::vector<named_buffer>;

extern CGV_API std::string to_string(const named_buffer_list& buffers, size_t base_location);

/// @brief Layout qualifiers for image formats (lower case names are used for better readability).
enum class ImageFormatLayoutQualifier : int32_t {
	// floating-point layout image formats
	rgba32f = 0,
	rgba16f,
	rg32f,
	rg16f,
	r11f_g11f_b10f,
	r32f,
	r16f,
	rgba16,
	rgb10_a2,
	rgba8,
	rg16,
	rg8,
	r16,
	r8,
	rgba16_snorm,
	rgba8_snorm,
	rg16_snorm,
	rg8_snorm,
	r16_snorm,
	r8_snorm,

	// signed integer layout image formats
	rgba32i,
	rgba16i,
	rgba8i,
	rg32i,
	rg16i,
	rg8i,
	r32i,
	r16i,
	r8i,

	// unsigned integer layout image formats
	rgba32ui,
	rgba16ui,
	rgb10_a2ui,
	rgba8ui,
	rg32ui,
	rg16ui,
	rg8ui,
	r32ui,
	r16ui,
	r8ui,
};

extern CGV_API std::string to_string(ImageFormatLayoutQualifier qualifier);

extern CGV_API std::string get_type_prefix(ImageFormatLayoutQualifier qualifier);

extern CGV_API data_type get_data_type(ImageFormatLayoutQualifier qualifier);

class named_image : public named_object {
public:
	named_image(cgv::render::TextureType texture_type, ImageFormatLayoutQualifier image_format, const std::string& name, const memory_qualifier_list& memory_qualifiers = {}) : named_object(name), _texture_type(texture_type), _image_format(image_format), _memory_qualifiers(memory_qualifiers) {}

	cgv::render::TextureType texture_type() const {
		return _texture_type;
	}

	ImageFormatLayoutQualifier image_format() const {
		return _image_format;
	}

	memory_qualifier_list memory_qualifiers() const {
		return _memory_qualifiers.list();
	}

private:
	cgv::render::TextureType _texture_type;
	ImageFormatLayoutQualifier _image_format;
	memory_qualifier_storage _memory_qualifiers;
};

extern CGV_API std::string to_string(const named_image& image, size_t location);

using named_image_list = std::vector<named_image>;

extern CGV_API std::string to_string(const named_image_list& images, size_t base_location);

enum class SamplerBaseFormat {
	FloatingPoint,
	SignedInteger,
	UnsignedInteger
};

class named_texture : public named_object {
public:
	named_texture(cgv::render::TextureType texture_type, SamplerBaseFormat sampler_base_format, const std::string& name) : named_object(name), _texture_type(texture_type), _sampler_base_format(sampler_base_format) {}

	cgv::render::TextureType texture_type() const {
		return _texture_type;
	}

	SamplerBaseFormat sampler_base_format() const {
		return _sampler_base_format;
	}

private:
	cgv::render::TextureType _texture_type;
	SamplerBaseFormat _sampler_base_format;
};

extern CGV_API std::string get_sampler_string(const cgv::render::TextureType& texture_type, SamplerBaseFormat sampler_base_format);

extern CGV_API std::string to_string(const named_texture& texture, size_t location);

using named_texture_list = std::vector<named_texture>;

extern CGV_API std::string to_string(const named_texture_list& textures, size_t base_location);

namespace tag {

struct uniform {};
struct buffer {};
struct image {};
struct texture {};

} // namespace tag

namespace operation {

struct plus {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return lhs + " + " + rhs;
	}
};

struct minus {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return lhs + " - " + rhs;
	}
};

struct min {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return "min(" + lhs + ", " + rhs + ")";
	}
};

struct max {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return "max(" + lhs + ", " + rhs + ")";
	}
};

struct multiplies {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return lhs + " * " + rhs;
	}
};

struct divides {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return lhs + " / " + rhs;
	}
};

struct modulus {
	std::string operator()(const std::string& lhs, const std::string& rhs) {
		return lhs + " % " + rhs;
	}
};

} // namespace operation

} // namespace sl

#include <cgv/config/lib_end.h>
