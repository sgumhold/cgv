#include "function_definition_registry.h"

namespace cgv {
namespace gpgpu {

bool function_definition_registry::add_global_function(sl::function_definition function) {
	if(contains_function(function))
		return false;

	_global_indices_by_function_name[function.name].push_back(_global_functions.size());
	_global_functions.push_back(std::move(function));
	return true;
}

bool function_definition_registry::add_type_scoped_function(const sl::data_type& type, sl::function_definition function) {
	if(contains_function(function))
		return false;

	_type_scoped_indices_by_function_name[function.name].push_back(_type_scoped_functions.size());
	_indices_by_type_name[type.type_name()].push_back(_type_scoped_functions.size());
	_type_scoped_functions.push_back(std::move(function));
	return true;
}

const sl::function_definition_list& function_definition_registry::get_global_functions() const {
	return _global_functions;
}

sl::function_definition_list function_definition_registry::get_type_scoped_functions(const sl::data_type& type) const {
	auto it = _indices_by_type_name.find(type.type_name());
	if(it != _indices_by_type_name.end()) {
		sl::function_definition_list functions;
		functions.reserve(it->second.size());
		std::for_each(it->second.begin(), it->second.end(), [this, &functions](size_t index) {
			functions.push_back(_type_scoped_functions[index]);
		});
		return functions;
	}

	return {};
}

bool function_definition_registry::contains_function(const sl::function_definition& function) const {
	return contains_function(_global_functions, _global_indices_by_function_name, function) || contains_function(_type_scoped_functions, _type_scoped_indices_by_function_name, function);
}

bool function_definition_registry::contains_function(const sl::function_definition_list& functions, const index_map_type& indices_by_name, const sl::function_definition& function) const {
	auto it = indices_by_name.find(function.name);
	if(it != indices_by_name.end()) {
		for(size_t i : it->second) {
			if(functions[i] == function)
				return true;
		}
	}
	return false;
}

function_definition_registry& get_function_registry() {
	static function_definition_registry registry;
	return registry;
}

bool register_global_function(const sl::function_definition& function) {
	return get_function_registry().add_global_function(function);
}

bool register_type_scoped_function(const sl::data_type& type, const sl::function_definition& function) {
	return get_function_registry().add_type_scoped_function(type, function);
}

type_scoped_function_registrator::type_scoped_function_registrator(sl::data_type type) : _type(type) {}

bool type_scoped_function_registrator::add(const sl::function_definition& function) const {
	return register_type_scoped_function(_type, function);
}

} // namespace gpgpu
} // namespace cgv
