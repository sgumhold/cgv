#pragma once

#include <map>
#include <vector>

#include "sl.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API function_definition_registry {
private:
	using index_vector_type = std::vector<size_t>;
	using index_map_type = std::map<std::string, index_vector_type>;

public:
	bool add_global_function(sl::function_definition function);

	bool add_type_scoped_function(const sl::data_type& type, sl::function_definition function);

	const sl::function_definition_list& get_global_functions() const;

	sl::function_definition_list get_type_scoped_functions(const sl::data_type& type) const;

private:
	bool contains_function(const sl::function_definition& function) const;

	bool contains_function(const sl::function_definition_list& functions, const index_map_type& indices_by_name, const sl::function_definition& function) const;

	sl::function_definition_list _global_functions;
	sl::function_definition_list _type_scoped_functions;
	index_map_type _global_indices_by_function_name;
	index_map_type _type_scoped_indices_by_function_name;
	index_map_type _indices_by_type_name;
};

extern CGV_API function_definition_registry& get_function_registry();

extern CGV_API bool register_global_function(const sl::function_definition& function);

extern CGV_API bool register_type_scoped_function(const sl::data_type& type, const sl::function_definition& function);

class CGV_API type_scoped_function_registrator {
public:
	type_scoped_function_registrator(sl::data_type type);

	bool add(const sl::function_definition& function) const;

private:
	sl::data_type _type;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
