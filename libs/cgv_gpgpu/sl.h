#pragma once

#include <memory>
#include <string>
#include <vector>

#include "lib_begin.h"

namespace sl {

enum class Type {
	kVoid = 0,
	kBool,
	kInt,
	kUint,
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

	kArray,
	kStruct
};

extern CGV_API std::string to_string(Type type);

class CGV_API data_type;

class named_variable {
public:
	named_variable(const data_type& type, const std::string& name) : _type(std::make_shared<data_type>(type)), _name(name) {}

	const data_type* type() const {
		return _type.get();
	}

	const std::string& name() const {
		return _name;
	}

private:
	std::shared_ptr<data_type> _type;
	std::string _name;
};

extern CGV_API std::string to_string(const named_variable& variable);

using named_variable_list = std::vector<named_variable>;

extern CGV_API std::string to_string(const named_variable_list& list);

extern CGV_API std::string to_string(const named_variable_list& list, const std::string& prefix);

struct type_definition {
	std::string type_name;
	named_variable_list members;
	size_t size;
};

class CGV_API data_type {
public:
	data_type(Type type);

	data_type(Type type, size_t size);

	data_type(const std::string& name, const named_variable_list& members);

	bool is_valid() const;

	bool is_basic_type() const;

	Type type() const;

	named_variable_list members() const;

	std::string type_name() const;

	size_t array_length() const;

private:
	Type _basic_type = Type::kVoid;
	std::shared_ptr<type_definition> _definition;
};

extern CGV_API std::string get_typedef_str(const std::string& name, sl::data_type type);

} // namespace sl

#include <cgv/config/lib_end.h>
