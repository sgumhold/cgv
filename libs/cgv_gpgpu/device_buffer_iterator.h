#pragma once

#include <cgv/render/vertex_buffer.h>

namespace cgv {
namespace gpgpu {

using idxdiff_t = long long int;

class device_buffer_iterator {
public:
	device_buffer_iterator(const cgv::render::vertex_buffer& buffer, size_t index) : _buffer(buffer), _index(index) {}

	const cgv::render::vertex_buffer& buffer() const {
		return _buffer;
	}

	size_t index() const {
		return _index;
	}

	device_buffer_iterator& operator++() noexcept {
		++_index;
		return *this;
	}

	device_buffer_iterator operator++(int) noexcept {
		device_buffer_iterator temp = *this;
		++*this;
		return temp;
	}

	device_buffer_iterator& operator--() noexcept {
		--_index;
		return *this;
	}

	device_buffer_iterator operator--(int) noexcept {
		device_buffer_iterator temp = *this;
		--*this;
		return temp;
	}

	device_buffer_iterator& operator+=(const idxdiff_t offset) noexcept {
		_index += offset;
		return *this;
	}

	device_buffer_iterator operator+(const idxdiff_t offset) const noexcept {
		device_buffer_iterator temp = *this;
		temp += offset;
		return temp;
	}

	friend device_buffer_iterator operator+(const idxdiff_t offset, device_buffer_iterator next) noexcept {
		next += offset;
		return next;
	}

	device_buffer_iterator& operator-=(const idxdiff_t offset) noexcept {
		return *this += -offset;
	}

	device_buffer_iterator operator-(const idxdiff_t offset) const noexcept {
		device_buffer_iterator temp = *this;
		temp -= offset;
		return temp;
	}

	idxdiff_t operator-(const device_buffer_iterator& right) const noexcept {
		return _index - right._index;
	}

	bool operator==(const device_buffer_iterator& right) const noexcept {
		return &_buffer == &right._buffer && _index == right._index;
	}

private:
	const cgv::render::vertex_buffer& _buffer;
	size_t _index = 0;
};

bool compatible(device_buffer_iterator first, device_buffer_iterator second) {
	return &first.buffer() == &second.buffer();
}

device_buffer_iterator begin(const cgv::render::vertex_buffer& buffer) {
	return { buffer, 0 };
}

template<typename T>
device_buffer_iterator end(const cgv::render::vertex_buffer& buffer) {
	size_t size_in_bytes = buffer.get_size_in_bytes();
	size_t element_count = size_in_bytes / sizeof(T);
	return { buffer, element_count };
}

device_buffer_iterator end(const cgv::render::vertex_buffer& buffer, sl::data_type element_type) {
	size_t size_in_bytes = buffer.get_size_in_bytes();
	size_t element_count = size_in_bytes / sl::get_aligned_size(element_type);
	return { buffer, element_count };
}

idxdiff_t distance(device_buffer_iterator first, device_buffer_iterator last) {
	if(last.index() > first.index())
		return last - first;
	return 0;
}

} // namespace gpgpu
} // namespace cgv
