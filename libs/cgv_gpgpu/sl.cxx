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
		{ Type::kVoid, 0, 0, 0, 0 },		// kVoid
		{ Type::kBool, 1, 1, 4, 4 },		// kBool
		{ Type::kInt, 1, 1, 4, 4 },			// kInt
		{ Type::kUInt, 1, 1, 4, 4 },		// kUInt
		{ Type::kFloat, 1, 1, 4, 4 },		// kFloat
		{ Type::kDouble, 1, 1, 8, 8 },		// kDouble
		{ Type::kBool, 1, 2, 8, 8 },		// kBVec2
		{ Type::kBool, 1, 3, 12, 16 },		// kBVec3
		{ Type::kBool, 1, 4, 16, 16 },		// kBVec4
		{ Type::kInt, 1, 2, 8, 8 },			// kIVec2
		{ Type::kInt, 1, 3, 12, 16 },		// kIVec3
		{ Type::kInt, 1, 4, 16, 16 },		// kIVec4
		{ Type::kUInt, 1, 2, 8, 8 },		// kUVec2
		{ Type::kUInt, 1, 3, 12, 16 },		// kUVec3
		{ Type::kUInt, 1, 4, 16, 16 },		// kUVec4
		{ Type::kFloat, 1, 2, 8, 8 },		// kVec2
		{ Type::kFloat, 1, 3, 12, 16 },		// kVec3
		{ Type::kFloat, 1, 4, 16, 16 },		// kVec4
		{ Type::kDouble, 1, 2, 16, 16 },	// kDVec2
		{ Type::kDouble, 1, 3, 24, 32 },	// kDVec3
		{ Type::kDouble, 1, 4, 32, 32 },	// kDVec4
		{ Type::kFloat, 2, 2, 16, 8 },		// kMat2
		{ Type::kFloat, 3, 3, 36, 16 },		// kMat3
		{ Type::kFloat, 4, 4, 64, 16 },		// kMat4
		{ Type::kFloat, 2, 3, 24, 16 },		// kMat2x3
		{ Type::kFloat, 2, 4, 32, 16 },		// kMat2x4
		{ Type::kFloat, 3, 2, 24, 8 },		// kMat3x2
		{ Type::kFloat, 3, 4, 48, 16 },		// kMat3x4
		{ Type::kFloat, 4, 2, 32, 8 },		// kMat4x2
		{ Type::kFloat, 4, 3, 48, 16 },		// kMat4x3
		{ Type::kDouble, 2, 2, 32, 16 },	// kDMat2
		{ Type::kDouble, 3, 3, 72, 32 },	// kDMat3
		{ Type::kDouble, 4, 4, 128, 32 },	// kDMat4
		{ Type::kDouble, 2, 3, 48, 32 },	// kDMat2x3
		{ Type::kDouble, 2, 4, 64, 32 },	// kDMat2x4
		{ Type::kDouble, 3, 2, 48, 16 },	// kDMat3x2
		{ Type::kDouble, 3, 4, 96, 32 },	// kDMat3x4
		{ Type::kDouble, 4, 2, 64, 16 },	// kDMat4x2
		{ Type::kDouble, 4, 3, 96, 32 },	// kDMat4x3
		{ Type::kStruct, 0, 0, 0, 0 },		// kStruct
	}};
	return infos[static_cast<int32_t>(type)];
}

data_type::data_type() {}

data_type::data_type(Type type) : _base_type(type) {}

data_type::data_type(const std::string& name, const named_variable_list& members) : _base_type(Type::kStruct) {
	_definition = std::make_shared<type_definition>(type_definition{ name, members });
}

Type data_type::type() const {
	return _base_type;
}

named_variable_list data_type::members() const {
	if(_base_type == Type::kStruct)
		return { _definition->members };
	return {};
}

std::string data_type::type_name() const {
	if(_base_type == Type::kStruct)
		return _definition->type_name;
	else
		return to_string(_base_type);
}

bool data_type::is_valid() const {
	switch(_base_type) {
	case Type::kStruct:
		// Struct types are only valid if they have a non-empty name.
		return !type_name().empty();
	default:
		// Basic types are always valid.
		return true;
	}
}

bool data_type::is_void() const {
	return _base_type == Type::kVoid;
}

bool data_type::is_scalar() const {
	int32_t index = static_cast<int32_t>(_base_type);
	return index >= static_cast<int32_t>(Type::kBool) && index <= static_cast<int32_t>(Type::kDouble);
}

bool data_type::is_vector() const {
	int32_t index = static_cast<int32_t>(_base_type);
	return index >= static_cast<int32_t>(Type::kBVec2) && index <= static_cast<int32_t>(Type::kDVec4);
}

bool data_type::is_matrix() const {
	int32_t index = static_cast<int32_t>(_base_type);
	return index >= static_cast<int32_t>(Type::kMat2) && index <= static_cast<int32_t>(Type::kDMat4x3);
}

bool data_type::is_compound() const {
	return _base_type == Type::kStruct;
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
	case Type::kStruct:
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
	case MemoryQualifier::kCoherent:
		return "coherent";
	case MemoryQualifier::kVolatile:
		return "volatile";
	case MemoryQualifier::kRestrict:
		return "restrict";
	case MemoryQualifier::kReadOnly:
		return "readonly";
	case MemoryQualifier::kWriteOnly:
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
		MemoryQualifier::kCoherent,
		MemoryQualifier::kVolatile,
		MemoryQualifier::kRestrict,
		MemoryQualifier::kReadOnly,
		MemoryQualifier::kWriteOnly
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
	if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::k_rgba32ui))
		return "u";
	else if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::k_rgba32i))
		return "i";
	else
		return "";
}

data_type get_data_type(ImageFormatLayoutQualifier qualifier) {
	int32_t index = static_cast<int32_t>(qualifier);
	if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::k_rgba32ui))
		return Type::kUVec4;
	else if(index >= static_cast<int32_t>(ImageFormatLayoutQualifier::k_rgba32i))
		return Type::kIVec4;
	else
		return Type::kVec4;
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

	if(sampler_base_format == SamplerBaseFormat::kSignedInteger)
		return "i" + str;
	else if(sampler_base_format == SamplerBaseFormat::kUnsignedInteger)
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
