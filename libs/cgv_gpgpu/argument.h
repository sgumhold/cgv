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



struct resource_declaration_storage {
	std::string name;

	virtual bool is_buffer() const = 0;
};

struct uniform_declaration_storage : public resource_declaration_storage {
	sl::data_type type;
	size_t array_size = 0;

	virtual bool is_buffer() const { return false; };
};

struct buffer_declaration_storage : public resource_declaration_storage {
	sl::named_variable_list variables;
	sl::memory_qualifier_storage memory_qualifiers;

	virtual bool is_buffer() const { return true; };
};

class generic_arg_decl {
public:
	generic_arg_decl(const sl::data_type& type, const std::string& name) : _variable(type, name) {}

	generic_arg_decl(const sl::data_type& type, const std::string& name, size_t array_size) : _variable(type, name, array_size) {}

	generic_arg_decl(sl::tag::buffer, const sl::named_variable& variable, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", { variable } }, name), _memory_qualifiers(memory_qualifiers), _is_buffer(true) {}

	generic_arg_decl(sl::tag::buffer, const sl::named_variable_list& variables, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", variables }, name), _memory_qualifiers(memory_qualifiers), _is_buffer(true) {}

private:
	friend class ckadl;

	sl::named_variable _variable;
	sl::memory_qualifier_storage _memory_qualifiers;
	bool _is_buffer = false;
};

// TODO: provide iterator (or vector) initializers in all constructors with initializer lists
// TODO: do not use initializer list in sl::named buffer init but rather use const memory_qualifier_list&

// TODO: use this or list of generic_argument_declaration...?
struct ckadl {
	ckadl(std::initializer_list<generic_arg_decl> args) {
		for(auto& arg : args) {
			if(arg._is_buffer)
				//buffers.push_back(sl::named_buffer(arg._variables, arg._name, arg._memory_qualifiers.list()));
				buffers.push_back(sl::named_buffer(arg._variable.type()->members(), arg._variable.name(), arg._memory_qualifiers.list()));
			else
				//uniforms.push_back(sl::named_variable(arg._type, arg._name, arg._array_size));
				uniforms.push_back(arg._variable);
		}
	}

	sl::named_variable_list uniforms;
	sl::named_buffer_list buffers;
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
