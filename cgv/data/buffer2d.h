#pragma once

#include <vector>

namespace cgv {
namespace data {

template<typename T>
class buffer2d {
public:
	using vector_type = std::vector<T>;

	buffer2d() {}
	
	buffer2d(size_t width, size_t height) {
		set_size_internal(width, height);
		data_ = vector_type(size_);
	}

	buffer2d(size_t width, size_t height, const T& init) {
		set_size_internal(width, height);
		data_ = vector_type(size_, init);
	}

	size_t width() const {
		return width_;
	}

	size_t height() const {
		return height_;
	}

	size_t size() const {
		return size_;
	}

	bool empty() const {
		return size_ == 0;
	}

	typename std::vector<T>::iterator begin() {
		return data_.begin();
	}

	typename vector_type::iterator end() {
		return data_.end();
	}

	typename vector_type::const_iterator begin() const {
		return data_.cbegin();
	}

	typename vector_type::const_iterator end() const {
		return data_.cend();
	}

	T* data() {
		return data_.data();
	}

	const T* data() const {
		return data_.data();
	}

	void clear() {
		size_ = 0;
		data_.clear();
	}

	void resize(size_t width, size_t height) {
		set_size_internal(width, height);
		data_.resize(size_);
	}

	void resize(size_t width, size_t height, const T& init) {
		set_size_internal(width, height);
		data_.resize(size_, init);
	}

	T& operator[](size_t index) {
		return data_[index];
	}

	T operator[](size_t index) const {
		return data_[index];
	}

	T& operator()(size_t x, size_t y) {
		return data_[to_linear_index(x, y)];
	}

	T operator()(size_t x, size_t y) const {
		return data_[to_linear_index(x, y)];
	}

private:
	void set_size_internal(size_t width, size_t height) {
		width_ = width;
		height_ = height;
		size_ = width_ * height_;
	}

	size_t to_linear_index(size_t x, size_t y) const {
		return x + width_ * y;
	}

	size_t width_ = 0;
	size_t height_ = 0;
	size_t size_ = 0;
	vector_type data_;
};

} // namespace data
} // namespace cgv
