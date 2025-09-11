#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "sl.h"

namespace cgv {
namespace gpgpu {

class uniform_binding : public sl::named_object {
public:
	uniform_binding() : sl::named_object("") {}

	uniform_binding(const std::string& name) : sl::named_object(name) {}

	template<typename T>
	uniform_binding(const std::string& name, const T& value) : sl::named_object(name) {
		set(value);
	}

	uniform_binding(const std::string& name, cgv::render::type_descriptor descriptor, const void* address) : sl::named_object(name), _desc(descriptor), _addr(address) {}

	template<typename T>
	void operator=(const T& value) {
		set(value);
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

class buffer_binding : public sl::named_object {
public:
	buffer_binding() : sl::named_object("") {}

	buffer_binding(const std::string& name) : sl::named_object(name) {}

	buffer_binding(const std::string& name, const cgv::render::vertex_buffer& buffer) : sl::named_object(name) {
		_buffer = &buffer;
	}

	void operator=(const cgv::render::vertex_buffer& buffer) {
		_buffer = &buffer;
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
	mutable uint32_t _binding_index = 0;
	const cgv::render::vertex_buffer* _buffer = nullptr;
};

using buffer_binding_list = std::vector<buffer_binding>;

class image_binding : public sl::named_object {
public:
	image_binding() : sl::named_object("") {}

	image_binding(const std::string& name) : sl::named_object(name) {}

	image_binding(const std::string& name, cgv::render::texture& texture) : sl::named_object(name) {
		_texture = &texture;
	}

	void operator=(cgv::render::texture& texture) {
		_texture = &texture;
	}

	const cgv::render::texture* buffer() const {
		return _texture;
	};

	void bind(cgv::render::context& ctx, uint32_t index) const {
		_binding_index = index;
		// TODO: Expose access type to user?
		_texture->bind_as_image(ctx, index, 0, false, 0, cgv::render::AccessType::AT_READ_WRITE);
	}

	void unbind(cgv::render::context& ctx) const {
		// TODO: call unbind if it is ever implemented.
		//_texture->bind_as_image();
	}

private:
	mutable uint32_t _binding_index = 0;
	cgv::render::texture* _texture = nullptr;
};

using image_binding_list = std::vector<image_binding>;

class texture_binding : public sl::named_object {
public:
	texture_binding() : sl::named_object("") {}

	texture_binding(const std::string& name) : sl::named_object(name) {}

	texture_binding(const std::string& name, cgv::render::texture& texture) : sl::named_object(name) {
		_texture = &texture;
	}

	void operator=(cgv::render::texture& texture) {
		_texture = &texture;
	}

	const cgv::render::texture* buffer() const {
		return _texture;
	};

	void bind(cgv::render::context& ctx, uint32_t index) const {
		//_binding_index = index;
		_texture->enable(ctx, index);
	}

	void unbind(cgv::render::context& ctx) const {
		_texture->disable(ctx);
	}

private:
	//mutable uint32_t _binding_index = 0;
	cgv::render::texture* _texture = nullptr;
};

using texture_binding_list = std::vector<texture_binding>;

} // namespace gpgpu
} // namespace cgv
