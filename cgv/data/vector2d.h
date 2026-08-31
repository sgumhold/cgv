#pragma once

#include <vector>

namespace cgv {
namespace data {

template<typename T>
class vector2d {
public:
	using value_type = T;
	using vector_type = std::vector<T>;

	vector2d() {}
	
	vector2d(size_t width, size_t height) {
		set_size_internal(width, height);
		data_ = vector_type(size_);
	}

	vector2d(size_t width, size_t height, const T& init) {
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

	typename std::vector<T>::iterator begin() noexcept {
		return data_.begin();
	}

	typename std::vector<T>::const_iterator begin() const noexcept {
		return data_.begin();
	}

	typename std::vector<T>::iterator end() noexcept {
		return data_.end();
	}

	typename std::vector<T>::const_iterator end() const noexcept {
		return data_.end();
	}

	typename std::vector<T>::reverse_iterator rbegin() noexcept {
		return data_.rbegin();
	}

	typename std::vector<T>::const_reverse_iterator rbegin() const noexcept {
		return data_.rbegin();
	}

	typename std::vector<T>::reverse_iterator rend() noexcept {
		return data_.rend();
	}

	typename std::vector<T>::const_reverse_iterator rend() const noexcept {
		return data_.rend();
	}

	typename std::vector<T>::const_iterator cbegin() const noexcept {
		return begin();
	}

	typename std::vector<T>::const_iterator cend() const noexcept {
		return end();
	}

	typename std::vector<T>::const_reverse_iterator crbegin() const noexcept {
		return rbegin();
	}

	typename std::vector<T>::const_reverse_iterator crend() const noexcept {
		return rend();
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

	void push_back(const T& value) {
		data_.insert(data_.end(), width_, value);
		set_size_internal(width_, height_ + 1);
	}

	void push_back(const std::vector<T>& values) {
		size_t count = values.size() > width_ ? width_ : values.size();
		data_.insert(data_.end(), values.begin(), values.begin() + count);
		set_size_internal(width_, height_ + 1);
	}

	void pop_back() {
		data_.erase(data_.end() - width_, data_.end());
		set_size_internal(width_, height_ - 1);
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
