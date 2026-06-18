#include "sl.h"

#include <array>

#include <cgv/math/integer.h>
#include <cgv/utils/algorithm.h>

namespace sl {

std::string to_string(Type type) {
	static const std::array<std::string, 40> strs = {
		"void",
		"bool",
		"int",
		"uint",
		"float",
		"double",
		"bvec2",
		"bvec3",
		"bvec4",
		"ivec2",
		"ivec3",
		"ivec4",
		"uvec2",
		"uvec3",
		"uvec4",
		"vec2",
		"vec3",
		"vec4",
		"dVec2",
		"dVec3",
		"dVec4",
		"mat2",
		"mat3",
		"mat4",
		"mat2x3",
		"mat2x4",
		"mat3x2",
		"mat3x4",
		"mat4x2",
		"mat4x3",
		"dmat2",
		"dmat3",
		"dmat4",
		"dmat2x3",
		"dmat2x4",
		"dmat3x2",
		"dmat3x4",
		"dmat4x2",
		"dmat4x3",
		"struct"
	};
	return strs[static_cast<int32_t>(type)];
}

type_info get_type_info(Type type) {
	static const std::array<type_info, 40> infos = {{
		{ Type::Void, 0, 0, 0, 0 },			// Void
		{ Type::Bool, 1, 1, 4, 4 },			// Bool
		{ Type::Int, 1, 1, 4, 4 },			// Int
		{ Type::UInt, 1, 1, 4, 4 },			// UInt
		{ Type::Float, 1, 1, 4, 4 },		// Float
		{ Type::Double, 1, 1, 8, 8 },		// Double
		{ Type::Bool, 1, 2, 8, 8 },			// BVec2
		{ Type::Bool, 1, 3, 12, 16 },		// BVec3
		{ Type::Bool, 1, 4, 16, 16 },		// BVec4
		{ Type::Int, 1, 2, 8, 8 },			// IVec2
		{ Type::Int, 1, 3, 12, 16 },		// IVec3
		{ Type::Int, 1, 4, 16, 16 },		// IVec4
		{ Type::UInt, 1, 2, 8, 8 },			// UVec2
		{ Type::UInt, 1, 3, 12, 16 },		// UVec3
		{ Type::UInt, 1, 4, 16, 16 },		// UVec4
		{ Type::Float, 1, 2, 8, 8 },		// Vec2
		{ Type::Float, 1, 3, 12, 16 },		// Vec3
		{ Type::Float, 1, 4, 16, 16 },		// Vec4
		{ Type::Double, 1, 2, 16, 16 },		// DVec2
		{ Type::Double, 1, 3, 24, 32 },		// DVec3
		{ Type::Double, 1, 4, 32, 32 },		// DVec4
		{ Type::Float, 2, 2, 16, 8 },		// Mat2
		{ Type::Float, 3, 3, 36, 16 },		// Mat3
		{ Type::Float, 4, 4, 64, 16 },		// Mat4
		{ Type::Float, 2, 3, 24, 16 },		// Mat2x3
		{ Type::Float, 2, 4, 32, 16 },		// Mat2x4
		{ Type::Float, 3, 2, 24, 8 },		// Mat3x2
		{ Type::Float, 3, 4, 48, 16 },		// Mat3x4
		{ Type::Float, 4, 2, 32, 8 },		// Mat4x2
		{ Type::Float, 4, 3, 48, 16 },		// Mat4x3
		{ Type::Double, 2, 2, 32, 16 },		// DMat2
		{ Type::Double, 3, 3, 72, 32 },		// DMat3
		{ Type::Double, 4, 4, 128, 32 },	// DMat4
		{ Type::Double, 2, 3, 48, 32 },		// DMat2x3
		{ Type::Double, 2, 4, 64, 32 },		// DMat2x4
		{ Type::Double, 3, 2, 48, 16 },		// DMat3x2
		{ Type::Double, 3, 4, 96, 32 },		// DMat3x4
		{ Type::Double, 4, 2, 64, 16 },		// DMat4x2
		{ Type::Double, 4, 3, 96, 32 },		// DMat4x3
		{ Type::Struct, 0, 0, 0, 0 },		// Struct
	}};
	return infos[static_cast<int32_t>(type)];
}

data_type::data_type() {}

data_type::data_type(Type type) : _base_type(type) {}

data_type::data_type(const std::string& name, const named_variable_list& members) : _base_type(Type::Struct) {
	_definition = std::make_shared<type_definition>(type_definition{ name, members });
}

Type data_type::type() const {
	return _base_type;
}

named_variable_list data_type::members() const {
	if(_base_type == Type::Struct)
		return { _definition->members };
	return {};
}

std::string data_type::type_name() const {
	if(_base_type == Type::Struct)
		return _definition->type_name;
	else
		return to_string(_base_type);
}

bool data_type::is_valid() const {
	switch(_base_type) {
	case Type::Struct:
		// Struct types are only valid if they have a non-empty name.
		return !type_name().empty();
	default:
		// Basic types are always valid.
		return true;
	}
}

bool data_type::is_void() const {
	return _base_type == Type::Void;
}

bool data_type::is_scalar() const {
	int32_t index = static_cast<int32_t>(_base_type);
	return index >= static_cast<int32_t>(Type::Bool) && index <= static_cast<int32_t>(Type::Double);
}

bool data_type::is_vector() const {
	int32_t index = static_cast<int32_t>(_base_type);
	return index >= static_cast<int32_t>(Type::BVec2) && index <= static_cast<int32_t>(Type::DVec4);
}

bool data_type::is_matrix() const {
	int32_t index = static_cast<int32_t>(_base_type);
	return index >= static_cast<int32_t>(Type::Mat2) && index <= static_cast<int32_t>(Type::DMat4x3);
}

bool data_type::is_compound() const {
	return _base_type == Type::Struct;
}

size_t data_type::size_in_bytes() const {
	if(is_compound()) {
		size_t size = 0;
		size_t max_alignment = 0;
		for(const named_variable& member : _definition->members) {
			size_t member_alignment = member.type().alignment_in_bytes();
			max_alignment = std::max(member_alignment, max_alignment);

			if(size > 0)
				size = cgv::math::next_multiple_k_greater_than_n(member_alignment, size);

			size_t member_size = member.type().size_in_bytes();

			// Compound and matrix types behave like arrays, in that they have padding at the end.
			if(member.type().is_compound() || member.type().is_matrix())
				member_size = cgv::math::next_multiple_k_greater_than_n(member_alignment, member_size);

			if(member.array_size() > 0)
				// The size of the whole array is the number of elements times their alignment.
				// Arrays possibly have padding at the end which cannot be used for the next member.
				member_size = member.array_size() * member_alignment;

			size += member_size;
		}

		return size;
	}

	return get_type_info(_base_type).base_size;
}

size_t data_type::alignment_in_bytes() const {
	if(is_compound()) {
		size_t max_alignment = 0;
		for(const named_variable& member : _definition->members)
			max_alignment = std::max(member.type().alignment_in_bytes(), max_alignment);
		return max_alignment;
	}

	return get_type_info(_base_type).base_alignment;
}

size_t get_aligned_size(data_type type) {
	return cgv::math::next_multiple_k_greater_than_n(type.alignment_in_bytes(), type.size_in_bytes());
}

std::string get_type_definition_string(data_type type) {
	std::string type_name = type.type_name();
	switch(type.type()) {
	case Type::Struct:
		return "struct " + type_name + " { " + to_string(type.members()) + "};";
	default:
		return type_name;
	}
}

std::string get_alias_string(const std::string& alias, const std::string& name) {
	return "#define " + alias + " " + name;
}

std::string get_type_alias_string(const std::string& alias, data_type type) {
	return get_alias_string(alias, type.type_name());
}

std::string to_string(const named_variable& variable) {
	std::string res;
	res = variable.type().type_name() + " " + variable.name();
	if(variable.array_size() != 0)
		res += "[" + (variable.array_size() == varsize ? "" : std::to_string(variable.array_size())) + "]";
	return res;
}

std::string to_string(const named_variable_list& variables) {
	return cgv::utils::join(variables.begin(), variables.end(), "; ", true);
}

std::string to_string(const named_variable_list& variables, const std::string& prefix) {
	return cgv::utils::transform_join(variables.begin(), variables.end(), [&prefix](const named_variable& var) {
		return prefix + " " + to_string(var);
	}, "; ", true);
}

std::string to_string(MemoryQualifier qualifier) {
	switch(qualifier) {
	case MemoryQualifier::Coherent:
		return "coherent";
	case MemoryQualifier::Volatile:
		return "volatile";
	case MemoryQualifier::Restrict:
		return "restrict";
	case MemoryQualifier::ReadOnly:
		return "readonly";
	case MemoryQualifier::WriteOnly:
		return "writeonly";
	default:
		return "";
	}
}

std::string to_string(const memory_qualifier_list& qualifiers) {
	return cgv::utils::join(qualifiers.begin(), qualifiers.end(), " ", true);
}

memory_qualifier_storage::memory_qualifier_storage(const memory_qualifier_list& qualifiers) {
	_mask = 0;
	for(MemoryQualifier qualifier : qualifiers)
		_mask |= static_cast<int32_t>(qualifier);
}

memory_qualifier_list memory_qualifier_storage::list() const {
	const memory_qualifier_list all_qualifiers = {
		MemoryQualifier::Coherent,
		MemoryQualifier::Volatile,
		MemoryQualifier::Restrict,
		MemoryQualifier::ReadOnly,
		MemoryQualifier::WriteOnly
	};

	memory_qualifier_list qualifiers;
	for(MemoryQualifier qualifier : all_qualifiers) {
		if(_mask & static_cast<int32_t>(qualifier))
			qualifiers.push_back(qualifier);
	}

	return qualifiers;
}

std::string to_string(const named_buffer& buffer, size_t location) {
	std::string location_str = std::to_string(location);
	std::string name = buffer.name().empty() ? "buffer" + location_str : buffer.name();

	std::string res = "layout(std430, binding=" + location_str + ") ";
	res += to_string(buffer.memory_qualifiers());
	res += "buffer " + name + " {\n";
	res += to_string(buffer.variables());
	res += "\n};";

	return res;
}

std::string to_string(const named_buffer_list& buffers, size_t base_location) {
	return cgv::utils::transform_join(buffers.begin(), buffers.end(), [&base_location](const named_buffer& buffer) {
		return to_string(buffer, base_location++);
	}, "\n", true);
}

std::string to_string(ImageFormatLayoutQualifier qualifier) {
	static const std::array<std::string, 39> strs = {
		"rgba32f",
		"rgba16f",
		"rg32f",
		"rg16f",
		"r11f_g11f_b10f",
		"r32f",
		"r16f",
		"rgba16",
		"rgb10_a2",
		"rgba8",
		"rg16",
		"rg8",
		"r16",
		"r8",
		"rgba16_snorm",
		"rgba8_snorm",
		"rg16_snorm",
		"rg8_snorm",
		"r16_snorm",
		"r8_snorm",
		"rgba32i",
		"rgba16i",
		"rgba8i",
		"rg32i",
		"rg16i",
		"rg8i",
		"r32i",
		"r16i",
		"r8i",
		"rgba32ui",
		"rgba16ui",
		"rgb10_a2ui",
		"rgba8ui",
		"rg32ui",
		"rg16ui",
		"rg8ui",
		"r32ui",
		"r16ui",
		"r8ui"
	};
	return strs[static_cast<int32_t>(qualifier)];
}

std::string get_type_prefix(ImageFormatLayoutQualifier qualifier) {
	int32_t index = static_cast<int32_t>(qualifier);
	if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::rgba32ui))
		return "u";
	else if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::rgba32i))
		return "i";
	else
		return "";
}

data_type get_data_type(ImageFormatLayoutQualifier qualifier) {
	int32_t index = static_cast<int32_t>(qualifier);
	if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::rgba32ui))
		return Type::UVec4;
	else if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::rgba32i))
		return Type::IVec4;
	else
		return Type::Vec4;
}

std::string to_string(const named_image& image, size_t location) {
	std::string location_str = std::to_string(location);
	std::string name = image.name().empty() ? "image" + location_str : image.name();

	int32_t dims = 0;
	switch(image.texture_type()) {
	case cgv::render::TextureType::TT_1D:
		dims = 1;
		break;
	case cgv::render::TextureType::TT_2D:
		dims = 2;
		break;
	case cgv::render::TextureType::TT_3D:
		dims = 3;
		break;
	default:
		break;
	}

	std::string image_type = get_type_prefix(image.image_format()) + "image" + std::to_string(dims) + "D";

	std::string res = "layout(" + to_string(image.image_format()) + ", binding=" + location_str + ") uniform ";
	res += to_string(image.memory_qualifiers());
	res += image_type + " " + name + ";";
	
	return res;
}

std::string to_string(const named_image_list& images, size_t base_location) {
	return cgv::utils::transform_join(images.begin(), images.end(), [&base_location](const named_image& image) {
		return to_string(image, base_location++);
	}, "\n", true);
}

std::string get_sampler_string(const cgv::render::TextureType& texture_type, SamplerBaseFormat sampler_base_format) {
	std::string str = "sampler2D";

	switch(texture_type) {
	case cgv::render::TextureType::TT_1D: str = "sampler1D"; break;
	case cgv::render::TextureType::TT_2D: str = "sampler2D"; break;
	case cgv::render::TextureType::TT_3D: str = "sampler3D"; break;
	case cgv::render::TextureType::TT_1D_ARRAY: str = "sampler1DArray"; break;
	case cgv::render::TextureType::TT_2D_ARRAY: str = "sampler2DArray"; break;
	case cgv::render::TextureType::TT_CUBEMAP: str = "samplerCube"; break;
	case cgv::render::TextureType::TT_MULTISAMPLE_2D: str = "sampler2DMS"; break;
	case cgv::render::TextureType::TT_MULTISAMPLE_2D_ARRAY: str = "sampler2DMSArray"; break;
	case cgv::render::TextureType::TT_BUFFER: str = "samplerBuffer"; break;
	default: break;
	}

	if(sampler_base_format == SamplerBaseFormat::SignedInteger)
		return "i" + str;
	else if(sampler_base_format == SamplerBaseFormat::UnsignedInteger)
		return "u" + str;
	else
		return str;
}

std::string to_string(const named_texture& texture, size_t location) {
	std::string location_str = std::to_string(location);
	std::string name = texture.name().empty() ? "texture" + location_str : texture.name();
	std::string sampler_type = get_sampler_string(texture.texture_type(), texture.sampler_base_format());
	return "layout(binding=" + location_str + ") uniform " + sampler_type + " " + name + ";";
}

std::string to_string(const named_texture_list& textures, size_t base_location) {
	return cgv::utils::transform_join(textures.begin(), textures.end(), [&base_location](const named_texture& texture) {
		return to_string(texture, base_location++);
	}, "\n", true);
}

} // namespace sl
