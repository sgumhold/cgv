#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

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






class buffer_binding {
public:
	buffer_binding() {}

	buffer_binding(const std::string& name) : _name(name) {}

	buffer_binding(const std::string& name, const cgv::render::vertex_buffer& buffer) : _name(name) {
		_buffer = &buffer;
	}

	void operator=(const cgv::render::vertex_buffer& buffer) {
		_buffer = &buffer;
	}

	// TODO: Remove?
	const std::string& name() const {
		return _name;
	}

	// TODO: Remove?
	const cgv::render::vertex_buffer* buffer() const {
		return _buffer;
	};

private:
	friend class uniform_arguments;
	friend class compute_kernel;

	// TODO: remove
	friend class transform;

	void bind(cgv::render::context& ctx, uint32_t index) const {
		_binding_index = index;
		_buffer->bind(ctx, cgv::render::VBT_STORAGE, index);
	}

	void unbind(cgv::render::context& ctx) const {
		_buffer->unbind(ctx, cgv::render::VBT_STORAGE, _binding_index);
	}

	std::string _name;
	mutable uint32_t _binding_index = 0;
	const cgv::render::vertex_buffer* _buffer = nullptr;
};

using buffer_binding_list = std::vector<buffer_binding>;











class resource_binding {
public:
	resource_binding() {}

	resource_binding(const std::string& name) : _name(name) {}

	template<typename T>
	resource_binding(const std::string& name, const T& value) : _name(name) {
		set(value);
	}

	resource_binding(const std::string& name, const cgv::render::vertex_buffer& buffer) : _name(name) {
		_desc.coordinate_type = cgv::type::info::TypeId::TI_LAST; // TODO: use a new type for buffers
		_addr = &buffer;
	}

	template<typename T>
	void operator=(const T& value) {
		set(value);
	}

	void operator=(const cgv::render::vertex_buffer& buffer) {
		_desc.coordinate_type = cgv::type::info::TypeId::TI_LAST; // TODO: use a new type for buffers
		_addr = &buffer;
	}

private:
	friend class uniform_arguments;
	friend class compute_kernel;
	// TODO: remove?
	friend class compute_kernel_argument_binding_list3;

	template<typename T>
	void set(const T& value) {
		_desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor({});
		_addr = cgv::render::element_descriptor_traits<T>::get_address(value);
	}

	std::string _name;
	cgv::render::type_descriptor _desc;
	const void* _addr = nullptr;
};














struct compute_kernel_arguments_declaration {
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













class compute_kernel_arguments2 {
public:
	virtual size_t get_uniform_count() const = 0;
	virtual const uniform_binding& get_uniform(size_t index) const = 0;

	virtual size_t get_buffer_count() const = 0;
	virtual const buffer_binding& get_buffer(size_t index) const = 0;
};

class compute_kernel_argument_binding_list2 : public compute_kernel_arguments2 {
public:
	compute_kernel_argument_binding_list2() {}

	size_t get_uniform_count() const override {
		return _uniform_bindings.size();
	}

	const uniform_binding& get_uniform(size_t index) const override {
		return _uniform_bindings[index];
	}

	void set_uniforms(std::initializer_list<uniform_binding> bindings) {
		_uniform_bindings = bindings;
	}

	size_t get_buffer_count() const override {
		return _buffer_bindings.size();
	}

	const buffer_binding& get_buffer(size_t index) const override {
		return _buffer_bindings[index];
	}

	void set_buffers(std::initializer_list<buffer_binding> bindings) {
		_buffer_bindings = bindings;
	}

private:
	uniform_binding_list _uniform_bindings;
	buffer_binding_list _buffer_bindings;
};

class compute_kernel_argument_struct2 : public compute_kernel_arguments2 {
public:
	size_t get_uniform_count() const override {
		return _uniform_bindings.size();
	}

	const uniform_binding& get_uniform(size_t index) const override {
		return *_uniform_bindings[index];
	}

	size_t get_buffer_count() const override {
		return _buffer_bindings.size();
	}

	const buffer_binding& get_buffer(size_t index) const override {
		return *_buffer_bindings[index];
	}

protected:
	void connect(std::initializer_list<uniform_binding*> uniforms) {
		_uniform_bindings = uniforms;
	}

	void connect(std::initializer_list<buffer_binding*> buffers) {
		_buffer_bindings = buffers;
	}

private:
	std::vector<uniform_binding*> _uniform_bindings;
	std::vector<buffer_binding*> _buffer_bindings;
};

} // namespace gpgpu
} // namespace cgv
