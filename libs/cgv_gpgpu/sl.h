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

	kStruct
};

extern CGV_API std::string to_string(Type type);

/// Indicator for variable size arrays that are typically used in buffers.
static constexpr auto varsize{ static_cast<size_t>(-1) };

class CGV_API data_type;

class named_variable {
public:
	named_variable(const data_type& type, const std::string& name) : _type(std::make_shared<data_type>(type)), _name(name) {}

	named_variable(const data_type& type, const std::string& name, size_t array_size) : named_variable(type, name) {
		_array_size = array_size;
	}

	const data_type* type() const {
		return _type.get();
	}

	const std::string& name() const {
		return _name;
	}

	size_t array_size() const {
		return _array_size;
	}

private:
	std::shared_ptr<data_type> _type;
	std::string _name;
	size_t _array_size = 0;
};

extern CGV_API std::string to_string(const named_variable& variable);

using named_variable_list = std::vector<named_variable>;

extern CGV_API std::string to_string(const named_variable_list& variables);

extern CGV_API std::string to_string(const named_variable_list& variables, const std::string& prefix);

struct type_definition {
	std::string type_name;
	named_variable_list members;
};

class CGV_API data_type {
public:
	data_type(Type type);

	data_type(const std::string& name, const named_variable_list& members);

	bool is_valid() const;

	bool is_basic_type() const;

	Type type() const;

	named_variable_list members() const;

	std::string type_name() const;

private:
	Type _basic_type = Type::kVoid;
	std::shared_ptr<type_definition> _definition;
};

extern CGV_API std::string get_typedef_str(const std::string& name, sl::data_type type);

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

	memory_qualifier_storage(std::initializer_list<MemoryQualifier> qualifiers);

	memory_qualifier_list list() const;

private:
	int32_t _mask = 0;
};

class named_buffer {
public:
	named_buffer(const named_variable& variable, const std::string& name, std::initializer_list<sl::MemoryQualifier> memory_qualifiers = {}) : _variables({ variable }), _name(name), _memory_qualifiers(memory_qualifiers) {}

	named_buffer(const named_variable_list& variables, const std::string& name, std::initializer_list<sl::MemoryQualifier> memory_qualifiers = {}) : _variables(variables), _name(name), _memory_qualifiers(memory_qualifiers) {}

	const sl::named_variable_list& variables() const {
		return _variables;
	}

	const std::string& name() const {
		return _name;
	}

	sl::memory_qualifier_list memory_qualifiers() const {
		return _memory_qualifiers.list();
	}

private:
	named_variable_list _variables;
	std::string _name;
	sl::memory_qualifier_storage _memory_qualifiers;
};

extern CGV_API std::string to_string(const named_buffer& buffer, size_t location);

using named_buffer_list = std::vector<named_buffer>;

extern CGV_API std::string to_string(const named_buffer_list& buffers, size_t base_location);

} // namespace sl

#include <cgv/config/lib_end.h>
