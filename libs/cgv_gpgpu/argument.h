#pragma once

#include <cgv/render/context.h>

#include "sl.h"

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

struct compute_kernel_argument_declaration {
	sl::named_variable_list uniforms;
	sl::named_buffer_list buffers;
};

class compute_kernel_arguments {
public:
	virtual bool emtpy() const = 0;
	virtual size_t size() const = 0;
	virtual const uniform_binding& operator[](size_t index) const = 0;
};

class compute_kernel_argument_binding_list : public compute_kernel_arguments {
public:
	compute_kernel_argument_binding_list(std::initializer_list<uniform_binding> bindings) : _bindings(bindings) {}
	
	bool emtpy() const override {
		return _bindings.empty();
	}

	size_t size() const override {
		return _bindings.size();
	}

	const uniform_binding& operator[](size_t index) const override {
		return _bindings[index];
	}

private:
	uniform_binding_list _bindings;
};

class compute_kernel_argument_struct : public compute_kernel_arguments {
public:
	bool emtpy() const override {
		return _member_bindings.empty();
	}

	size_t size() const override {
		return _member_bindings.size();
	}

	const uniform_binding& operator[](size_t index) const override {
		return *_member_bindings[index];
	}

protected:
	void connect_members(std::initializer_list<uniform_binding*> members) {
		_member_bindings.clear();
		_member_bindings.reserve(members.size());
		for(uniform_binding* member : members)
			_member_bindings.push_back(member);
	}

private:
	std::vector<uniform_binding*> _member_bindings;
};

} // namespace gpgpu
} // namespace cgv
