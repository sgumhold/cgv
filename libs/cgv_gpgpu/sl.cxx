#include "sl.h"

#include <cgv/utils/algorithm.h>

namespace sl {

std::string to_string(Type type) {
	static std::vector<std::string> strs = {
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
	return strs[static_cast<int>(type)];
}

data_type::data_type() {}

data_type::data_type(Type type) : _base_type(type) {}

data_type::data_type(const std::string& name, const named_variable_list& members) : _base_type(Type::kStruct) {
	_definition = std::make_shared<type_definition>(type_definition{ name, members });
}

bool data_type::is_valid() const {
	switch(_base_type) {
	case sl::Type::kStruct:
		// struct types are only valid if they have a non-empty name
		return !type_name().empty();
	default:
		// basic types are always valid
		return true;
	}
}

bool data_type::is_void() const {
	return _base_type == Type::kVoid;
}

bool data_type::is_compound() const {
	return _base_type == Type::kStruct;
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

std::string get_type_definition_string(sl::data_type type) {
	std::string type_name = type.type_name();
	switch(type.type()) {
	case sl::Type::kStruct:
		return "struct " + type_name + " { " + to_string(type.members()) + "};";
	default:
		return type_name;
	}
}

std::string get_type_alias_string(const std::string& alias, sl::data_type type) {
	return "#define " + alias + " " + type.type_name();
}

std::string to_string(const named_variable& variable) {
	std::string res;
	res = variable.type().type_name() + " " + variable.name();
	if(variable.array_size() != 0)
		res += "[" + (variable.array_size() == sl::varsize ? "" : std::to_string(variable.array_size())) + "]";
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
	case sl::MemoryQualifier::kCoherent:
		return "coherent";
	case sl::MemoryQualifier::kVolatile:
		return "volatile";
	case sl::MemoryQualifier::kRestrict:
		return "restrict";
	case sl::MemoryQualifier::kReadOnly:
		return "readonly";
	case sl::MemoryQualifier::kWriteOnly:
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
	const sl::memory_qualifier_list all_qualifiers = {
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

std::string to_string(const named_buffer_list& buffers, size_t base_index) {
	return cgv::utils::transform_join(buffers.begin(), buffers.end(), [&base_index](const named_buffer& buffer) {
		return to_string(buffer, base_index++);
	}, "\n", true);
}

} // namespace sl
