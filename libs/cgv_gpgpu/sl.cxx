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
		"array",
		"struct"
	};
	return strs[static_cast<int>(type)];
}

std::string to_string(const named_variable& variable) {
	return variable.type()->type_name() + " " + variable.name();
}

std::string to_string(const named_variable_list& list) {
	return cgv::utils::join(list.begin(), list.end(), "; ", true);
}

std::string to_string(const named_variable_list& list, const std::string& prefix) {
	return cgv::utils::transform_join(list.begin(), list.end(), [&prefix](const named_variable& var) {
		return prefix + " " + to_string(var);
	}, "; ", true);
}

data_type::data_type(Type type) : _basic_type(type) {}

data_type::data_type(Type type, size_t size) : _basic_type(Type::kArray) {
	_definition = std::make_shared<type_definition>(type_definition{ "", {}, size });
}

data_type::data_type(const std::string& name, const named_variable_list& members) : _basic_type(Type::kStruct) {
	_definition = std::make_shared<type_definition>(type_definition{ name, members, members.size() });
}

bool data_type::is_valid() const {
	switch(_basic_type) {
	case sl::Type::kArray:
		// array types are only valid if they have a non-zero length
		return array_length() > 0;
	case sl::Type::kStruct:
		// struct types are only valid if they have a non-empty name
		return !type_name().empty();
	default:
		// basic types are always valid
		return true;
	}
}

bool data_type::is_basic_type() const {
	return _basic_type != Type::kArray && _basic_type != Type::kStruct;
}

Type data_type::type() const {
	return _basic_type;
}

named_variable_list data_type::members() const {
	if(_basic_type == Type::kStruct)
		return { _definition->members };
	return {};
}

std::string data_type::type_name() const {
	if(_basic_type == Type::kStruct)
		return _definition->type_name;
	else
		return to_string(_basic_type);
}

size_t data_type::array_length() const {
	if(_basic_type == Type::kArray)
		return _definition->size;
	return 0;
}

std::string get_typedef_str(const std::string& name, sl::data_type type) {
	std::string type_name = type.type_name();

	switch(type.type()) {
	case sl::Type::kArray:
		return "#define " + name + " " + type_name + "[" + std::to_string(type.array_length()) + "]";
	case sl::Type::kStruct:
	{
		std::string def = "struct " + type_name + " { " + to_string(type.members()) + "};";
		if(type_name != name)
			def += "\n#define " + name + " " + type_name;
		return def;
	}
	default:
		return "#define " + name + " " + type_name;
	}
};

} // namespace sl
