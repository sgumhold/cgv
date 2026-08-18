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

	uniform_binding(const std::string& name, bool value) : sl::named_object(name) {
		store_value(value);
	}

	uniform_binding(const std::string& name, int32_t value) : sl::named_object(name) {
		store_value(value);
	}

	uniform_binding(const std::string& name, uint32_t value) : sl::named_object(name) {
		store_value(value);
	}

	uniform_binding(const std::string& name, float value) : sl::named_object(name) {
		store_value(value);
	}

	uniform_binding(const std::string& name, double value) : sl::named_object(name) {
		store_value(value);
	}

	template<typename T, cgv::type::uint32_type N>
	uniform_binding(const std::string& name, const cgv::math::fvec<T, N>* value) : sl::named_object(name) {
		store_address(value);
	}

	template<typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
	uniform_binding(const std::string& name, const cgv::math::fmat<T, N, M>* value) : sl::named_object(name) {
		store_address(value);
	}

	template<typename T, typename std::enable_if<sl::traits::is_fundamental_sl_scalar_type_v<T>, bool>::type = true>
	uniform_binding(const std::string& name, const sl::data_type& type, T value) : sl::named_object(name) {
		store_value(value);
		// Overwrite descriptor set by store_value
		_desc = sl::get_type_descriptor(type);
	}

	void bind(bool value) {
		store_value(value);
	}

	void bind(int32_t value) {
		store_value(value);
	}

	void bind(uint32_t value) {
		store_value(value);
	}

	void bind(float value) {
		store_value(value);
	}

	void bind(double value) {
		store_value(value);
	}

	template<typename T, cgv::type::uint32_type N>
	void bind(const cgv::math::fvec<T, N>* value) {
		store_address(value);
	}

	template<typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
	void bind(const cgv::math::fmat<T, N, M>* value) {
		store_address(value);
	}

	template<typename T, typename std::enable_if<sl::traits::is_fundamental_sl_scalar_type_v<T>, bool>::type = true>
	void bind(const sl::data_type& type, T value) {
		store_value(value);
		// Overwrite descriptor set by store_value
		_desc = sl::get_type_descriptor(type);
	}

	cgv::render::type_descriptor descriptor() const {
		return _desc;
	};

	const void* address() const {
		if(_stores_value)
			return static_cast<const void*>(&_value_buffer);
		else
			return _addr;
	};

private:
	template<typename T>
	void store_value(T value) {
		_stores_value = true;
		_desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor({});
		T& buffer = reinterpret_cast<T&>(_value_buffer[0]);
		buffer = value;
	}

	template<typename T>
	void store_address(const T* address) {
		_stores_value = false;
		_desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor({});
		_addr = address;
	}

	bool _stores_value = false;
	cgv::render::type_descriptor _desc;
	union {
		const void* _addr = nullptr;
		uint8_t _value_buffer[sizeof(_addr)];
	};
};

using uniform_binding_list = std::vector<uniform_binding>;

template<typename T, typename Enable = void>
class typed_uniform_binding : public uniform_binding {
public:
	typed_uniform_binding() = delete;
	~typed_uniform_binding() = delete;
};

template<typename T>
class typed_uniform_binding<T, typename std::enable_if<sl::traits::is_fundamental_sl_scalar_type<T>::value>::type> : public uniform_binding {
public:
	typed_uniform_binding() {}

	typed_uniform_binding(const std::string& name) : uniform_binding(name) {}

	typed_uniform_binding(const std::string& name, T value) : uniform_binding(name, value) {}

	void bind(T value) {
		uniform_binding::bind(value);
	}
};

template<typename T, cgv::type::uint32_type N>
class typed_uniform_binding<cgv::math::fvec<T, N>, typename std::enable_if<sl::traits::is_fundamental_sl_scalar_type<T>::value>::type> : public uniform_binding {
public:
	using value_type = cgv::math::fvec<T, N>;

	typed_uniform_binding() {}

	typed_uniform_binding(const std::string& name) : uniform_binding(name) {}

	typed_uniform_binding(const std::string& name, const value_type* value) : uniform_binding(name, value) {}

	void bind(const value_type* value) {
		uniform_binding::bind(value);
	}
};

template<typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
class typed_uniform_binding<cgv::math::fmat<T, N, M>, typename std::enable_if<sl::traits::is_fundamental_sl_scalar_type<T>::value>::type> : public uniform_binding {
public:
	using value_type = cgv::math::fmat<T, N, M>;

	typed_uniform_binding() {}

	typed_uniform_binding(const std::string& name) : uniform_binding(name) {}

	typed_uniform_binding(const std::string& name, const value_type* value) : uniform_binding(name, value) {}

	void bind(const value_type* value) {
		uniform_binding::bind(value);
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
		_texture->enable(ctx, index);
	}

	void unbind(cgv::render::context& ctx) const {
		_texture->disable(ctx);
	}

private:
	cgv::render::texture* _texture = nullptr;
};

using texture_binding_list = std::vector<texture_binding>;

} // namespace gpgpu
} // namespace cgv
