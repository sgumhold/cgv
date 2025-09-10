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

enum class Type : int32_t {
	kVoid = 0,

	kBool,
	kInt,
	kUInt,
	kFloat,
	kDouble,

	kBVec2,
	kBVec3,
	kBVec4,

	kIVec2,
	kIVec3,
	kIVec4,

	kUVec2,
	kUVec3,
	kUVec4,

	kVec2,
	kVec3,
	kVec4,

	kDVec2,
	kDVec3,
	kDVec4,

	kMat2,
	kMat3,
	kMat4,

	kMat2x3,
	kMat2x4,
	kMat3x2,
	kMat3x4,
	kMat4x2,
	kMat4x3,

	kDMat2,
	kDMat3,
	kDMat4,

	kDMat2x3,
	kDMat2x4,
	kDMat3x2,
	kDMat3x4,
	kDMat4x2,
	kDMat4x3,

	kStruct
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
	Type _base_type = Type::kVoid;
	std::shared_ptr<type_definition> _definition;
};

extern CGV_API size_t get_aligned_size(data_type type);

extern CGV_API std::string get_type_definition_string(data_type type);

extern CGV_API std::string get_alias_string(const std::string& alias, const std::string& type);

extern CGV_API std::string get_type_alias_string(const std::string& alias, data_type type);

struct type_definition {
	std::string type_name;
	named_variable_list members;
};

class named_variable {
public:
	named_variable(const data_type& type, const std::string& name) : _type(type), _name(name) {}

	named_variable(const data_type& type, const std::string& name, size_t array_size) : named_variable(type, name) {
		_array_size = array_size;
	}

	const data_type& type() const {
		return _type;
	}

	const std::string& name() const {
		return _name;
	}

	size_t array_size() const {
		return _array_size;
	}

private:
	data_type _type;
	std::string _name;
	size_t _array_size = 0;
};

extern CGV_API std::string to_string(const named_variable& variable);

extern CGV_API std::string to_string(const named_variable_list& variables);

extern CGV_API std::string to_string(const named_variable_list& variables, const std::string& prefix);

enum class MemoryQualifier : int32_t {
	kNone = 0,
	kCoherent = 1,
	kVolatile = 2,
	kRestrict = 4,
	kReadOnly = 8,
	kWriteOnly = 16
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

class named_buffer {
public:
	named_buffer(const named_variable& variable, const std::string& name, const memory_qualifier_list& memory_qualifiers = {}) : _variables({ variable }), _name(name), _memory_qualifiers(memory_qualifiers) {}
	
	named_buffer(const named_variable_list& variables, const std::string& name, const memory_qualifier_list& memory_qualifiers = {}) : _variables(variables), _name(name), _memory_qualifiers(memory_qualifiers) {}

	const named_variable_list& variables() const {
		return _variables;
	}

	const std::string& name() const {
		return _name;
	}

	memory_qualifier_list memory_qualifiers() const {
		return _memory_qualifiers.list();
	}

private:
	named_variable_list _variables;
	std::string _name;
	memory_qualifier_storage _memory_qualifiers;
};

extern CGV_API std::string to_string(const named_buffer& buffer, size_t location);

using named_buffer_list = std::vector<named_buffer>;

extern CGV_API std::string to_string(const named_buffer_list& buffers, size_t base_location);

enum class ImageFormatLayoutQualifier : int32_t {
	// floating-point layout image formats
	k_rgba32f = 0,
	k_rgba16f,
	k_rg32f,
	k_rg16f,
	k_r11f_g11f_b10f,
	k_r32f,
	k_r16f,
	k_rgba16,
	k_rgb10_a2,
	k_rgba8,
	k_rg16,
	k_rg8,
	k_r16,
	k_r8,
	k_rgba16_snorm,
	k_rgba8_snorm,
	k_rg16_snorm,
	k_rg8_snorm,
	k_r16_snorm,
	k_r8_snorm,

	// signed integer layout image formats
	k_rgba32i,
	k_rgba16i,
	k_rgba8i,
	k_rg32i,
	k_rg16i,
	k_rg8i,
	k_r32i,
	k_r16i,
	k_r8i,

	// unsigned integer layout image formats
	k_rgba32ui,
	k_rgba16ui,
	k_rgb10_a2ui,
	k_rgba8ui,
	k_rg32ui,
	k_rg16ui,
	k_rg8ui,
	k_r32ui,
	k_r16ui,
	k_r8ui,
};

extern CGV_API std::string to_string(ImageFormatLayoutQualifier qualifier);

extern CGV_API std::string get_type_prefix(ImageFormatLayoutQualifier qualifier);






class named_image {
public:
	named_image(cgv::render::TextureType texture_type, ImageFormatLayoutQualifier image_format, const std::string& name, const memory_qualifier_list& memory_qualifiers = {}) : _texture_type(texture_type), _image_format(image_format), _name(name), _memory_qualifiers(memory_qualifiers) {}

	cgv::render::TextureType texture_type() const {
		return _texture_type;
	}

	ImageFormatLayoutQualifier image_format() const {
		return _image_format;
	}

	const std::string& name() const {
		return _name;
	}

	memory_qualifier_list memory_qualifiers() const {
		return _memory_qualifiers.list();
	}

private:
	cgv::render::TextureType _texture_type;
	ImageFormatLayoutQualifier _image_format;
	std::string _name;
	memory_qualifier_storage _memory_qualifiers;
};

extern CGV_API std::string to_string(const named_image& image, size_t location);

using named_image_list = std::vector<named_image>;

extern CGV_API std::string to_string(const named_image_list& images, size_t base_location);








// TODO: Support unsigned and signed integer samplers (update to_string methods!).
class named_texture {
public:
	named_texture(cgv::render::TextureType texture_type, const std::string& name) : _texture_type(texture_type), _name(name) {}

	cgv::render::TextureType texture_type() const {
		return _texture_type;
	}

	const std::string& name() const {
		return _name;
	}

private:
	cgv::render::TextureType _texture_type;
	std::string _name;
};

extern CGV_API std::string get_sampler_string(const cgv::render::TextureType& texture_type);

extern CGV_API std::string to_string(const named_texture& texture, size_t location);

using named_texture_list = std::vector<named_texture>;

extern CGV_API std::string to_string(const named_texture_list& textures, size_t base_location);








namespace tag {

struct uniform {};
struct buffer {};
struct image {};
struct texture {};

} // namespace tag

namespace traits {

template<class T>
/*inline*/ constexpr bool is_instance_of_fvec_v = std::false_type{};

template<class T, cgv::type::uint32_type N>
/*inline*/ constexpr bool is_instance_of_fvec_v<cgv::math::fvec<T, N>> = std::true_type{};

template<class T>
/*inline*/ constexpr bool is_instance_of_fmat_v = std::false_type{};

template<class T, cgv::type::uint32_type N, cgv::type::uint32_type M>
/*inline*/ constexpr bool is_instance_of_fmat_v<cgv::math::fmat<T, N, M>> = std::true_type{};

// TODO: Move is_instance_of(_v) to cgv/type/traits (can use inline if _HAS_CXX17 is defined)
template<class T, template<class...> class U>
/*inline*/ constexpr bool is_instance_of_v = std::false_type{};

template<template<class...> class U, class ...Vs>
/*inline*/ constexpr bool is_instance_of_v<U<Vs...>, U> = std::true_type{};

template<class T>
/*inline*/ constexpr bool is_fundamental_sl_type_v =
	std::is_arithmetic_v<std::remove_cv_t<T>> ||
	is_instance_of_fvec_v<T> ||
	is_instance_of_fmat_v<T>;

template<class T>
struct is_fundamental_sl_type : std::bool_constant<is_fundamental_sl_type_v<T>> {};

} // namespace traits

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
