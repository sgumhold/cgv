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

	uniform_binding(const std::string& name, cgv::render::type_descriptor descriptor, const void* address) : _name(name), _desc(descriptor), _addr(address) {}

	template<typename T>
	void operator=(const T& value) {
		set(value);
	}

	const std::string& name() const {
		return _name;
	}

	cgv::render::type_descriptor descriptor() const {
		return _desc;
	};

	const void* address() const {
		return _addr;
	};

private:
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

template<typename T>
class typed_uniform_binding : public uniform_binding {
public:
	typed_uniform_binding() {}

	typed_uniform_binding(const std::string& name) : uniform_binding(name) {}

	typed_uniform_binding(const std::string& name, const T& value) : uniform_binding(name, value) {}

	void operator=(const T& value) {
		uniform_binding::operator=(value);
	}
};

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

	const std::string& name() const {
		return _name;
	}

	const cgv::render::vertex_buffer* buffer() const {
		return _buffer;
	};

	void bind(cgv::render::context& ctx, uint32_t index) const {
		_binding_index = index;
		_buffer->bind(ctx, cgv::render::VBT_STORAGE, index);
	}

	void unbind(cgv::render::context& ctx) const {
		_buffer->unbind(ctx, cgv::render::VBT_STORAGE, _binding_index);
	}

private:
	std::string _name;
	mutable uint32_t _binding_index = 0;
	const cgv::render::vertex_buffer* _buffer = nullptr;
};

using buffer_binding_list = std::vector<buffer_binding>;

// TODO: provide iterator (or vector) initializers in all constructors with initializer lists.

class argument_definition {
	friend struct argument_definitions;

public:
	argument_definition(const sl::data_type& type, const std::string& name) : _variable(type, name) {}

	argument_definition(const sl::data_type& type, const std::string& name, size_t array_size) : _variable(type, name, array_size) {}

	argument_definition(sl::tag::buffer, const sl::named_variable& variable, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", { variable } }, name), _memory_qualifiers(memory_qualifiers), _is_buffer(true) {}

	argument_definition(sl::tag::buffer, const sl::named_variable_list& variables, const std::string& name, const sl::memory_qualifier_list& memory_qualifiers = {}) :
		_variable({ "", variables }, name), _memory_qualifiers(memory_qualifiers), _is_buffer(true) {}

private:
	sl::named_variable _variable;
	sl::memory_qualifier_storage _memory_qualifiers;
	bool _is_buffer = false;
};

struct argument_definitions {
	argument_definitions() {}

	argument_definitions(std::initializer_list<argument_definition> arguments) {
		for(const argument_definition& argument : arguments) {
			if(argument._is_buffer)
				buffers.push_back(sl::named_buffer(argument._variable.type().members(), argument._variable.name(), argument._memory_qualifiers.list()));
			else
				uniforms.push_back(argument._variable);
		}
	}

	sl::named_variable_list uniforms;
	sl::named_buffer_list buffers;
};

class argument_binding {
	friend class argument_binding_list;

	/*
	enum ArgumentTypeId : std::underlying_type<cgv::type::info::TypeId>::type {
		RTI_BUFFER = cgv::type::info::TypeId::TI_LAST,
		RTI_TEXTURE
	};
	*/

	// A bit dirty, but use class TypeId in the descriptor to mark this as a buffer resource binding.
	static const cgv::type::info::TypeId buffer_type_id = cgv::type::info::TypeId::TI_CLASS;

public:
	template<typename T>
	argument_binding(const std::string& name, const T& value) : _name(name) {
		_desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor({});
		_addr = cgv::render::element_descriptor_traits<T>::get_address(value);
	}

	argument_binding(const std::string& name, const cgv::render::vertex_buffer& buffer) : _name(name) {
		_desc.coordinate_type = buffer_type_id;
		_addr = &buffer;
	}

	bool is_buffer() const {
		return _desc.coordinate_type == buffer_type_id;
	}

private:
	std::string _name;
	cgv::render::type_descriptor _desc;
	const void* _addr = nullptr;
};

class argument_bindings {
public:
	virtual size_t get_uniform_count() const {
		return 0;
	}

	virtual const uniform_binding* get_uniform(size_t index) const {
		return nullptr;
	}

	virtual size_t get_buffer_count() const {
		return 0;
	}

	virtual const buffer_binding* get_buffer(size_t index) const {
		return nullptr;
	}
};

class argument_binding_list : public argument_bindings {
public:
	argument_binding_list(std::initializer_list<argument_binding> bindings) {
		for(const argument_binding& binding : bindings) {
			if(binding.is_buffer())
				_buffer_bindings.push_back({ binding._name, *reinterpret_cast<const cgv::render::vertex_buffer*>(binding._addr) });
			else
				_uniform_bindings.push_back({ binding._name, binding._desc, binding._addr });
		}
	}

	size_t get_uniform_count() const override {
		return _uniform_bindings.size();
	}

	const uniform_binding* get_uniform(size_t index) const override {
		return &_uniform_bindings[index];
	}

	size_t get_buffer_count() const override {
		return _buffer_bindings.size();
	}

	const buffer_binding* get_buffer(size_t index) const override {
		return &_buffer_bindings[index];
	}

private:
	uniform_binding_list _uniform_bindings;
	buffer_binding_list _buffer_bindings;
};

class argument_binding_struct : public argument_bindings {
public:
	size_t get_uniform_count() const override {
		return _uniform_bindings.size();
	}

	const uniform_binding* get_uniform(size_t index) const override {
		return _uniform_bindings[index];
	}

	size_t get_buffer_count() const override {
		return _buffer_bindings.size();
	}

	const buffer_binding* get_buffer(size_t index) const override {
		return _buffer_bindings[index];
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
