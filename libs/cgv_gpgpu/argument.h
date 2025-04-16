#pragma once

#include <cgv/render/context.h>

namespace cgv {
namespace gpgpu {

class uniform_binding {
public:
	uniform_binding() {}

	uniform_binding(const std::string& name) : _name(name) {}

	template<typename T>
	uniform_binding(const std::string& name, const T& value) : _name(name) {
		set(value);
	}

	template<typename T>
	void operator=(const T& value) {
		set(value);
	}

private:
	friend class uniform_arguments;
	friend class compute_kernel;

	template<typename T>
	void set(const T& value) {
		_desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor({});
		_addr = cgv::render::element_descriptor_traits<T>::get_address(value);
	}

	std::string _name;
	cgv::render::type_descriptor _desc;
	const void* _addr = nullptr;
};

using uniform_binding_list = std::vector<uniform_binding>;

class uniform_arguments {
protected:
	void connect(std::initializer_list<uniform_binding*> arguments) {
		_bindings.clear();
		_bindings.reserve(arguments.size());
		for(uniform_binding* argument : arguments)
			_bindings.push_back(argument);
	}

private:
	friend class compute_kernel;

	std::vector<uniform_binding*> _bindings;
};

} // namespace gpgpu
} // namespace cgv
